/**
 * @file MusicAlgorithm.cpp
 * @brief MUSIC算法实现 — 协方差特征分解 + 伪谱频率/DOA估计
 */

#include "utils/signal15/MusicAlgorithm.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <cmath>

/* ── 构造/配置 ── */

/** @brief 构造函数 @param parent 父对象 */
MusicAlgorithm::MusicAlgorithm(QObject* parent)
    : QObject(parent)
    , m_mode(Mode::FrequencyEstimation)
    , m_signalCount(1)
    , m_sensors(8)
    , m_snapshots(100)
    , m_searchPoints(512)
    , m_samplingRate(1.0)
{
}

/** @brief 设置估计模式 @param mode 模式 */
void MusicAlgorithm::setMode(Mode mode) { m_mode = mode; }

/** @brief 设置信号源数 @param count 信号数 */
void MusicAlgorithm::setSignalCount(int count) { m_signalCount = qMax(1, count); }

/** @brief 设置阵列参数 @param sensors 传感器数 @param snapshots 快照数 */
void MusicAlgorithm::setArrayParams(int sensors, int snapshots)
{
    m_sensors = qMax(2, sensors);
    m_snapshots = qMax(1, snapshots);
}

/** @brief 设置搜索网格密度 @param points 搜索点数 */
void MusicAlgorithm::setSearchPoints(int points)
{
    m_searchPoints = qMax(64, points);
}

/** @brief 设置采样率 @param fs 采样率 */
void MusicAlgorithm::setSamplingRate(double fs) { m_samplingRate = qMax(1e-6, fs); }

/* ── 核心估计 ── */

/** @brief 执行MUSIC估计 @param data 输入数据[snapshots x sensors] @return 估计结果 */
MusicAlgorithm::MusicResult MusicAlgorithm::estimate(
    const QVector<QVector<double>>& data)
{
    MusicResult result;
    if (data.isEmpty() || data[0].size() < 2) return result;

    QElapsedTimer timer;
    timer.start();

    int M = data[0].size(); /* 传感器/阵元数 */
    int sigCount = qMin(m_signalCount, M - 1);

    /* 1. 计算采样协方差矩阵 */
    auto R = computeCovariance(data);

    /* 2. 特征分解 */
    QVector<double> eigenvalues;
    QVector<QVector<double>> eigenvectors;
    eigenDecompose(R, eigenvalues, eigenvectors);

    /* 3. 构造噪声子空间(最小M-sigCount个特征值对应的特征向量) */
    /* 先排序特征值索引 */
    QVector<int> idx(eigenvalues.size());
    for (int i = 0; i < idx.size(); ++i) idx[i] = i;
    std::sort(idx.begin(), idx.end(), [&](int a, int b) {
        return eigenvalues[a] < eigenvalues[b];
    });

    int noiseDim = M - sigCount;
    QVector<QVector<double>> noiseSubspace(noiseDim);
    for (int i = 0; i < noiseDim; ++i) {
        noiseSubspace[i] = eigenvectors[idx[i]];
    }

    /* 4. 计算伪谱 */
    result.searchGrid.resize(m_searchPoints);
    if (m_mode == Mode::FrequencyEstimation) {
        for (int i = 0; i < m_searchPoints; ++i) {
            result.searchGrid[i] = static_cast<double>(i)
                / static_cast<double>(m_searchPoints) * 0.5; /* 归一化频率[0,0.5] */
        }
    } else {
        for (int i = 0; i < m_searchPoints; ++i) {
            result.searchGrid[i] = -90.0 + 180.0 * static_cast<double>(i)
                / static_cast<double>(m_searchPoints - 1); /* 角度[-90,90] */
        }
    }

    result.pseudoSpectrum = computePseudoSpectrum(noiseSubspace);

    /* 5. 提取峰值 */
    result.estimatedValues = extractPeaks(result.pseudoSpectrum, result.searchGrid);
    result.signalCount = sigCount;
    result.resolution = 1.0 / static_cast<double>(m_searchPoints);

    /* 更新统计 */
    ++m_stats.totalEstimations;
    m_stats.totalSnapshotsProcessed += data.size();
    m_stats.totalSignalsDetected += sigCount;
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEstimations;

    emit estimationComplete(sigCount, elapsed);
    return result;
}

