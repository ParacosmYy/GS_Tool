/**
 * @file Beamformer.cpp
 * @brief 波束形成器实现 — DAS/MVDR/Frost/导向矢量/波束图
 */

#include "utils/signal20/Beamformer.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <numeric>

/** @brief 构造函数 @param parent 父对象 */
Beamformer::Beamformer(QObject* parent)
    : QObject(parent)
    , m_algorithm(Algorithm::DelayAndSum)
    , m_sampleRate(44100.0)
    , m_azimuth(0.0)
    , m_elevation(0.0)
    , m_timeSum(0.0)
{
    m_config.elementCount = 4;
    m_config.spacing = 0.5;
    m_config.speedOfSound = 343.0;
}

/** @brief 设置阵列配置 @param config 阵列参数 */
void Beamformer::setArrayConfig(const ArrayConfig& config)
{
    m_config = config;
    if (m_config.positions.isEmpty()) {
        m_config.positions.resize(m_config.elementCount);
        for (int i = 0; i < m_config.elementCount; ++i) {
            m_config.positions[i] = static_cast<double>(i) * m_config.spacing;
        }
    }
}

/** @brief 设置波束形成算法 @param algorithm 算法 */
void Beamformer::setAlgorithm(Algorithm algorithm)
{
    m_algorithm = algorithm;
}

/** @brief 设置采样率 @param rate 采样率 */
void Beamformer::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

/** @brief 设置波束指向角 @param azimuthDeg 方位角 @param elevationDeg 仰角 */
void Beamformer::setSteeringDirection(double azimuthDeg, double elevationDeg)
{
    m_azimuth = qDegreesToRadians(azimuthDeg);
    m_elevation = qDegreesToRadians(elevationDeg);
    ++m_stats.totalSteeringUpdates;
    emit steeringUpdated(azimuthDeg);
}

/** @brief 计算导向矢量 @param frequencyHz 频率 @return 导向矢量(实部/虚部交错) */
QVector<double> Beamformer::steeringVector(double frequencyHz) const
{
    int M = m_config.elementCount;
    QVector<double> sv(2 * M); /* 实部/虚部交错存储 */

    double wavelength = m_config.speedOfSound / qMax(1.0, frequencyHz);
    double k = 2.0 * M_PI / wavelength; /* 波数 */

    for (int m = 0; m < M; ++m) {
        double pos = (m < m_config.positions.size())
            ? m_config.positions[m] : static_cast<double>(m) * m_config.spacing;
        double phase = k * pos * qSin(m_azimuth) * qCos(m_elevation);
        sv[2 * m] = qCos(phase);      /* 实部 */
        sv[2 * m + 1] = qSin(phase);  /* 虚部 */
    }

    return sv;
}

/** @brief 处理多通道数据 @param input [通道×样本] @return 波束输出 */
Beamformer::BeamOutput Beamformer::process(const QVector<QVector<double>>& input)
{
    BeamOutput result;

    switch (m_algorithm) {
    case Algorithm::DelayAndSum:
        result.output = delayAndSum(input);
        break;
    case Algorithm::MVDR:
        result.output = mvdr(input);
        break;
    case Algorithm::Frost:
        result.output = frost(input);
        break;
    }

    if (!result.output.isEmpty()) {
        result.arrayGainDb = computeArrayGain(input, result.output);
        result.snrImprovementDb = result.arrayGainDb;
        if (result.arrayGainDb > m_stats.peakArrayGainDb) {
            m_stats.peakArrayGainDb = result.arrayGainDb;
        }
    }

    emit processingComplete(result.output.size(), result.arrayGainDb);
    return result;
}

/** @brief 延迟求和波束形成 @param input 多通道输入 @param weights 权重 @return 输出 */
QVector<double> Beamformer::delayAndSum(const QVector<QVector<double>>& input,
                                         const QVector<double>& weights)
{
    QElapsedTimer timer;
    timer.start();

    int M = input.size();
    if (M == 0) return {};

    int N = input[0].size();
    QVector<double> output(N, 0.0);

    /* 默认均匀权重 */
    QVector<double> w = weights;
    if (w.isEmpty()) {
        w.resize(M);
        for (int m = 0; m < M; ++m) w[m] = 1.0 / M;
    }

    /* 对每个通道应用时延并加权求和 */
    for (int m = 0; m < M; ++m) {
        double delaySamples = computeDelay(m) * m_sampleRate;
        int intDelay = static_cast<int>(qRound(delaySamples));
        double fracDelay = delaySamples - intDelay;

        for (int n = 0; n < N; ++n) {
            int idx = n - intDelay;
            double sample = 0.0;

            if (idx >= 0 && idx < N) {
                /* 分数延迟插值(一阶) */
                if (idx + 1 < N && qAbs(fracDelay) > 1e-10) {
                    sample = input[m][idx] * (1.0 - fracDelay)
                           + input[m][idx + 1] * fracDelay;
                } else {
                    sample = input[m][idx];
                }
            }
            output[n] += w[m] * sample;
        }
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    m_stats.totalProcessed += static_cast<quint64>(N);
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalProcessed);

    return output;
}

