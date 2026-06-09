/**
 * @file WHT8.cpp
 * @brief WHT8 实现
 *
 * 实现沃尔什-哈达玛变换：自然序快速计算与序列号到自然序位逆序置换。
 */

#include "utils/fft255/WHT8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

WHT8::WHT8(QObject *parent) : QObject(parent) {}
WHT8::~WHT8() = default;

/* ---- Check power of 2 ---- */

bool WHT8::isPowerOf2(int n) { return n > 0 && (n & (n - 1)) == 0; }

/* ---- Bit-reversal ---- */

int WHT8::bitReverse(int x, int bits) const
{
    int rev = 0;
    for (int b = 0; b < bits; ++b) {
        if (x & (1 << b)) rev |= (1 << (bits - 1 - b));
    }
    return rev;
}

/* ---- Build sequency permutation via Gray code ---- */

void WHT8::buildSequencyPermutation()
{
    int n = m_size;
    if (n <= 0) return;

    int bits = 0;
    for (int tmp = n; tmp > 1; tmp >>= 1) bits++;

    // Sequency order: sort by number of sign changes (Gray code order)
    // Map: sequency index k -> natural index = bit_reverse(gray_code(k))
    m_sequencyPerm.resize(n);
    for (int k = 0; k < n; ++k) {
        // Compute Gray code of k
        int gray = k ^ (k >> 1);
        // Bit-reverse the Gray code to get natural index
        m_sequencyPerm[k] = bitReverse(gray, bits);
    }
}

/* ---- In-place fast WHT (butterfly) ---- */

void WHT8::fastWHT(QVector<double>& data) const
{
    int n = data.size();
    if (n <= 1) return;

    // Butterfly stages: O(N log N)
    for (int len = 2; len <= n; len <<= 1) {
        int halfLen = len / 2;
        for (int i = 0; i < n; i += len) {
            for (int j = 0; j < halfLen; ++j) {
                double a = data[i + j];
                double b = data[i + j + halfLen];
                data[i + j] = a + b;
                data[i + j + halfLen] = a - b;
            }
        }
    }

    // Normalize by 1/sqrt(N) for orthogonal WHT
    double norm = 1.0 / qSqrt(static_cast<double>(n));
    for (auto& v : data) v *= norm;
}

/* ---- Prepare ---- */

bool WHT8::prepare(int n, Order order)
{
    if (!isPowerOf2(n)) return false;
    m_size = n;
    m_order = order;
    buildSequencyPermutation();
    m_stats.transformSize = n;
    return true;
}

/* ---- Forward WHT ---- */

QVector<double> WHT8::forward(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    if (m_size <= 0) return {};

    QVector<double> data(m_size, 0.0);
    for (int i = 0; i < qMin(input.size(), m_size); ++i)
        data[i] = input[i];

    // Apply sequency permutation before butterfly if sequency output desired
    if (m_order == Sequency) {
        QVector<double> permuted(m_size, 0.0);
        for (int i = 0; i < m_size; ++i)
            permuted[i] = data[m_sequencyPerm[i]];
        data = permuted;
        m_stats.numPermutations++;
    }

    fastWHT(data);

    m_stats.numTransforms++;
    m_stats.totalOps++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit transformCompleted(m_size, static_cast<int>(m_order), elapsed);
    return data;
}

/* ---- Inverse WHT ---- */

QVector<double> WHT8::inverse(const QVector<double>& coefficients)
{
    QElapsedTimer timer;
    timer.start();

    if (m_size <= 0) return {};

    // WHT is self-inverse (up to normalization), so forward = inverse
    QVector<double> data(m_size, 0.0);
    for (int i = 0; i < qMin(coefficients.size(), m_size); ++i)
        data[i] = coefficients[i];

    fastWHT(data);

    // Undo sequency permutation if needed
    if (m_order == Sequency) {
        QVector<double> unpermuted(m_size, 0.0);
        for (int i = 0; i < m_size; ++i)
            unpermuted[m_sequencyPerm[i]] = data[i];
        data = unpermuted;
        m_stats.numPermutations++;
    }

    m_stats.numTransforms++;
    m_stats.totalOps++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit transformCompleted(m_size, static_cast<int>(m_order), elapsed);
    return data;
}

/* ---- Sequency permutation table ---- */

QVector<int> WHT8::sequencyPermutation() const
{
    return m_sequencyPerm;
}

/* ---- Reset ---- */

void WHT8::resetStatistics()
{
    m_sequencyPerm.clear();
    m_size = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
