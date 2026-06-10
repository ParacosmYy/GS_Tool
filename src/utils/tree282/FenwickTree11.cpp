/**
 * @file FenwickTree11.cpp
 * @brief FenwickTree11 实现
 *
 * 实现树状数组：差分BIT的范围更新与点查询批量高效区间修改。
 */

#include "utils/tree282/FenwickTree11.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

FenwickTree11::FenwickTree11(QObject *parent)
    : QObject(parent) {}

FenwickTree11::~FenwickTree11() = default;

/* ---- Internal BIT operations ---- */

void FenwickTree11::internalUpdate(QVector<double>& bit, int i, double delta)
{
    // 1-indexed: propagate upward
    while (i <= m_n) {
        bit[i] += delta;
        i += i & (-i);  // Add lowest set bit
    }
}

double FenwickTree11::internalQuery(const QVector<double>& bit, int i) const
{
    double sum = 0.0;
    while (i > 0) {
        sum += bit[i];
        i -= i & (-i);  // Remove lowest set bit
    }
    return sum;
}

/* ---- Combined prefix sum: sum_{k=1}^{i} [BIT1[k]*(i+1) - BIT2[k]] ---- */

double FenwickTree11::combinedPrefixQuery(int i) const
{
    // For range-update/range-query variant:
    // prefix_sum(i) = (i+1) * query(BIT1, i) - query(BIT2, i)
    return static_cast<double>(i + 1) * internalQuery(m_bit1, i)
           - internalQuery(m_bit2, i);
}

/* ---- Initialize tree ---- */

void FenwickTree11::init(int n)
{
    m_n = qMax(0, n);
    m_bit1 = QVector<double>(m_n + 1, 0.0);  // 1-indexed
    m_bit2 = QVector<double>(m_n + 1, 0.0);
    m_stats.treeSize = m_n;
}

/* ---- Initialize from existing values ---- */

void FenwickTree11::initFromValues(const QVector<double>& values)
{
    int n = values.size();
    init(n);

    // Build difference array BIT: treat initial values as range updates on single points
    // Or build standard BIT by point updates
    for (int i = 0; i < n; ++i)
        pointUpdate(i, values[i]);
}

/* ---- Range update: add delta to [l, r] using difference BIT ---- */

void FenwickTree11::rangeUpdate(int l, int r, double delta)
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0 || l < 0 || r >= m_n || l > r) return;

    // Convert to 1-indexed
    l++; r++;

    // Range update via difference array on two BITs:
    // Update(l, delta) and Update(r+1, -delta) on BIT1
    // Update(l, delta*(l-1)) and Update(r+1, -delta*r) on BIT2
    internalUpdate(m_bit1, l, delta);
    internalUpdate(m_bit1, r + 1, -delta);
    internalUpdate(m_bit2, l, delta * (l - 1));
    internalUpdate(m_bit2, r + 1, -delta * r);

    double elapsed = timer.elapsed();
    m_stats.numRangeUpdates++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit rangeUpdateDone(l - 1, r - 1, delta, elapsed);
}

/* ---- Point query: get value at index i ---- */

double FenwickTree11::pointQuery(int i) const
{
    if (m_n == 0 || i < 0 || i >= m_n) return 0.0;

    // Point query in difference BIT = prefix sum at i
    // For pure range-update/point-query, only BIT1 is needed:
    // point(i) = query(BIT1, i+1)
    return internalQuery(m_bit1, i + 1);
}

/* ---- Point update: add delta at index i ---- */

void FenwickTree11::pointUpdate(int i, double delta)
{
    if (m_n == 0 || i < 0 || i >= m_n) return;

    // Point update = range update on single element [i, i]
    int idx1 = i + 1;  // 1-indexed
    internalUpdate(m_bit1, idx1, delta);
    internalUpdate(m_bit1, idx1 + 1, -delta);
    internalUpdate(m_bit2, idx1, delta * (idx1 - 1));
    internalUpdate(m_bit2, idx1 + 1, -delta * idx1);
}

/* ---- Prefix query: sum of [0, i] ---- */

double FenwickTree11::prefixQuery(int i) const
{
    if (m_n == 0 || i < 0) return 0.0;
    i = qMin(i, m_n - 1);
    return combinedPrefixQuery(i + 1);
}

/* ---- Range query: sum of [l, r] ---- */

double FenwickTree11::rangeQuery(int l, int r) const
{
    if (m_n == 0 || l < 0 || r >= m_n || l > r) return 0.0;
    m_stats.numPointQueries++;

    // Range sum = prefix(r) - prefix(l-1)
    double sumR = combinedPrefixQuery(r + 1);
    double sumLm1 = (l > 0) ? combinedPrefixQuery(l) : 0.0;
    return sumR - sumLm1;
}

/* ---- Batch range update ---- */

FenwickTree11::BatchResult FenwickTree11::batchRangeUpdate(
    const QVector<QVector<double>>& operations)
{
    QElapsedTimer timer;
    timer.start();

    BatchResult result;
    for (const auto& op : operations) {
        if (op.size() >= 3) {
            int l = static_cast<int>(op[0]);
            int r = static_cast<int>(op[1]);
            double delta = op[2];
            // Direct update without per-operation signal emission for speed
            if (m_n > 0 && l >= 0 && r < m_n && l <= r) {
                l++; r++;
                internalUpdate(m_bit1, l, delta);
                internalUpdate(m_bit1, r + 1, -delta);
                internalUpdate(m_bit2, l, delta * (l - 1));
                internalUpdate(m_bit2, r + 1, -delta * r);
                result.numOperations++;
                m_stats.numRangeUpdates++;
            }
        }
    }

    result.processingTimeMs = timer.elapsed();
    m_stats.totalOps++;
    m_timeSum += result.processingTimeMs;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit batchDone(result.numOperations, result.processingTimeMs);

    return result;
}

/* ---- Get all point values ---- */

QVector<double> FenwickTree11::getAllValues() const
{
    QVector<double> values(m_n, 0.0);
    for (int i = 0; i < m_n; ++i)
        values[i] = pointQuery(i);
    return values;
}

/* ---- Reset ---- */

void FenwickTree11::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_bit1.clear();
    m_bit2.clear();
    m_n = 0;
}
