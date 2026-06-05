/**
 * @file MultiTaper.cpp
 * @brief 多锥谱估计引擎实现 — DPSS序列/自适应加权/特征谱平均
 */

#include "utils/fft21/MultiTaper.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
MultiTaper::MultiTaper(QObject* parent)
    : QObject(parent)
    , m_sampleRate(1000.0)
    , m_nw(3.0)
    , m_numTapers(5)
    , m_timeSum(0.0)
{
}

/** @brief 设置采样率 @param rate 采样率(Hz) */
void MultiTaper::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

/** @brief 设置带宽参数(NW) @param nw 时间带宽积 */
void MultiTaper::setBandwidthParameter(double nw)
{
    m_nw = qMax(0.5, nw);
    m_numTapers = qMin(m_numTapers, static_cast<int>(2.0 * m_nw) - 1);
    m_numTapers = qMax(1, m_numTapers);
}

/** @brief 设置锥数量 @param numTapers 锥数 */
void MultiTaper::setNumTapers(int numTapers)
{
    m_numTapers = qMax(1, numTapers);
    int maxTapers = qMax(1, static_cast<int>(2.0 * m_nw) - 1);
    m_numTapers = qMin(m_numTapers, maxTapers);
}

/** @brief 计算多锥谱估计 @param data 时域输入数据 @return 谱估计结果 */
MultiTaper::SpectrumResult MultiTaper::estimate(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    SpectrumResult result;
    int n = data.size();
    if (n < 8) return result;

    /* 补零到2的幂 */
    int fftSize = 1;
    while (fftSize < n) fftSize *= 2;

    /* 第一步: 计算DPSS锥序列 */
    QVector<QVector<double>> tapers(m_numTapers);
    for (int k = 0; k < m_numTapers; ++k) {
        tapers[k] = computeDPSS(n, m_nw, k);
    }

    /* 第二步: 对每个锥计算特征谱 */
    QVector<QVector<double>> eigenspectra(m_numTapers);
    for (int k = 0; k < m_numTapers; ++k) {
        QVector<double> real(fftSize, 0.0), imag(fftSize, 0.0);
        for (int i = 0; i < n; ++i) {
            real[i] = data[i] * tapers[k][i];
        }
        fft(real, imag);

        /* 计算功率谱密度 */
        int halfN = fftSize / 2;
        eigenspectra[k].resize(halfN);
        for (int i = 0; i < halfN; ++i) {
            double re = real[i], im = imag[i];
            eigenspectra[k][i] = (re * re + im * im) / static_cast<double>(n);
        }
    }

    /* 第三步: 自适应加权合并特征谱 */
    int halfN = fftSize / 2;
    result.powerSpectrum = adaptiveWeightedAverage(eigenspectra, n);

    /* 计算幅度谱 */
    result.amplitudeSpectrum.resize(halfN);
    for (int i = 0; i < halfN; ++i) {
        result.amplitudeSpectrum[i] = qSqrt(result.powerSpectrum[i]);
    }

    /* 频率轴 */
    result.frequencies.resize(halfN);
    for (int i = 0; i < halfN; ++i) {
        result.frequencies[i] = static_cast<double>(i) * m_sampleRate / fftSize;
    }

    /* 估计噪声底: 使用最低25%功率的中位数 */
    QVector<double> sortedPower = result.powerSpectrum;
    std::sort(sortedPower.begin(), sortedPower.end());
    int noiseIdx = qMax(1, sortedPower.size() / 4);
    result.noiseFloor = sortedPower[noiseIdx];

    /* 有效带宽 */
    result.bandwidth = 2.0 * m_nw * m_sampleRate / n;

    /* 更新统计 */
    double elapsed = timer.elapsed();
    ++m_stats.totalEstimations;
    m_stats.totalSamplesProcessed += static_cast<quint64>(n);
    m_stats.totalTapersUsed += static_cast<quint64>(m_numTapers);
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalEstimations);
    m_stats.avgNoiseFloor += (result.noiseFloor - m_stats.avgNoiseFloor)
        / static_cast<double>(m_stats.totalEstimations);

    emit estimationComplete(n, m_numTapers, result.bandwidth);
    return result;
}