/** @brief MVDR波束形成 @param input 多通道输入 @return 输出 */
QVector<double> Beamformer::mvdr(const QVector<QVector<double>>& input)
{
    QElapsedTimer timer;
    timer.start();

    int M = input.size();
    if (M == 0) return {};
    int N = input[0].size();

    /* 估计协方差矩阵 */
    auto R = estimateCovariance(input);

    /* 对角加载正则化 */
    double diagLoad = 1e-4;
    for (int i = 0; i < M; ++i) R[i][i] += diagLoad;

    /* R^-1 */
    auto Rinv = invertMatrix(R);

    /* 导向矢量(使用中心频率近似) */
    double centerFreq = m_sampleRate / 4.0;
    auto sv = steeringVector(centerFreq);

    /* w = R^-1 * a / (a^H * R^-1 * a) */
    QVector<double> wReal(M, 0.0), wImag(M, 0.0);
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < M; ++j) {
            wReal[i] += Rinv[i][j] * sv[2 * j];
            wImag[i] += Rinv[i][j] * sv[2 * j + 1];
        }
    }

    /* 归一化分母 a^H * R^-1 * a */
    double denomReal = 0.0, denomImag = 0.0;
    for (int i = 0; i < M; ++i) {
        denomReal += sv[2 * i] * wReal[i] + sv[2 * i + 1] * wImag[i];
        denomImag += sv[2 * i] * wImag[i] - sv[2 * i + 1] * wReal[i];
    }
    double denomMag = qSqrt(denomReal * denomReal + denomImag * denomImag);
    if (denomMag < 1e-10) denomMag = 1e-10;

    for (int i = 0; i < M; ++i) {
        wReal[i] /= denomMag;
        wImag[i] /= denomMag;
    }

    /* 应用复数权重(简化为实数处理) */
    QVector<double> output(N, 0.0);
    for (int n = 0; n < N; ++n) {
        for (int m = 0; m < M; ++m) {
            output[n] += wReal[m] * input[m][n];
        }
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    m_stats.totalProcessed += static_cast<quint64>(N);
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalProcessed);

    return output;
}

/** @brief Frost自适应波束形成 @param input 多通道输入 @param stepSize 步长 @param constraintLambda 约束参数 @return 输出 */
QVector<double> Beamformer::frost(const QVector<QVector<double>>& input,
                                   double stepSize, double constraintLambda)
{
    QElapsedTimer timer;
    timer.start();

    int M = input.size();
    if (M == 0) return {};
    int N = input[0].size();

    /* 初始化自适应权重 */
    if (m_frostWeights.size() != M) {
        m_frostWeights.resize(M);
        for (int m = 0; m < M; ++m) m_frostWeights[m] = 1.0 / M;
    }

    /* 导向矢量约束 */
    auto sv = steeringVector(m_sampleRate / 4.0);
    QVector<double> constraint(M);
    for (int m = 0; m < M; ++m) constraint[m] = sv[2 * m];

    QVector<double> output(N, 0.0);

    for (int n = 0; n < N; ++n) {
        /* 计算输出 */
        double y = 0.0;
        for (int m = 0; m < M; ++m) {
            y += m_frostWeights[m] * input[m][n];
        }
        output[n] = y;

        /* 计算约束违反量 */
        double constraintDot = 0.0;
        for (int m = 0; m < M; ++m) {
            constraintDot += constraint[m] * m_frostWeights[m];
        }
        double constraintError = 1.0 - constraintDot;

        /* LMS更新 + 约束投影 */
        for (int m = 0; m < M; ++m) {
            /* 无约束梯度 */
            double gradient = -y * input[m][n];
            /* 投影到约束子空间 */
            gradient += constraintLambda * constraintError * constraint[m];
            m_frostWeights[m] -= stepSize * gradient;
        }

        /* 重新归一化以满足约束 */
        constraintDot = 0.0;
        for (int m = 0; m < M; ++m) {
            constraintDot += constraint[m] * m_frostWeights[m];
        }
        if (qAbs(constraintDot) > 1e-10) {
            for (int m = 0; m < M; ++m) {
                m_frostWeights[m] /= constraintDot;
            }
        }
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    m_stats.totalProcessed += static_cast<quint64>(N);
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalProcessed);

    return output;
}

/** @brief 计算阵列增益 @param input 多通道输入 @param output 波束输出 @return 增益(dB) */
double Beamformer::computeArrayGain(const QVector<QVector<double>>& input,
                                     const QVector<double>& output) const
{
    int M = input.size();
    int N = qMin(input.isEmpty() ? 0 : input[0].size(), output.size());
    if (M == 0 || N == 0) return 0.0;

    /* 输出功率 */
    double outputPower = 0.0;
    for (int n = 0; n < N; ++n) outputPower += output[n] * output[n];
    outputPower /= N;

    /* 平均输入功率 */
    double inputPower = 0.0;
    for (int m = 0; m < M; ++m) {
        for (int n = 0; n < N; ++n) {
            inputPower += input[m][n] * input[m][n];
        }
    }
    inputPower /= (M * N);

    if (inputPower < 1e-20) return 0.0;
    return 10.0 * qLn(outputPower / inputPower) / qLn(10.0);
}

