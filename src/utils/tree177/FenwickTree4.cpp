/**
 * @file FenwickTree4.cpp
 * @brief FenwickTree4 实现
 *
 * 实现Fenwick树：1D/2D变体、区间更新/查询、k阶统计量、逆序对。
 */

#include "utils/tree177/FenwickTree4.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

FenwickTree4::FenwickTree4(QObject *parent)
    : QObject(parent)
{
}

FenwickTree4::~FenwickTree4() = default;

/* ---- Initialize 1D ---- */

void FenwickTree4::init1D(int size)
{
    m_size = qMax(1, size);
    m_tree.resize(m_size + 1, 0.0);
    m_tree2.resize(m_size + 1, 0.0);
    m_rows = 0;
    m_cols = 0;
    m_stats.dimensions = 1;
    m_stats.treeSize = m_size;
}

/* ---- Build from array ---- */

void FenwickTree4::build(const QVector<double>& data)
{
    int n = data.size();
    init1D(n);

    for (int i = 0; i < n; ++i) {
        /* Direct BIT construction: O(n) */
        m_tree[i + 1] += data[i];
        int j = i + 1 + lsb(i + 1);
        if (j <= m_size)
            m_tree[j] += m_tree[i + 1];
    }
}

/* ---- Initialize 2D ---- */

void FenwickTree4::init2D(int rows, int cols)
{
    m_rows = qMax(1, rows);
    m_cols = qMax(1, cols);
    m_tree2D.resize(m_rows + 1);
    for (int i = 0; i <= m_rows; ++i)
        m_tree2D[i].resize(m_cols + 1, 0.0);
    m_size = 0;
    m_stats.dimensions = 2;
    m_stats.treeSize = m_rows * m_cols;
}

/* ---- 1D: Point update ---- */

void FenwickTree4::update(int idx, double delta)
{
    /* BIT is 1-indexed internally */
    for (int i = idx + 1; i <= m_size; i += lsb(i))
        m_tree[i] += delta;

    m_stats.totalUpdates++;
}

/* ---- 1D: Prefix sum [1..idx] ---- */

double FenwickTree4::prefixSum(int idx) const
{
    double sum = 0.0;
    for (int i = idx + 1; i > 0; i -= lsb(i))
        sum += m_tree[i];

    const_cast<FenwickTree4*>(this)->m_stats.totalQueries++;
    return sum;
}

/* ---- 1D: Range sum [l..r] ---- */

double FenwickTree4::rangeSum(int l, int r) const
{
    if (l > r) return 0.0;
    double result = prefixSum(r) - (l > 0 ? prefixSum(l - 1) : 0.0);
    emit const_cast<FenwickTree4*>(this)->queryCompleted(result);
    return result;
}

/* ---- Range update [l..r] += delta ---- */

void FenwickTree4::rangeUpdate(int l, int r, double delta)
{
    /* Uses two BITs for range update / point query or range query */
    /* BIT1: for range update, standard trick */
    for (int i = l + 1; i <= m_size; i += lsb(i))
        m_tree[i] += delta;
    if (r + 2 <= m_size)
        for (int i = r + 2; i <= m_size; i += lsb(i))
            m_tree[i] -= delta;

    /* For range query support, update second BIT */
    for (int i = l + 1; i <= m_size; i += lsb(i))
        m_tree2[i] += delta * l;
    if (r + 2 <= m_size)
        for (int i = r + 2; i <= m_size; i += lsb(i))
            m_tree2[i] -= delta * (r + 1);

    m_stats.totalUpdates++;
}

/* ---- Point query (with range updates) ---- */

double FenwickTree4::pointQuery(int idx) const
{
    double sum = 0.0;
    for (int i = idx + 1; i > 0; i -= lsb(i))
        sum += m_tree[i];
    const_cast<FenwickTree4*>(this)->m_stats.totalQueries++;
    return sum;
}

/* ---- Range query [l..r] (with range updates) ---- */

double FenwickTree4::rangeQuery(int l, int r) const
{
    /* Standard BIT range query */
    return prefixSum(r) - (l > 0 ? prefixSum(l - 1) : 0.0);
}

