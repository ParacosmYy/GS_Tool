/**
 * @file WalshHadamard.cpp
 * @brief WalshHadamard 实现
 *
 * 实现快速Walsh-Hadamard变换：蝶形运算、自然序/序率序/位反转序排列。
 */

#include "utils/fft167/WalshHadamard.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

WalshHadamard::WalshHadamard(QObject* parent)
    : QObject(parent)
{
}

WalshHadamard::~WalshHadamard() = default;

void WalshHadamard::setOrdering(Ordering order)
{
    m_ordering = order;
}

int WalshHadamard::bitReverse(int x, int log2n)
{
    int result = 0;
    for (int i = 0; i < log2n; ++i) {
        result = (result << 1) | (x & 1);
        x >>= 1;
    }
    return result;
}

int WalshHadamard::grayCode(int x)
{
    return x ^ (x >> 1);
}

void WalshHadamard::fwhtCore(QVector<double>& data)
{
    int n = data.size();
    /* Fast Walsh-Hadamard Transform: butterfly operations */
    for (int step = 1; step < n; step <<= 1) {
        for (int i = 0; i < n; i += (step << 1)) {
            for (int j = i; j < i + step; ++j) {
                double a = data[j];
                double b = data[j + step];
                data[j] = a + b;
                data[j + step] = a - b;
            }
        }
    }
}

QVector<double> WalshHadamard::toSequencyOrder(const QVector<double>& data)
{
    int n = data.size();
    if (n <= 1) return data;

    /* Compute log2(n) */
    int log2n = 0;
    int tmp = n;
    while (tmp > 1) { tmp >>= 1; log2n++; }

    QVector<double> result(n);
    for (int i = 0; i < n; ++i) {
        /* Sequency order: index by Gray code of bit-reversed index */
        int br = bitReverse(i, log2n);
        int gray = grayCode(br);
        result[i] = (gray < n) ? data[gray] : 0.0;
    }
    return result;
}

QVector<double> WalshHadamard::toBitReversalOrder(const QVector<double>& data)
{
    int n = data.size();
    if (n <= 1) return data;

    int log2n = 0;
    int tmp = n;
    while (tmp > 1) { tmp >>= 1; log2n++; }

    QVector<double> result(n);
    for (int i = 0; i < n; ++i) {
        int br = bitReverse(i, log2n);
        result[i] = (br < n) ? data[br] : 0.0;
    }
    return result;
}

QVector<double> WalshHadamard::forward(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    if (n <= 1) return input;

    /* Ensure power of 2 */
    int sz = 1;
    while (sz < n) sz <<= 1;

    QVector<double> data(sz, 0.0);
    for (int i = 0; i < n; ++i) data[i] = input[i];

    /* Core FWHT */
    fwhtCore(data);

    /* Apply ordering */
    switch (m_ordering) {
    case Sequency:
        data = toSequencyOrder(data);
        break;
    case BitReversal:
        data = toBitReversalOrder(data);
        break;
    case Natural:
    default:
        break;
    }

    /* Normalize by 1/sqrt(N) for orthonormal transform */
    double norm = 1.0 / qSqrt(static_cast<double>(sz));
    for (int i = 0; i < sz; ++i)
        data[i] *= norm;

    /* Trim to original length */
    data.resize(n);

    m_stats.totalForward++;
    m_stats.lastTransformSize = sz;
    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalForward + m_stats.totalInverse;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit forwardCompleted(sz);
    return data;
}

QVector<double> WalshHadamard::inverse(const QVector<double>& coeffs)
{
    QElapsedTimer timer;
    timer.start();

    int n = coeffs.size();
    if (n <= 1) return coeffs;

    int sz = 1;
    while (sz < n) sz <<= 1;

    QVector<double> data(sz, 0.0);
    for (int i = 0; i < n; ++i) data[i] = coeffs[i];

    /* Reverse ordering if needed (inverse permutation) */
    switch (m_ordering) {
    case Sequency: {
        /* Inverse sequency: need to invert the mapping */
        int log2n = 0;
        int tmp = sz;
        while (tmp > 1) { tmp >>= 1; log2n++; }
        QVector<double> temp(sz);
        for (int i = 0; i < sz; ++i) {
            int br = bitReverse(i, log2n);
            int gray = grayCode(br);
            if (gray < sz) temp[gray] = data[i];
        }
        data = temp;
        break;
    }
    case BitReversal: {
        int log2n = 0;
        int tmp = sz;
        while (tmp > 1) { tmp >>= 1; log2n++; }
        QVector<double> temp(sz);
        for (int i = 0; i < sz; ++i) {
            int br = bitReverse(i, log2n);
            if (br < sz) temp[br] = data[i];
        }
        data = temp;
        break;
    }
    default:
        break;
    }

    /* Denormalize */
    double norm = qSqrt(static_cast<double>(sz));
    for (int i = 0; i < sz; ++i)
        data[i] *= norm;

    /* FWHT is self-inverse (same butterfly) */
    fwhtCore(data);

    /* Normalize by 1/N */
    for (int i = 0; i < sz; ++i)
        data[i] /= sz;

    data.resize(n);

    m_stats.totalInverse++;
    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalForward + m_stats.totalInverse;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit inverseCompleted(sz);
    return data;
}

void WalshHadamard::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