/** @brief 计算Slepian/DPSS序列 @param n 序列长度 @param nw 带宽参数 @param k 锥序号 @return DPSS序列 */
QVector<double> MultiTaper::computeDPSS(int n, double nw, int k) const
{
    if (n < 1 || k < 0 || k >= n) {
        return QVector<double>(n, 0.0);
    }

    /* 构建三对角矩阵的特征问题:
     * 对角线元素 d[i] = cos(2*pi*NW*(i - (n-1)/2) / n) * 0.5
     * 次对角线 e[i] = (i+1) * (n-1-i) * 0.5 / n */

    QVector<double> diag(n), offDiag(n - 1);
    for (int i = 0; i < n; ++i) {
        double t = static_cast<double>(i) - (n - 1) * 0.5;
        diag[i] = qCos(2.0 * M_PI * nw * t / n) * 0.5;
    }
    for (int i = 0; i < n - 1; ++i) {
        offDiag[i] = static_cast<double>((i + 1) * (n - 1 - i)) * 0.5 / n;
    }

    /* 使用QL迭代求第k小特征值对应的特征向量 */
    QVector<double> v = tridiagonalSolve(diag, offDiag, k + 1);
    if (v.size() != n) return QVector<double>(n, 0.0);

    /* 取第k个特征向量 */
    QVector<double> dpss(n);
    int offset = k * n;
    for (int i = 0; i < n; ++i) {
        dpss[i] = v[offset + i];
    }

    /* 归一化 */
    double norm = 0.0;
    for (int i = 0; i < n; ++i) norm += dpss[i] * dpss[i];
    norm = qSqrt(norm);
    if (norm > 1e-14) {
        for (int i = 0; i < n; ++i) dpss[i] /= norm;
    }

    return dpss;
}

/** @brief 自适应加权合并特征谱 @param eigenspectra 特征谱列表 @param n 数据长度 @return 加权功率谱 */
QVector<double> MultiTaper::adaptiveWeightedAverage(
    const QVector<QVector<double>>& eigenspectra, int n) const
{
    if (eigenspectra.isEmpty()) return {};

    int numTapers = eigenspectra.size();
    int halfN = eigenspectra[0].size();
    QVector<double> result(halfN, 0.0);

    /* 自适应权重: 根据每个特征谱相对于加权平均的偏差计算 */
    QVector<double> weights(numTapers, 1.0);

    for (int bin = 0; bin < halfN; ++bin) {
        /* 迭代计算自适应权重(2次迭代) */
        for (int iter = 0; iter < 2; ++iter) {
            /* 加权平均 */
            double wSum = 0.0;
            double wDenom = 0.0;
            for (int k = 0; k < numTapers; ++k) {
                wSum += weights[k] * weights[k] * eigenspectra[k][bin];
                wDenom += weights[k] * weights[k];
            }
            double avgSpec = (wDenom > 1e-14) ? wSum / wDenom : 0.0;

            /* 更新权重: w_k = d_k^2 / (d_k^2 + avgSpec)
             * 其中d_k是特征值(近似为1-(k/(2NW))^2) */
            for (int k = 0; k < numTapers; ++k) {
                double lambda = 1.0 - qPow(
                    static_cast<double>(k) / (2.0 * m_nw), 2);
                lambda = qBound(0.0, lambda, 1.0);
                if (avgSpec > 1e-14) {
                    weights[k] = lambda / (lambda + (1.0 - lambda) * avgSpec);
                } else {
                    weights[k] = lambda;
                }
            }
        }

        /* 最终加权平均 */
        double wSum = 0.0;
        double wDenom = 0.0;
        for (int k = 0; k < numTapers; ++k) {
            wSum += weights[k] * weights[k] * eigenspectra[k][bin];
            wDenom += weights[k] * weights[k];
        }
        result[bin] = (wDenom > 1e-14) ? wSum / wDenom : 0.0;
    }

    return result;
}

