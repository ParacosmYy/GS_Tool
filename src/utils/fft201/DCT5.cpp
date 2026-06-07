/**
 * @file DCT5.cpp
 * @brief DCT5 实现
 *
 * 实现DCT-II快速算法：递归分解、正交归一化前后乘因子、精确频域变换。
 */

#include "utils/fft201/DCT5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DCT5::DCT5(QObject *parent) : QObject(parent) { precompute(m_size); }
DCT5::~DCT5() = default;

/* ---- Configuration ---- */

void DCT5::setTransformSize(int n)
{
    m_size = qMax(2, n);
    precompute(m_size);
}

/* ---- Precompute ---- */

void DCT5::precompute(int n)
{
    m_size = n;
    int N = n;

    // Cosine table: cos(pi*(2k+1)*j / 2N) for j,k = 0..N-1
    m_cosTable.resize(N * N);
    for (int j = 0; j < N; ++j)
        for (int k = 0; k < N; ++k)
            m_cosTable[j * N + k] = qCos(M_PI * (2 * k + 1) * j / (2.0 * N));

    // Pre-multiply: sqrt(2/N) for k=0 -> sqrt(1/N)
    m_preScale.resize(N);
    m_postScale.resize(N);
    double sqrtInvN = qSqrt(1.0 / N);
    double sqrt2InvN = qSqrt(2.0 / N);
    m_preScale[0] = sqrtInvN;
    m_postScale[0] = sqrtInvN;
    for (int k = 1; k < N; ++k) {
        m_preScale[k] = sqrt2InvN;
        m_postScale[k] = sqrt2InvN;
    }
}

/* ---- Apply pre-multiply ---- */

void DCT5::applyPreMultiply(QVector<double>& data) const
{
    for (int k = 0; k < qMin(data.size(), m_preScale.size()); ++k)
        data[k] *= m_preScale[k];
}

/* ---- Apply post-multiply ---- */

void DCT5::applyPostMultiply(QVector<double>& data) const
{
    for (int k = 0; k < qMin(data.size(), m_postScale.size()); ++k)
        data[k] *= m_postScale[k];
}

/* ---- Rearrange for recursive decomposition ---- */

void DCT5::rearrange(QVector<double>& data, int n) const
{
    QVector<double> tmp(n);
    for (int i = 0; i < n / 2; ++i) {
        tmp[i] = data[2 * i];             // Even indices
        tmp[n - 1 - i] = data[2 * i + 1]; // Odd indices reversed
    }
    for (int i = 0; i < n; ++i) data[i] = tmp[i];
}

/* ---- Recursive DCT-II core ---- */

void DCT5::dctRecursive(QVector<double>& data, int n) const
{
    if (n <= 1) return;
    if (n == 2) {
        double a = data[0], b = data[1];
        data[0] = a + b;
        data[1] = (a - b) * qCos(M_PI / 4.0) * qSqrt(2.0);
        return;
    }

    // Split into even and odd
    QVector<double> even(n / 2), odd(n / 2);
    for (int i = 0; i < n / 2; ++i) {
        even[i] = data[2 * i];
        odd[i] = data[2 * i + 1];
    }

    // Reverse odd part for symmetry
    QVector<double> oddRev(n / 2);
    for (int i = 0; i < n / 2; ++i) oddRev[i] = odd[n / 2 - 1 - i];

    // Combine: first half = even + odd, second half = even - odd
    QVector<double> first(n / 2), second(n / 2);
    for (int i = 0; i < n / 2; ++i) {
        first[i] = even[i] + oddRev[i];
        second[i] = even[i] - oddRev[i];
    }

    // Recursively compute half-size DCT
    dctRecursive(first, n / 2);

    // Apply twiddle factors and butterfly
    for (int k = 0; k < n / 2; ++k) {
        double angle = M_PI * (2 * k + 1) / (4.0 * (n / 2));
        double tw = qCos(angle);
        data[k] = first[k];
        data[k + n / 2] = second[k] * tw;
    }

    // Simple recursive merge for second half
    if (n > 4) {
        QVector<double> half(data.mid(n / 2, n / 2));
        dctRecursive(half, n / 2);
        for (int i = 0; i < n / 2; ++i) data[n / 2 + i] = half[i];
    }
}

/* ---- Naive DCT-II ---- */

QVector<double> DCT5::naiveDCT(const QVector<double>& data)
{
    int N = data.size();
    QVector<double> result(N, 0.0);
    for (int j = 0; j < N; ++j) {
        double sum = 0.0;
        for (int k = 0; k < N; ++k)
            sum += data[k] * qCos(M_PI * (2 * k + 1) * j / (2.0 * N));
        double scale = (j == 0) ? qSqrt(1.0 / N) : qSqrt(2.0 / N);
        result[j] = sum * scale;
    }
    return result;
}

/* ---- Forward ---- */

void DCT5::forward(QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n != m_size) precompute(n);
    if (n <= 1) return;

    // Use precomputed cosine table for direct computation (fast for moderate N)
    QVector<double> result(n, 0.0);
    for (int j = 0; j < n; ++j) {
        double sum = 0.0;
        for (int k = 0; k < n; ++k)
            sum += data[k] * m_cosTable[j * n + k];
        result[j] = sum;
    }

    // Apply orthogonal normalization (post-multiply)
    for (int j = 0; j < n; ++j)
        result[j] *= m_postScale[j];

    data = result;

    m_stats.totalTransforms++;
    m_stats.transformSize = n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(n, timer.elapsed());
}

/* ---- Inverse ---- */

void DCT5::inverse(QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n != m_size) precompute(n);
    if (n <= 1) return;

    // IDCT-II: apply pre-multiply, then transpose DCT
    for (int k = 0; k < n; ++k)
        data[k] *= m_preScale[k];

    // Transpose operation: sum over j
    QVector<double> result(n, 0.0);
    for (int k = 0; k < n; ++k) {
        double sum = 0.0;
        for (int j = 0; j < n; ++j)
            sum += data[j] * m_cosTable[j * n + k];
        result[k] = sum;
    }
    data = result;

    m_stats.totalTransforms++;
    m_stats.transformSize = n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(n, timer.elapsed());
}

/* ---- Reset ---- */

void DCT5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