/** @brief 计算波束图 @param frequencyHz 频率 @param angleResolutionDeg 角度分辨率 @return (角度列表, 增益列表) */
QPair<QVector<double>, QVector<double>> Beamformer::beamPattern(
    double frequencyHz, double angleResolutionDeg) const
{
    QVector<double> angles, gains;
    double wavelength = m_config.speedOfSound / qMax(1.0, frequencyHz);
    double k = 2.0 * M_PI / wavelength;
    int M = m_config.elementCount;

    for (double deg = -90.0; deg <= 90.0; deg += angleResolutionDeg) {
        double theta = qDegreesToRadians(deg);
        angles.append(deg);

        /* 计算阵列响应 */
        double responseReal = 0.0, responseImag = 0.0;
        for (int m = 0; m < M; ++m) {
            double pos = (m < m_config.positions.size())
                ? m_config.positions[m] : static_cast<double>(m) * m_config.spacing;
            double phase = k * pos * (qSin(theta) - qSin(m_azimuth));
            responseReal += qCos(phase);
            responseImag += qSin(phase);
        }

        double magnitude = qSqrt(responseReal * responseReal + responseImag * responseImag) / M;
        gains.append(20.0 * qLn(qMax(magnitude, 1e-10)) / qLn(10.0));
    }

    return {angles, gains};
}

/** @brief 重置统计 */
void Beamformer::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_frostWeights.clear();
}

/** @brief 计算阵元延迟 @param elementIdx 阵元索引 @return 延迟(秒) */
double Beamformer::computeDelay(int elementIdx) const
{
    double pos = (elementIdx < m_config.positions.size())
        ? m_config.positions[elementIdx]
        : static_cast<double>(elementIdx) * m_config.spacing;

    return pos * qSin(m_azimuth) * qCos(m_elevation) / m_config.speedOfSound;
}

/** @brief 估计协方差矩阵 @param input 多通道输入 @return 协方差矩阵 */
QVector<QVector<double>> Beamformer::estimateCovariance(
    const QVector<QVector<double>>& input) const
{
    int M = input.size();
    int N = (M > 0) ? input[0].size() : 0;

    QVector<QVector<double>> R(M, QVector<double>(M, 0.0));

    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < M; ++j) {
            double sum = 0.0;
            for (int n = 0; n < N; ++n) {
                sum += input[i][n] * input[j][n];
            }
            R[i][j] = sum / qMax(1, N);
        }
    }

    return R;
}

/** @brief 2×2矩阵求逆 @param M 矩阵 @param inv 逆矩阵 */
void Beamformer::invertMatrix2x2(const QVector<QVector<double>>& M,
                                  QVector<QVector<double>>& inv) const
{
    double det = M[0][0] * M[1][1] - M[0][1] * M[1][0];
    if (qAbs(det) < 1e-15) det = 1e-15;

    inv[0][0] = M[1][1] / det;
    inv[0][1] = -M[0][1] / det;
    inv[1][0] = -M[1][0] / det;
    inv[1][1] = M[0][0] / det;
}

/** @brief 矩阵求逆(Gauss-Jordan) @param M 矩阵 @return 逆矩阵 */
QVector<QVector<double>> Beamformer::invertMatrix(
    const QVector<QVector<double>>& M) const
{
    int n = M.size();
    QVector<QVector<double>> aug(n, QVector<double>(2 * n, 0.0));

    /* 构造增广矩阵 [M | I] */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) aug[i][j] = M[i][j];
        aug[i][n + i] = 1.0;
    }

    /* Gauss-Jordan消元 */
    for (int i = 0; i < n; ++i) {
        double pivot = aug[i][i];
        if (qAbs(pivot) < 1e-15) {
            /* 寻找非零主元 */
            for (int k = i + 1; k < n; ++k) {
                if (qAbs(aug[k][i]) > 1e-15) {
                    std::swap(aug[i], aug[k]);
                    pivot = aug[i][i];
                    break;
                }
            }
        }
        if (qAbs(pivot) < 1e-15) continue;

        for (int j = 0; j < 2 * n; ++j) aug[i][j] /= pivot;

        for (int k = 0; k < n; ++k) {
            if (k == i) continue;
            double factor = aug[k][i];
            for (int j = 0; j < 2 * n; ++j) {
                aug[k][j] -= factor * aug[i][j];
            }
        }
    }

    /* 提取逆矩阵 */
    QVector<QVector<double>> inv(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            inv[i][j] = aug[i][n + j];
        }
    }

    return inv;
}