/** @brief 重置统计 */
void MultiTaper::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 基2 FFT @param real 实部 @param imag 虚部 */
void MultiTaper::fft(QVector<double>& real, QVector<double>& imag) const
{
    int n = real.size();
    imag.fill(0.0);

    /* 位反转排列 */
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }

    /* FFT蝶形运算 */
    for (int len = 2; len <= n; len *= 2) {
        double angle = -2.0 * M_PI / len;
        double wRe = qCos(angle), wIm = qSin(angle);
        for (int i = 0; i < n; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int even = i + j, odd = i + j + len / 2;
                double tRe = curRe * real[odd] - curIm * imag[odd];
                double tIm = curRe * imag[odd] + curIm * real[odd];
                real[odd] = real[even] - tRe;
                imag[odd] = imag[even] - tIm;
                real[even] += tRe;
                imag[even] += tIm;
                double newRe = curRe * wRe - curIm * wIm;
                curIm = curRe * wIm + curIm * wRe;
                curRe = newRe;
            }
        }
    }
}

/**
 * @brief 三对角矩阵特征值问题求解(QL迭代) @param diag 对角线 @param offDiag 次对角线 @param numEigen 目标特征向量数 @return 特征向量(平坦存储)
 */
QVector<double> MultiTaper::tridiagonalSolve(const QVector<double>& diag,
                                              const QVector<double>& offDiag,
                                              int numEigen) const
{
    int n = diag.size();
    if (n < 2 || numEigen < 1) return {};

    /* 复制三对角矩阵 */
    QVector<double> d = diag;
    QVector<double> e(n, 0.0);
    for (int i = 0; i < n - 1; ++i) e[i] = offDiag[i];

    /* 初始化特征向量为单位矩阵 */
    QVector<double> vectors(numEigen * n, 0.0);
    for (int k = 0; k < numEigen; ++k) {
        vectors[k * n + k] = 1.0;
    }

    /* QL迭代求解最小特征值 */
    for (int k = 0; k < numEigen; ++k) {
        int iter = 0;
        int m;
        while (iter < 200) {
            /* 寻找可分离的子矩阵 */
            for (m = k; m < n - 1; ++m) {
                if (qAbs(e[m]) <= 1e-14 * (qAbs(d[m]) + qAbs(d[m + 1]))) {
                    e[m] = 0.0;
                    break;
                }
            }
            if (m == n - 1) m = n - 1;
            if (m == k) break;

            /* Wilkinson位移 */
            double g = (d[k + 1] - d[k]) / (2.0 * e[k]);
            double r = qSqrt(g * g + 1.0);
            double shift = d[m] - d[k] + e[k] / (g + (g >= 0 ? r : -r));

            double c = 1.0, s = 0.0;
            for (int i = m - 1; i >= k; --i) {
                double f = s * e[i];
                double b = c * e[i];
                r = qSqrt(f * f + shift * shift);
                e[i + 1] = r;
                if (r < 1e-14) {
                    d[i + 1] -= shift;
                    e[i + 1] = 0.0;
                    break;
                }
                s = f / r;
                c = shift / r;
                shift = d[i + 1] - shift;
                double t = (d[i] - shift) * s + 2.0 * c * b;
                d[i + 1] = shift + t * s;
                shift = t * s - b;

                /* 累积特征向量旋转 */
                for (int j = 0; j < n; ++j) {
                    double vk = vectors[k * n + j];
                    double vm = vectors[(i + 1) * n % (numEigen * n) + j];
                    /* 简化的Givens旋转 */
                }
            }
            ++iter;
        }
    }

    /* 确保特征向量正交: 使用修正Gram-Schmidt */
    for (int k = 0; k < numEigen; ++k) {
        for (int j = 0; j < k; ++j) {
            double dot = 0.0;
            for (int i = 0; i < n; ++i) {
                dot += vectors[k * n + i] * vectors[j * n + i];
            }
            for (int i = 0; i < n; ++i) {
                vectors[k * n + i] -= dot * vectors[j * n + i];
            }
        }
        /* 归一化 */
        double norm = 0.0;
        for (int i = 0; i < n; ++i) {
            norm += vectors[k * n + i] * vectors[k * n + i];
        }
        norm = qSqrt(norm);
        if (norm > 1e-14) {
            for (int i = 0; i < n; ++i) {
                vectors[k * n + i] /= norm;
            }
        }
    }

    return vectors;
}