/** @brief 自动检测信号数(MDL准则) @param data 输入数据 @return 检测到的信号数 */
int MusicAlgorithm::detectSignalCount(const QVector<QVector<double>>& data)
{
    if (data.isEmpty()) return 0;

    int M = data[0].size();
    int N = data.size();

    auto R = computeCovariance(data);
    QVector<double> eigenvalues;
    QVector<QVector<double>> eigenvectors;
    eigenDecompose(R, eigenvalues, eigenvectors);

    /* 按降序排列特征值 */
    QVector<double> sorted = eigenvalues;
    std::sort(sorted.begin(), sorted.end(), std::greater<double>());

    /* MDL准则 */
    double bestMDL = 1e300;
    int bestK = 0;

    for (int k = 0; k < M - 1; ++k) {
        /* 几何均值和算术均值(噪声部分) */
        double logGeoMean = 0.0;
        double arithMean = 0.0;
        for (int i = k; i < M; ++i) {
            double val = qMax(sorted[i], 1e-15);
            logGeoMean += std::log(val);
            arithMean += val;
        }
        int p = M - k;
        logGeoMean /= p;
        arithMean /= p;

        double mdl = -N * p * (logGeoMean - std::log(arithMean))
                   + 0.5 * k * (2 * M - k - 1) * std::log(static_cast<double>(N));

        if (mdl < bestMDL) {
            bestMDL = mdl;
            bestK = k;
        }
    }

    return bestK;
}

/* ── 私有: 协方差矩阵 ── */

/** @brief 计算采样协方差矩阵 R = (1/N) * X^H * X */
QVector<QVector<double>> MusicAlgorithm::computeCovariance(
    const QVector<QVector<double>>& data) const
{
    int M = data[0].size();
    int N = data.size();

    QVector<QVector<double>> R(M, QVector<double>(M, 0.0));

    for (int i = 0; i < M; ++i) {
        for (int j = i; j < M; ++j) {
            double sum = 0.0;
            for (int n = 0; n < N; ++n) {
                sum += data[n][i] * data[n][j];
            }
            R[i][j] = sum / static_cast<double>(N);
            R[j][i] = R[i][j];
        }
    }
    return R;
}

/* ── 私有: Jacobi特征分解 ── */

/** @brief 对称矩阵Jacobi特征分解 */
void MusicAlgorithm::eigenDecompose(
    const QVector<QVector<double>>& mat,
    QVector<double>& eigenvalues,
    QVector<QVector<double>>& eigenvectors)
{
    int n = mat.size();
    QVector<QVector<double>> A = mat; /* 工作副本 */
    eigenvectors.resize(n);
    for (int i = 0; i < n; ++i) {
        eigenvectors[i].resize(n, 0.0);
        eigenvectors[i][i] = 1.0;
    }

    const int maxIter = 100;
    const double tol = 1e-10;

    for (int iter = 0; iter < maxIter; ++iter) {
        /* 找最大非对角元 */
        double maxOff = 0.0;
        int p = 0, q = 1;
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                if (std::abs(A[i][j]) > maxOff) {
                    maxOff = std::abs(A[i][j]);
                    p = i; q = j;
                }
            }
        }
        if (maxOff < tol) break;

        /* 计算旋转角度 */
        double app = A[p][p], aqq = A[q][q], apq = A[p][q];
        double theta = 0.0;
        if (std::abs(app - aqq) < 1e-15) {
            theta = M_PI / 4.0;
        } else {
            theta = 0.5 * std::atan2(2.0 * apq, app - aqq);
        }
        double c = std::cos(theta), s = std::sin(theta);

        /* 旋转变换 */
        for (int i = 0; i < n; ++i) {
            if (i == p || i == q) continue;
            double aip = A[i][p], aiq = A[i][q];
            A[i][p] = c * aip + s * aiq;
            A[p][i] = A[i][p];
            A[i][q] = -s * aip + c * aiq;
            A[q][i] = A[i][q];
        }
        double newPP = c * c * app + 2.0 * s * c * apq + s * s * aqq;
        double newQQ = s * s * app - 2.0 * s * c * apq + c * c * aqq;
        A[p][p] = newPP;
        A[q][q] = newQQ;
        A[p][q] = 0.0;
        A[q][p] = 0.0;

        /* 更新特征向量 */
        for (int i = 0; i < n; ++i) {
            double vp = eigenvectors[i][p];
            double vq = eigenvectors[i][q];
            eigenvectors[i][p] = c * vp + s * vq;
            eigenvectors[i][q] = -s * vp + c * vq;
        }
    }

    /* 提取特征值 */
    eigenvalues.resize(n);
    for (int i = 0; i < n; ++i) eigenvalues[i] = A[i][i];
}

