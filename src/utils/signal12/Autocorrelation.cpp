/**
 * @file Autocorrelation.cpp
 * @brief 快速自相关分析引擎实现 — FFT加速 + 偏差校正
 */

#include "utils/signal12/Autocorrelation.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
Autocorrelation::Autocorrelation(QObject* parent)
    : QObject(parent)
    , m_sampleRate(1000.0)
    , m_biasMode(BiasMode::Normalized)
    , m_timeSum(0.0)
{
}

/** @brief 设置采样率 @param rate 采样率 */
void Autocorrelation::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

/** @brief 设置偏差校正模式 @param mode 校正模式 */
void Autocorrelation::setBiasMode(BiasMode mode)
{
    m_biasMode = mode;
}

/** @brief 计算完整自相关(基于FFT) @param data 输入信号 @return 自相关序列 */
QVector<double> Autocorrelation::compute(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n < 2) return {};

    /* 补零到2*N防止循环卷积混叠 */
    int fftSize = nextPowerOf2(2 * n);

    /* 构造FFT输入: 数据补零 */
    QVector<double> real(fftSize, 0.0);
    QVector<double> imag(fftSize, 0.0);
    for (int i = 0; i < n; ++i) {
        real[i] = data[i];
    }

    /* 正向FFT */
    fft(real, imag);

    /* 计算功率谱: |X(k)|^2 */
    for (int i = 0; i < fftSize; ++i) {
        double re = real[i], im = imag[i];
        real[i] = re * re + im * im;
        imag[i] = 0.0;
    }

    /* 逆FFT(取实部) = 自相关 */
    for (int i = 0; i < fftSize; ++i) {
        imag[i] = -imag[i]; /* 共轭 */
    }
    fft(real, imag);

    /* 提取有效部分(lag 0 ~ n-1)，归一化FFT结果 */
    QVector<double> autocorr(n, 0.0);
    double scale = 1.0 / static_cast<double>(fftSize);
    for (int i = 0; i < n; ++i) {
        autocorr[i] = real[i] * scale;
    }

    /* 偏差校正 */
    if (m_biasMode == BiasMode::Unbiased) {
        for (int i = 0; i < n; ++i) {
            if (n - i > 0) {
                autocorr[i] *= static_cast<double>(n) / static_cast<double>(n - i);
            }
        }
    } else if (m_biasMode == BiasMode::Normalized) {
        double r0 = autocorr[0];
        if (qAbs(r0) > 1e-15) {
            for (int i = 0; i < n; ++i) {
                autocorr[i] /= r0;
            }
        }
    }

    /* 更新统计 */
    m_stats.totalComputations++;
    m_stats.totalSamplesProcessed += n;
    double elapsed = static_cast<double>(timer.nsecsElapsed()) / 1e6;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalComputations);

    emit autocorrelationComputed(n, n - 1);
    return autocorr;
}

/** @brief 计算指定滞后范围的自相关 @param data 输入 @param maxLag 最大滞后 @return 自相关序列 */
QVector<double> Autocorrelation::computeUpToLag(const QVector<double>& data, int maxLag)
{
    QVector<double> full = compute(data);
    if (full.isEmpty()) return {};

    int effectiveLag = qMin(maxLag, full.size() - 1);
    return full.mid(0, effectiveLag + 1);
}

/** @brief 检测信号主周期 @param data 输入 @param minPeriod 最小周期 @param maxPeriod 最大周期 @return 结果 */
Autocorrelation::PeriodResult Autocorrelation::detectPeriod(
    const QVector<double>& data, int minPeriod, int maxPeriod)
{
    QVector<double> autocorr = compute(data);
    if (autocorr.isEmpty()) return {};

    int effectiveMax = (maxPeriod <= 0) ? autocorr.size() - 1
                                        : qMin(maxPeriod, autocorr.size() - 1);
    return detectPeriodFromAutocorr(autocorr, minPeriod, effectiveMax);
}

/** @brief 从自相关序列检测周期 @param autocorr 自相关 @param minLag 最小滞后 @param maxLag 最大滞后 @return 结果 */
Autocorrelation::PeriodResult Autocorrelation::detectPeriodFromAutocorr(
    const QVector<double>& autocorr, int minLag, int maxLag)
{
    PeriodResult result;
    if (autocorr.size() < 3) return result;

    int effectiveMax = (maxLag <= 0) ? autocorr.size() - 1
                                     : qMin(maxLag, autocorr.size() - 1);

    /* 寻找自相关的第一个显著峰值(在第一个零交叉之后) */
    double bestCorr = -1e10;
    int bestLag = -1;

    for (int lag = minLag; lag <= effectiveMax; ++lag) {
        /* 局部最大值检测 */
        if (lag > 0 && lag < autocorr.size() - 1) {
            if (autocorr[lag] > autocorr[lag - 1]
                && autocorr[lag] > autocorr[lag + 1]) {
                if (autocorr[lag] > bestCorr) {
                    bestCorr = autocorr[lag];
                    bestLag = lag;
                }
            }
        }
    }

    if (bestLag > 0) {
        result.period = static_cast<double>(bestLag);
        result.confidence = qBound(0.0, bestCorr, 1.0);
        result.frequency = m_sampleRate / result.period;
    }

    return result;
}

/** @brief 基2 FFT(原地) @param real 实部 @param imag 虚部 */
void Autocorrelation::fft(QVector<double>& real, QVector<double>& imag) const
{
    int n = real.size();
    imag.fill(0.0);

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
    for (int len = 2; len <= n; len *= 2) {
        double angle = -2.0 * M_PI / static_cast<double>(len);
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

/** @brief 补零到2的幂 @param n 当前大小 @return >= n的最小2的幂 */
int Autocorrelation::nextPowerOf2(int n)
{
    int p = 1;
    while (p < n) p *= 2;
    return p;
}

/** @brief 重置统计信息 */
void Autocorrelation::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
