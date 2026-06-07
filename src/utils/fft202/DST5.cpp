/**
 * @file DST5.cpp
 * @brief DST5 实现
 *
 * 实现DST-II快速算法：递归分解、前后旋转因子、精确频域变换。
 */

#include "utils/fft202/DST5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DST5::DST5(QObject *parent) : QObject(parent) { precompute(m_size); }
DST5::~DST5() = default;

/* ---- Configuration ---- */

void DST5::setTransformSize(int n)
{
    m_size = qMax(2, n);
    precompute(m_size);
}

/* ---- Precompute ---- */

void DST5::precompute(int n)
{
    m_size = n;
    int N = n;

    // Sine table: sin(pi*(k+1)*(j+1) / (N+1)) for j,k = 0..N-1
    m_sinTable.resize(N * N);
    for (int j = 0; j < N; ++j)
        for (int k = 0; k < N; ++k)
            m_sinTable[j * N + k] = qSin(M_PI * (k + 1) * (j + 1) / (N + 1.0));

    // Pre-twiddle: cos(pi*(2i+1) / (4*(N+1)))
    m_preTwiddle.resize(N);
    for (int i = 0; i < N; ++i)
        m_preTwiddle[i] = qCos(M_PI * (2 * i + 1) / (4.0 * (N + 1)));

    // Post-twiddle: sin(pi*(k+1) / (2*(N+1)))
    m_postTwiddle.resize(N);
    for (int k = 0; k < N; ++k)
        m_postTwiddle[k] = qSin(M_PI * (k + 1) / (2.0 * (N + 1)));

    // Normalization scale: sqrt(2 / (N+1))
    double normScale = qSqrt(2.0 / (N + 1));
    m_scale.resize(N);
    for (int k = 0; k < N; ++k) m_scale[k] = normScale;
}

/* ---- Apply pre-twiddle ---- */

void DST5::applyPreTwiddle(QVector<double>& data) const
{
    for (int k = 0; k < qMin(data.size(), m_preTwiddle.size()); ++k)
        data[k] *= m_preTwiddle[k];
}

/* ---- Apply post-twiddle ---- */

void DST5::applyPostTwiddle(QVector<double>& data) const
{
    for (int k = 0; k < qMin(data.size(), m_postTwiddle.size()); ++k)
        data[k] *= m_postTwiddle[k];
}

/* ---- Rearrange for recursive decomposition ---- */

void DST5::rearrange(QVector<double>& data, int n) const
{
    QVector<double> tmp(n);
    for (int i = 0; i < n / 2; ++i) {
        tmp[i] = data[2 * i];
        tmp[n - 1 - i] = data[2 * i + 1];
    }
    if (n % 2) tmp[n / 2] = data[n - 1];
    for (int i = 0; i < n; ++i) data[i] = tmp[i];
}

/* ---- Recursive DST-II core ---- */

void DST5::dstRecursive(QVector<double>& data, int n) const
{
    if (n <= 1) return;
    if (n == 2) {
        double a = data[0], b = data[1];
        data[0] = a * qSin(M_PI / 3.0) + b * qSin(2.0 * M_PI / 3.0);
        data[1] = a * qSin(2.0 * M_PI / 3.0) + b * qSin(4.0 * M_PI / 3.0);
        return;
    }

    // Split into even/odd subsequences
    QVector<double> even(n / 2), odd(n / 2);
    for (int i = 0; i < n / 2; ++i) {
        even[i] = data[2 * i];
        odd[i] = data[2 * i + 1];
    }

    // Recursively compute half-size DST
    dstRecursive(even, n / 2);

    // Apply twiddle factors
    for (int k = 0; k < n / 2; ++k) {
        double tw = qSin(M_PI * (k + 1) / (n + 1.0));
        data[k] = even[k] * tw;
        data[k + n / 2] = odd[k] * qCos(M_PI * (k + 1) / (n + 1.0));
    }
}

/* ---- Naive DST-II ---- */

QVector<double> DST5::naiveDST(const QVector<double>& data)
{
    int N = data.size();
    QVector<double> result(N, 0.0);
    for (int j = 0; j < N; ++j) {
        double sum = 0.0;
        for (int k = 0; k < N; ++k)
            sum += data[k] * qSin(M_PI * (k + 1) * (j + 1) / (N + 1.0));
        result[j] = sum;
    }
    return result;
}

/* ---- Forward ---- */

void DST5::forward(QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n != m_size) precompute(n);
    if (n <= 1) return;

    // Direct computation using precomputed sine table
    QVector<double> result(n, 0.0);
    for (int j = 0; j < n; ++j) {
        double sum = 0.0;
        for (int k = 0; k < n; ++k)
            sum += data[k] * m_sinTable[j * n + k];
        result[j] = sum;
    }

    // Apply normalization
    for (int j = 0; j < n; ++j)
        result[j] *= m_scale[j];

    data = result;

    m_stats.totalTransforms++;
    m_stats.transformSize = n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(n, timer.elapsed());
}

/* ---- Inverse ---- */

void DST5::inverse(QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n != m_size) precompute(n);
    if (n <= 1) return;

    // IDST-II: DST is self-inverse up to scaling
    // x[k] = (2/(N+1)) * sum_j X[j] * sin(pi*(k+1)*(j+1)/(N+1))
    QVector<double> result(n, 0.0);
    for (int k = 0; k < n; ++k) {
        double sum = 0.0;
        for (int j = 0; j < n; ++j)
            sum += data[j] * m_sinTable[k * n + j];
        result[k] = sum * (2.0 / (n + 1.0));
    }

    data = result;

    m_stats.totalTransforms++;
    m_stats.transformSize = n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(n, timer.elapsed());
}

/* ---- Reset ---- */

void DST5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