/* ── 私有: 伪谱计算 ── */

/** @brief 构造MUSIC伪谱 P = 1 / (a^H * Un * Un^H * a) */
QVector<double> MusicAlgorithm::computePseudoSpectrum(
    const QVector<QVector<double>>& noiseSubspace)
{
    int M = noiseSubspace.isEmpty() ? m_sensors : noiseSubspace[0].size();
    int noiseDim = noiseSubspace.size();
    QVector<double> spectrum(m_searchPoints);

    for (int i = 0; i < m_searchPoints; ++i) {
        /* 构造导向向量 a(theta) */
        QVector<double> steering(M);
        if (m_mode == Mode::FrequencyEstimation) {
            double freq = static_cast<double>(i)
                        / static_cast<double>(m_searchPoints) * 0.5;
            for (int m = 0; m < M; ++m) {
                steering[m] = std::cos(2.0 * M_PI * freq * m);
            }
        } else {
            double angle = -90.0 + 180.0 * static_cast<double>(i)
                         / static_cast<double>(m_searchPoints - 1);
            double rad = qDegreesToRadians(angle);
            for (int m = 0; m < M; ++m) {
                steering[m] = std::cos(M_PI * m * std::sin(rad));
            }
        }

        /* 计算 a^H * Un * Un^H * a = ||Un^H * a||^2 */
        double denom = 0.0;
        for (int k = 0; k < noiseDim; ++k) {
            double dot = 0.0;
            for (int m = 0; m < M; ++m) {
                dot += noiseSubspace[k][m] * steering[m];
            }
            denom += dot * dot;
        }

        spectrum[i] = (denom > 1e-15) ? (10.0 * std::log10(1.0 / denom)) : 60.0;
    }

    return spectrum;
}

/* ── 私有: 峰值提取 ── */

/** @brief 从伪谱提取前sigCount个峰值 */
QVector<double> MusicAlgorithm::extractPeaks(
    const QVector<double>& pseudoSpectrum,
    const QVector<double>& grid)
{
    QVector<double> peaks;
    if (pseudoSpectrum.size() < 3) return peaks;

    /* 找所有局部峰值 */
    QList<QPair<double, int>> candidates;
    for (int i = 1; i < pseudoSpectrum.size() - 1; ++i) {
        if (pseudoSpectrum[i] > pseudoSpectrum[i - 1]
            && pseudoSpectrum[i] > pseudoSpectrum[i + 1]) {
            candidates.append({pseudoSpectrum[i], i});
        }
    }

    /* 按幅度降序排列 */
    std::sort(candidates.begin(), candidates.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    int count = qMin(m_signalCount, candidates.size());
    for (int i = 0; i < count; ++i) {
        double val = grid[candidates[i].second];
        if (m_mode == Mode::FrequencyEstimation) {
            val *= m_samplingRate; /* 转换为Hz */
        }
        peaks.append(val);
    }
    return peaks;
}

/* ── 统计 ── */

/** @brief 重置统计 */
void MusicAlgorithm::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