/* ---- 2D: Point update ---- */

void FenwickTree4::update2D(int row, int col, double delta)
{
    for (int i = row + 1; i <= m_rows; i += lsb(i))
        for (int j = col + 1; j <= m_cols; j += lsb(j))
            m_tree2D[i][j] += delta;

    m_stats.totalUpdates++;
}

/* ---- 2D: Prefix sum ---- */

double FenwickTree4::prefixSum2D(int row, int col) const
{
    double sum = 0.0;
    for (int i = row + 1; i > 0; i -= lsb(i))
        for (int j = col + 1; j > 0; j -= lsb(j))
            sum += m_tree2D[i][j];

    const_cast<FenwickTree4*>(this)->m_stats.totalQueries++;
    return sum;
}

/* ---- 2D: Rectangle sum ---- */

double FenwickTree4::rectSum(int r1, int c1, int r2, int c2) const
{
    /* Inclusion-exclusion */
    double a = prefixSum2D(r2, c2);
    double b = (r1 > 0) ? prefixSum2D(r1 - 1, c2) : 0.0;
    double c = (c1 > 0) ? prefixSum2D(r2, c1 - 1) : 0.0;
    double d = (r1 > 0 && c1 > 0) ? prefixSum2D(r1 - 1, c1 - 1) : 0.0;

    double result = a - b - c + d;
    emit const_cast<FenwickTree4*>(this)->queryCompleted(result);
    return result;
}

/* ---- k-th order statistic ---- */

int FenwickTree4::findKth(int k) const
{
    /* Works for non-negative integer frequency BIT */
    /* Binary search on BIT */
    int pos = 0;
    int bitMask = 1;
    while (bitMask <= m_size) bitMask <<= 1;
    bitMask >>= 1;

    for (; bitMask > 0; bitMask >>= 1) {
        int next = pos + bitMask;
        if (next <= m_size && m_tree[next] < k) {
            k -= static_cast<int>(m_tree[next]);
            pos = next;
        }
    }
    return pos; /* 0-indexed result */
}

/* ---- Count inversions ---- */

qint64 FenwickTree4::countInversions(QVector<double> data) const
{
    int n = data.size();
    if (n <= 1) return 0;

    /* Coordinate compression */
    QVector<double> sorted = data;
    std::sort(sorted.begin(), sorted.end());
    sorted.erase(std::unique(sorted.begin(), sorted.end()), sorted.end());

    /* Create a temporary BIT for counting */
    QVector<double> bit(sorted.size() + 1, 0.0);
    qint64 inversions = 0;

    /* Process right to left */
    for (int i = n - 1; i >= 0; --i) {
        /* Find compressed index */
        int idx = std::lower_bound(sorted.begin(), sorted.end(), data[i])
                  - sorted.begin();

        /* Query count of elements smaller than current */
        double cnt = 0.0;
        for (int j = idx; j > 0; j -= lsb(j))
            cnt += bit[j];

        inversions += static_cast<qint64>(i - cnt -
                     (n - 1 - i - (static_cast<int>(prefixSum(n - 1)) - static_cast<int>(cnt))));

        /* Update BIT */
        for (int j = idx + 1; j <= static_cast<int>(sorted.size()); j += lsb(j))
            bit[j] += 1.0;
    }

    /* Recalculate properly */
    inversions = 0;
    bit.fill(0.0);
    for (int i = n - 1; i >= 0; --i) {
        int idx = std::lower_bound(sorted.begin(), sorted.end(), data[i])
                  - sorted.begin();
        for (int j = idx; j > 0; j -= lsb(j))
            inversions += static_cast<qint64>(bit[j]);
        for (int j = idx + 1; j <= static_cast<int>(sorted.size()); j += lsb(j))
            bit[j] += 1.0;
    }

    return inversions;
}

/* ---- Clear ---- */

void FenwickTree4::clear()
{
    m_tree.fill(0.0);
    m_tree2.fill(0.0);
    for (int i = 0; i <= m_rows; ++i)
        m_tree2D[i].fill(0.0);
}

/* ---- Reset statistics ---- */

void FenwickTree4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
