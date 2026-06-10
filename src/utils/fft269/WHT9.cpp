/**
 * @file WHT9.cpp
 * @brief WHT9 实现
 *
 * 实现沃尔什-哈达玛变换：序列序快速计算与Paley序蝶形结构。
 */

#include "utils/fft269/WHT9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

WHT9::WHT9(QObject *parent)
    : QObject(parent) {}

WHT9::~WHT9() = default;

/* ---- Configuration ---- */

void WHT9::setOrdering(Ordering mode)
{
    m_ordering = mode;
}

/* ---- Utility ---- */

int WHT9::numStages(int n)
{
    int s = 0;
    while (n > 1) { n >>= 1; s++; }
    return s;
}

bool WHT9::isPow2(int n)
{
    return n > 0 && (n & (n - 1)) == 0;
}

int WHT9::grayCode(int x)
{
    return x ^ (x >> 1);
}

int WHT9::bitReverse(int x, int bits)
{
    int rev = 0;
    for (int b = 0; b < bits; ++b) {
        if (x & (1 << b)) rev |= (1 << (bits - 1 - b));
    }
    return rev;
}

int WHT9::paleyIndex(int idx, int bits)
{
    return bitReverse(idx, bits);
}

void WHT9::bitReversePerm(QVector<double>& data)
{
    int n = data.size();
    int bits = numStages(n);
    for (int i = 0; i < n; ++i) {
        int rev = bitReverse(i, bits);
        if (rev > i) std::swap(data[i], data[rev]);
    }
}

/* ---- Fast WHT (natural Hadamard order) ---- */

QVector<double> WHT9::transform(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    if (!isPow2(n)) return input;

    int stages = numStages(n);
    QVector<double> data = input;

    // Fast Walsh-Hadamard Transform: butterfly structure
    // Natural (Hadamard) order: in-place O(N log N)
    for (int s = 1; s <= stages; ++s) {
        int m = 1 << s;       // current block size
        int halfM = m >> 1;   // half block
        for (int k = 0; k < n; k += m) {
            for (int j = 0; j < halfM; ++j) {
                double a = data[k + j];
                double b = data[k + j + halfM];
                data[k + j] = a + b;
                data[k + j + halfM] = a - b;
            }
        }
    }

    // Scale by 1/sqrt(N) for orthonormal WHT
    double scale = 1.0 / qSqrt(static_cast<double>(n));
    for (int i = 0; i < n; ++i)
        data[i] *= scale;

    // Reorder to requested ordering
    QVector<double> result = data;
    if (m_ordering == Sequency) {
        result = toSequencyOrder(data);
    } else if (m_ordering == Paley) {
        result.resize(n);
        int bits = numStages(n);
        for (int i = 0; i < n; ++i)
            result[paleyIndex(i, bits)] = data[i];
    } else if (m_ordering == BitReversal) {
        result = data;
        bitReversePerm(result);
    }

    double elapsed = timer.elapsed();
    m_stats.transformSize = n;
    m_stats.numStages = stages;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit transformComputed(n, stages, elapsed);

    return result;
}

/* ---- Paley-ordered butterfly WHT ---- */

QVector<double> WHT9::transformPaley(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    if (!isPow2(n)) return input;

    int stages = numStages(n);
    QVector<double> data = input;

    // Bit-reverse input first for Paley ordering
    bitReversePerm(data);

    // Standard butterfly on bit-reversed input
    for (int s = 1; s <= stages; ++s) {
        int m = 1 << s;
        int halfM = m >> 1;
        for (int k = 0; k < n; k += m) {
            for (int j = 0; j < halfM; ++j) {
                double a = data[k + j];
                double b = data[k + j + halfM];
                data[k + j] = a + b;
                data[k + j + halfM] = a - b;
            }
        }
    }

    double scale = 1.0 / qSqrt(static_cast<double>(n));
    for (int i = 0; i < n; ++i)
        data[i] *= scale;

    double elapsed = timer.elapsed();
    m_stats.transformSize = n;
    m_stats.numStages = stages;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit transformComputed(n, stages, elapsed);

    return data;
}

/* ---- Inverse WHT (same as forward for orthonormal WHT) ---- */

QVector<double> WHT9::inverseTransform(const QVector<double>& spectrum)
{
    // Orthonormal WHT is its own inverse
    return transform(spectrum);
}

/* ---- Reorder to sequency (Walsh) order ---- */

QVector<double> WHT9::toSequencyOrder(const QVector<double>& natural) const
{
    int n = natural.size();
    int bits = numStages(n);
    QVector<double> result(n);

    // Sequency order: sort by Gray code of natural index
    for (int i = 0; i < n; ++i) {
        int seqIdx = grayCode(i);
        result[seqIdx] = natural[i];
    }
    return result;
}

/* ---- Reset ---- */

void WHT9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
