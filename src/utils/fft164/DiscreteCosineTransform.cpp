/**
 * @file DiscreteCosineTransform.cpp
 * @brief DiscreteCosineTransform 实现
 *
 * 实现Lee快速DCT-II/III算法：基于蝶形分解的递归DCT，
 * 类似FFT的O(N log N)复杂度。
 */

#include "utils/fft164/DiscreteCosineTransform.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
DiscreteCosineTransform::DiscreteCosineTransform(QObject* parent)
    : QObject(parent)
{
}

DiscreteCosineTransform::~DiscreteCosineTransform() = default;

/**
 * @brief 补零到2的幂
 */
int DiscreteCosineTransform::nextPowerOf2(int n)
{
    if (n <= 0) return 1;
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

/**
 * @brief Lee快速DCT-II递归实现
 *
 * DCT-II: X[k] = sum_{n=0}^{N-1} x[n] * cos(pi*(2n+1)*k / (2N))
 *
 * Lee分解: 将N点DCT分为两个N/2点DCT:
 *   偶数部分: y_e[n] = x[n] + x[N-1-n]
 *   奇数部分: y_o[n] = (x[n] - x[N-1-n]) / (2*cos(pi*(2n+1)/(2N)))
 * 然后 X[k] = DCT(y_e)[k] + DCT(y_o)[k] for even k
 */
void DiscreteCosineTransform::fastDCT2(QVector<double>& data, int start, int len) const
{
    if (len <= 1) return;

    const int half = len / 2;
    QVector<double> even(half), odd(half);

    for (int i = 0; i < half; ++i) {
        even[i] = data[start + i] + data[start + len - 1 - i];
    }

    for (int i = 0; i < half; ++i) {
        double cosFactor = qCos(M_PI * (2 * i + 1) / (2.0 * len));
        double diff = data[start + i] - data[start + len - 1 - i];
        odd[i] = diff / (2.0 * cosFactor);
    }

    fastDCT2(even, 0, half);
    fastDCT2(odd, 0, half);

    /* Interleave results */
    for (int i = 0; i < half; ++i) {
        data[start + 2 * i] = even[i];
        data[start + 2 * i + 1] = odd[i];
    }

    /* Unfold: combine even/odd DCT results */
    QVector<double> result(len);
    for (int k = 0; k < half; ++k) {
        result[2 * k] = even[k] + odd[k];
    }
    for (int k = 0; k < half; ++k) {
        result[2 * k + 1] = even[k] - odd[k];
    }
    for (int i = 0; i < len; ++i) {
        data[start + i] = result[i];
    }
}

/**
 * @brief Lee快速DCT-III递归实现
 *
 * DCT-III (IDCT): x[n] = (1/2)*X[0] + sum_{k=1}^{N-1} X[k] * cos(pi*k*(2n+1)/(2N))
 *
 * Inverse of Lee decomposition.
 */
void DiscreteCosineTransform::fastDCT3(QVector<double>& data, int start, int len) const
{
    if (len <= 1) return;

    const int half = len / 2;
    QVector<double> even(half), odd(half);

    for (int k = 0; k < half; ++k) {
        even[k] = data[start + 2 * k] + data[start + 2 * k + 1];
        odd[k] = data[start + 2 * k] - data[start + 2 * k + 1];
    }

    fastDCT3(even, 0, half);
    fastDCT3(odd, 0, half);

    for (int n = 0; n < half; ++n) {
        double cosFactor = qCos(M_PI * (2 * n + 1) / (2.0 * len));
        data[start + n] = even[n] + 2.0 * cosFactor * odd[n];
    }
    for (int n = 0; n < half; ++n) {
        double cosFactor = qCos(M_PI * (2 * (half - 1 - n) + 1) / (2.0 * len));
        data[start + len - 1 - n] = even[half - 1 - n] - 2.0 * cosFactor * odd[half - 1 - n];
    }
}

/**
 * @brief DCT-II正变换
 */
QVector<double> DiscreteCosineTransform::forward(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    if (signal.isEmpty()) return QVector<double>();

    int paddedLen = nextPowerOf2(signal.size());
    QVector<double> data(paddedLen, 0.0);
    for (int i = 0; i < signal.size(); ++i) {
        data[i] = signal[i];
    }

    fastDCT2(data, 0, paddedLen);

    /* Scale: standard DCT-II normalization */
    data[0] *= qSqrt(1.0 / paddedLen);
    for (int i = 1; i < paddedLen; ++i) {
        data[i] *= qSqrt(2.0 / paddedLen);
    }

    /* Compute energy compaction */
    m_stats.energyCompaction = energyCompaction(data);

    m_stats.totalForward++;
    m_timeSum += timer.elapsed();
    quint64 totalOps = m_stats.totalForward + m_stats.totalInverse;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit forwardCompleted(signal.size());
    return data;
}

/**
 * @brief DCT-III逆变换
 */
QVector<double> DiscreteCosineTransform::inverse(const QVector<double>& coeffs, int originalLength)
{
    QElapsedTimer timer;
    timer.start();

    if (coeffs.isEmpty()) return QVector<double>();

    int paddedLen = nextPowerOf2(coeffs.size());
    QVector<double> data(paddedLen, 0.0);

    /* Inverse scaling */
    for (int i = 0; i < qMin(coeffs.size(), paddedLen); ++i) {
        if (i == 0) {
            data[i] = coeffs[i] * qSqrt(1.0 / paddedLen);
        } else {
            data[i] = coeffs[i] * qSqrt(2.0 / paddedLen);
        }
    }

    fastDCT3(data, 0, paddedLen);

    /* Truncate to original length */
    if (originalLength > 0 && data.size() > originalLength) {
        data.resize(originalLength);
    }

    m_stats.totalInverse++;
    m_timeSum += timer.elapsed();
    quint64 totalOps = m_stats.totalForward + m_stats.totalInverse;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit inverseCompleted(originalLength);
    return data;
}

/**
 * @brief 频带截断
 */
QVector<double> DiscreteCosineTransform::truncate(const QVector<double>& signal, int keepCoeffs)
{
    if (signal.isEmpty()) return QVector<double>();

    QVector<double> coeffs = forward(signal);
    int keep = qMin(keepCoeffs, coeffs.size());

    /* Zero out high-frequency coefficients */
    for (int i = keep; i < coeffs.size(); ++i) {
        coeffs[i] = 0.0;
    }

    return inverse(coeffs, signal.size());
}

/**
 * @brief 计算能量集中度(前1/4系数能量占总能量比)
 */
double DiscreteCosineTransform::energyCompaction(const QVector<double>& coeffs) const
{
    if (coeffs.isEmpty()) return 0.0;

    double totalEnergy = 0.0;
    double quarterEnergy = 0.0;
    int quarter = qMax(1, coeffs.size() / 4);

    for (int i = 0; i < coeffs.size(); ++i) {
        double e = coeffs[i] * coeffs[i];
        totalEnergy += e;
        if (i < quarter) quarterEnergy += e;
    }

    return (totalEnergy > 1e-20) ? quarterEnergy / totalEnergy : 0.0;
}

void DiscreteCosineTransform::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
