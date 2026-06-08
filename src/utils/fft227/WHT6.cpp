/**
 * @file WHT6.cpp
 * @brief WHT6 实现
 *
 * 实现Walsh-Hadamard变换：自然序快速Hadamard、Gray码列率转换。
 */

#include "utils/fft227/WHT6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

WHT6::WHT6(QObject *parent) : QObject(parent) {}
WHT6::~WHT6() = default;

/* ---- Gray code ---- */

int WHT6::grayCode(int val) const
{
    return val ^ (val >> 1);
}

/* ---- Bit reversal ---- */

int WHT6::bitReverse(int val, int bits) const
{
    int result = 0;
    for (int i = 0; i < bits; ++i) {
        result = (result << 1) | (val & 1);
        val >>= 1;
    }
    return result;
}

/* ---- Compute Gray code permutation ---- */

void WHT6::computeGrayPermutation()
{
    int n = m_n;
    m_grayPermute.resize(n);
    m_grayPermuteInv.resize(n);

    // For sequency ordering, index k maps to grayCode(bitReverse(k))
    for (int k = 0; k < n; ++k) {
        int br = bitReverse(k, m_stages);
        m_grayPermute[k] = grayCode(br);
    }

    // Build inverse permutation
    for (int k = 0; k < n; ++k)
        m_grayPermuteInv[m_grayPermute[k]] = k;
}

/* ---- In-place natural-order fast Hadamard transform ---- */

void WHT6::fastHadamard(QVector<double>& data) const
{
    int n = data.size();
    // Fast Walsh-Hadamard: butterfly stages
    for (int len = 1; len < n; len <<= 1) {
        for (int i = 0; i < n; i += (len << 1)) {
            for (int j = 0; j < len; ++j) {
                double a = data[i + j];
                double b = data[i + j + len];
                data[i + j] = a + b;
                data[i + j + len] = a - b;
            }
        }
    }
}

/* ---- Prepare ---- */

bool WHT6::prepare(int n)
{
    if (n < 2) return false;
    int test = n;
    while (test > 1) {
        if (test % 2 != 0) return false;
        test /= 2;
    }

    m_n = n;
    m_stages = 0;
    int tmp = n;
    while (tmp > 1) { tmp >>= 1; m_stages++; }

    m_stats.transformSize = n;
    computeGrayPermutation();
    return true;
}

/* ---- Forward WHT ---- */

QVector<double> WHT6::forward(const QVector<double>& input, Ordering order) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> data(m_n, 0.0);
    for (int i = 0; i < qMin(input.size(), m_n); ++i)
        data[i] = input[i];

    // Natural-order fast Hadamard transform
    const_cast<WHT6*>(this)->fastHadamard(data);

    // Convert to requested ordering
    if (order == SequencyOrder) {
        QVector<double> sequency(m_n);
        for (int k = 0; k < m_n; ++k)
            sequency[k] = data[m_grayPermute[k]];
        data = sequency;
    }

    // Normalize by 1/sqrt(N) for orthogonal transform
    double scale = 1.0 / qSqrt(static_cast<double>(m_n));
    for (int i = 0; i < m_n; ++i)
        data[i] *= scale;

    const_cast<WHT6*>(this)->m_stats.numForwards++;
    const_cast<WHT6*>(this)->m_stats.totalOps++;
    const_cast<WHT6*>(this)->m_timeSum += timer.elapsed();
    const_cast<WHT6*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;
    emit const_cast<WHT6*>(this)->transformCompleted(
        m_n, (order == SequencyOrder) ? "sequency" : "natural", timer.elapsed());
    return data;
}

/* ---- Inverse WHT ---- */

QVector<double> WHT6::inverse(const QVector<double>& input, Ordering order) const
{
    // WHT is self-inverse (with 1/sqrt(N) normalization applied)
    return forward(input, order);
}

/* ---- To sequency order ---- */

QVector<double> WHT6::toSequencyOrder(const QVector<double>& natural) const
{
    int n = qMin(natural.size(), m_n);
    QVector<double> result(m_n, 0.0);
    for (int k = 0; k < n; ++k)
        result[k] = natural[m_grayPermute[k]];
    return result;
}

/* ---- To natural order ---- */

QVector<double> WHT6::toNaturalOrder(const QVector<double>& sequency) const
{
    int n = qMin(sequency.size(), m_n);
    QVector<double> result(m_n, 0.0);
    for (int k = 0; k < n; ++k)
        result[m_grayPermute[k]] = sequency[k];
    return result;
}

/* ---- Sequency table ---- */

QVector<int> WHT6::sequencyTable() const
{
    QVector<int> table(m_n);
    for (int k = 0; k < m_n; ++k) {
        int br = bitReverse(k, m_stages);
        int g = grayCode(br);
        // Count zero crossings (sign changes) in Walsh function
        int seq = 0;
        for (int b = m_stages - 1; b > 0; --b)
            if (((g >> b) & 1) != ((g >> (b - 1)) & 1)) seq++;
        table[k] = seq;
    }
    return table;
}

/* ---- Reset ---- */

void WHT6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_grayPermute.clear();
    m_grayPermuteInv.clear();
    m_n = 0;
    m_stages = 0;
}
