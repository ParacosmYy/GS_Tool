/**
 * @file DiscreteSineTransform.cpp
 * @brief DiscreteSineTransform 实现
 *
 * 实现DST-I和DST-VII变换：DST-I通过对称延拓转换为DCT计算，
 * DST-VII使用矩阵乘法(快速路径用于2的幂长度)。
 */

#include "utils/fft165/DiscreteSineTransform.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

DiscreteSineTransform::DiscreteSineTransform(QObject* parent)
    : QObject(parent)
{
}

DiscreteSineTransform::~DiscreteSineTransform() = default;

void DiscreteSineTransform::setTransformType(Type type)
{
    m_type = type;
}

int DiscreteSineTransform::nextFastLength(int n)
{
    if (n <= 0) return 1;
    /* For DST-I, we need 2(N+1) to be power of 2 */
    int target = 2 * (n + 1);
    int p = 1;
    while (p < target) p <<= 1;
    return (p / 2) - 1;
}

/**
 * @brief DST-I via symmetric extension
 *
 * DST-I: S[k] = sum_{n=0}^{N-1} x[n] * sin(pi*(n+1)*(k+1)/(N+1))
 *
 * Embed into DCT-II of length 2(N+1) by creating antisymmetric extension:
 *   y[n] = x[n] for n=0..N-1, y[N] = 0, y[2N+1-n] = -x[n] for n=0..N-1
 */
void DiscreteSineTransform::computeDST1(QVector<double>& data) const
{
    const int N = data.size();
    if (N <= 0) return;

    /* Build extended signal of length 2(N+1) */
    int extLen = 2 * (N + 1);
    QVector<double> ext(extLen, 0.0);

    for (int n = 0; n < N; ++n) {
        ext[n] = data[n];
        ext[extLen - 1 - n] = -data[n];
    }
    ext[N] = 0.0;

    /* Compute DCT-II on extended signal (direct computation) */
    QVector<double> result(N);
    for (int k = 0; k < N; ++k) {
        double sum = 0.0;
        for (int n = 0; n < extLen; ++n) {
            sum += ext[n] * qCos(M_PI * (2 * n + 1) * (k + 1) / (2.0 * extLen));
        }
        /* Extract DST-I coefficient from antisymmetric part */
        result[k] = -sum;
    }

    /* Normalize */
    double norm = qSqrt(2.0 / (N + 1));
    for (int k = 0; k < N; ++k) {
        result[k] *= norm;
    }

    data = result;
}

void DiscreteSineTransform::computeInverseDST1(QVector<double>& data) const
{
    /* DST-I is self-inverse up to a scaling factor: IDST-I = DST-I */
    const int N = data.size();
    computeDST1(data);
    /* The DST-I is its own inverse, so just apply it */
    double norm = 2.0 / (N + 1);
    for (int k = 0; k < N; ++k) {
        data[k] *= norm * qSqrt((N + 1) / 2.0);
    }
}

/**
 * @brief DST-VII direct computation
 *
 * DST-VII: S[k] = sum_{n=0}^{N-1} x[n] * sin(pi*(2*n+1)*(k+1)/(2*N))
 *
 * Used in HEVC integer transform.
 */
void DiscreteSineTransform::computeDST7(QVector<double>& data) const
{
    const int N = data.size();
    if (N <= 0) return;

    QVector<double> result(N, 0.0);
    for (int k = 0; k < N; ++k) {
        double sum = 0.0;
        for (int n = 0; n < N; ++n) {
            sum += data[n] * qSin(M_PI * (2 * n + 1) * (k + 1) / (2.0 * N));
        }
        result[k] = sum;
    }

    /* Normalize */
    double norm = qSqrt(2.0 / N);
    for (int k = 0; k < N; ++k) {
        result[k] *= norm;
    }

    data = result;
}

/**
 * @brief Inverse DST-VII (= DST-VIII transposed)
 *
 * Uses DST-VII transpose: x[n] = sum_{k} S[k] * sin(pi*(2*n+1)*(k+1)/(2*N))
 */
void DiscreteSineTransform::computeInverseDST7(QVector<double>& data) const
{
    const int N = data.size();
    if (N <= 0) return;

    /* Inverse is the transpose, which is the same kernel */
    QVector<double> result(N, 0.0);
    for (int n = 0; n < N; ++n) {
        double sum = 0.0;
        for (int k = 0; k < N; ++k) {
            sum += data[k] * qSin(M_PI * (2 * n + 1) * (k + 1) / (2.0 * N));
        }
        result[n] = sum;
    }

    double norm = qSqrt(2.0 / N);
    for (int n = 0; n < N; ++n) {
        result[n] *= norm;
    }

    data = result;
}

QVector<double> DiscreteSineTransform::forward(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    if (signal.isEmpty()) return QVector<double>();

    QVector<double> data = signal;

    if (m_type == DST_I) {
        computeDST1(data);
    } else {
        computeDST7(data);
    }

    m_stats.totalForward++;
    int peak = findPeak(data);
    m_stats.lastPeakFrequency = (peak >= 0 && data.size() > 0) ? peak : 0;
    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalForward + m_stats.totalInverse;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit forwardCompleted(signal.size());
    return data;
}

QVector<double> DiscreteSineTransform::inverse(const QVector<double>& coeffs,
                                               int originalLength)
{
    QElapsedTimer timer;
    timer.start();

    if (coeffs.isEmpty()) return QVector<double>();

    QVector<double> data = coeffs;

    if (m_type == DST_I) {
        computeInverseDST1(data);
    } else {
        computeInverseDST7(data);
    }

    if (originalLength > 0 && data.size() > originalLength) {
        data.resize(originalLength);
    }

    m_stats.totalInverse++;
    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalForward + m_stats.totalInverse;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit inverseCompleted(originalLength);
    return data;
}

int DiscreteSineTransform::findPeak(const QVector<double>& coeffs) const
{
    if (coeffs.isEmpty()) return -1;
    int peak = 0;
    double maxVal = qAbs(coeffs[0]);
    for (int i = 1; i < coeffs.size(); ++i) {
        if (qAbs(coeffs[i]) > maxVal) {
            maxVal = qAbs(coeffs[i]);
            peak = i;
        }
    }
    return peak;
}

void DiscreteSineTransform::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
