/**
 * @file WienerFilter2.cpp
 * @brief 频域维纳滤波器实现 — 最优降噪
 * @author Serial Tool Team
 * @date 2026-06-05
 */

#include "utils/signal6/WienerFilter2.h"

#include <QElapsedTimer>
#include <QtMath>

/** @brief 构造函数
 *  @param fftSize FFT大小(默认512)
 *  @param parent 父对象 */
WienerFilter2::WienerFilter2(int fftSize, QObject *parent)
    : QObject(parent)
    , m_fftSize(nextPowerOf2(qMax(4, fftSize)))
    , m_noisePower(m_fftSize / 2 + 1, 0.0)
    , m_noiseEstimated(false)
{
}

/** @brief 从纯噪声样本估计噪声功率谱: 对噪声数据做FFT，计算幅度平方的期望
 *  @param noise 纯噪声数据(长度应>=fftSize) */
void WienerFilter2::estimateNoise(const QVector<double> &noise)
{
    if (noise.isEmpty()) return;

    int halfN = m_fftSize / 2 + 1;
    m_noisePower.fill(0.0);
    m_noisePower.resize(halfN);

    int numFrames = noise.size() / m_fftSize;
    if (numFrames < 1) numFrames = 1;

    /* 对每帧噪声计算功率谱并累加平均 */
    for (int f = 0; f < numFrames; ++f) {
        int start = f * m_fftSize;
        int len = qMin(m_fftSize, noise.size() - start);
        if (len < m_fftSize && f > 0) break;

        QVector<double> real(m_fftSize, 0.0), imag(m_fftSize, 0.0);
        for (int i = 0; i < len; ++i) {
            /* 汉宁窗 */
            double w = 0.5 * (1.0 - qCos(2.0 * M_PI * i / m_fftSize));
            real[i] = noise[start + i] * w;
        }

        fftImpl(real, imag, false);

        for (int k = 0; k < halfN; ++k) {
            double power = (real[k] * real[k] + imag[k] * imag[k])
                           / (m_fftSize * m_fftSize);
            m_noisePower[k] += power;
        }
    }

    /* 平均 */
    for (int k = 0; k < halfN; ++k) {
        m_noisePower[k] /= numFrames;
    }

    m_noiseEstimated = true;
}

/** @brief 对含噪信号进行维纳滤波: 分帧 -> FFT -> 维纳增益 -> IFFT -> 重叠相加
 *  @param signal 含噪信号
 *  @return 滤波后的信号 */
QVector<double> WienerFilter2::process(const QVector<double> &signal)
{
    QElapsedTimer timer;
    timer.start();

    if (signal.isEmpty() || !m_noiseEstimated) {
        return QVector<double>(signal);
    }

    int halfN = m_fftSize / 2 + 1;
    int hopSize = m_fftSize / 2; /* 50%重叠 */
    int numFrames = (signal.size() - m_fftSize) / hopSize + 1;
    if (numFrames < 1) numFrames = 1;

    QVector<double> output(signal.size(), 0.0);
    QVector<double> windowSum(signal.size(), 0.0);

    for (int f = 0; f < numFrames; ++f) {
        int start = f * hopSize;

        /* 加窗 */
        QVector<double> real(m_fftSize, 0.0), imag(m_fftSize, 0.0);
        for (int i = 0; i < m_fftSize && start + i < signal.size(); ++i) {
            double w = 0.5 * (1.0 - qCos(2.0 * M_PI * i / m_fftSize));
            real[i] = signal[start + i] * w;
        }

        fftImpl(real, imag, false);

        /* 应用维纳滤波器: H(k) = max(|X(k)|^2 - N(k), 0) / max(|X(k)|^2, eps) */
        for (int k = 0; k < halfN; ++k) {
            double sigPower = real[k] * real[k] + imag[k] * imag[k];
            double noisePow = m_noisePower[k] * m_fftSize * m_fftSize;

            /* 维纳增益 = (S - N) / S，下限0 */
            double gain = qMax(sigPower - noisePow, 0.0)
                          / qMax(sigPower, 1e-12);

            real[k] *= gain;
            imag[k] *= gain;
        }

        /* 对称填充: 共轭对称扩展到全频段 */
        for (int k = halfN; k < m_fftSize; ++k) {
            int mirror = m_fftSize - k;
            real[k] = real[mirror];
            imag[k] = -imag[mirror];
        }

        /* IFFT */
        fftImpl(real, imag, true);

        /* 重叠相加 */
        for (int i = 0; i < m_fftSize && start + i < signal.size(); ++i) {
            double w = 0.5 * (1.0 - qCos(2.0 * M_PI * i / m_fftSize));
            output[start + i] += real[i] / m_fftSize;
            windowSum[start + i] += w * w;
        }
    }

    /* 窗函数归一化 */
    for (int i = 0; i < output.size(); ++i) {
        if (windowSum[i] > 1e-12) {
            output[i] /= windowSum[i];
        }
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalProcessed;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalProcessed);

    emit processCompleted(numFrames, elapsed);
    return output;
}

/** @brief 手动设置噪声功率谱
 *  @param power 噪声功率谱密度(长度=fftSize/2+1) */
void WienerFilter2::setNoisePower(const QVector<double> &power)
{
    int halfN = m_fftSize / 2 + 1;
    m_noisePower = power;
    m_noisePower.resize(halfN);
    /* 填充不足部分为0 */
    for (int i = 0; i < halfN; ++i) {
        if (i >= power.size()) {
            m_noisePower[i] = 0.0;
        }
    }
    m_noiseEstimated = true;
}

/** @brief 设置FFT大小 @param size 必须是2的幂 */
void WienerFilter2::setFftSize(int size)
{
    m_fftSize = nextPowerOf2(qMax(4, size));
    m_noisePower.fill(0.0);
    m_noisePower.resize(m_fftSize / 2 + 1);
    m_noiseEstimated = false;
}

/** @brief 重置统计计数器 */
void WienerFilter2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 基2 FFT内部实现(就地)
 *  @param real 实部
 *  @param imag 虚部
 *  @param inverse 是否逆变换 */
void WienerFilter2::fftImpl(QVector<double> &real, QVector<double> &imag,
                            bool inverse) const
{
    int n = real.size();
    if (n <= 1) return;

    /* 位反转排列 */
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        if (i < j) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }

    /* 蝶形运算 */
    double dir = inverse ? 1.0 : -1.0;
    for (int len = 2; len <= n; len *= 2) {
        double angle = dir * 2.0 * M_PI / len;
        double wRe = qCos(angle), wIm = qSin(angle);

        for (int i = 0; i < n; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int even = i + j;
                int odd = i + j + len / 2;

                double tRe = curRe * real[odd] - curIm * imag[odd];
                double tIm = curRe * imag[odd] + curIm * real[odd];

                real[odd] = real[even] - tRe;
                imag[odd] = imag[even] - tIm;
                real[even] += tRe;
                imag[even] += tIm;

                double newRe = curRe * wRe - curIm * wIm;
                double newIm = curRe * wIm + curIm * wRe;
                curRe = newRe;
                curIm = newIm;
            }
        }
    }
}

/** @brief 求下一个2的幂 @param n 输入值 @return >=n的最小2的幂 */
int WienerFilter2::nextPowerOf2(int n)
{
    int p = 1;
    while (p < n) p *= 2;
    return p;
}
