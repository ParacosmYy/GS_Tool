/**
 * @file FenwickTree8.cpp
 * @brief FenwickTree8 实现
 *
 * 实现二维树状数组：2D范围求和、点更新与二维累积频率计算。
 */

#include "utils/tree240/FenwickTree8.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

FenwickTree8::FenwickTree8(QObject *parent) : QObject(parent) {}
FenwickTree8::~FenwickTree8() = default;

/* ---- Internal helpers ---- */

double FenwickTree8::prefixSum1D(const QVector<double>& arr, int idx) const
{
    double sum = 0.0;
    while (idx > 0) {
        sum += arr[idx];
        idx -= lsb(idx);
    }
    return sum;
}

void FenwickTree8::update1D(QVector<double>& arr, int idx, double delta)
{
    while (idx < arr.size()) {
        arr[idx] += delta;
        idx += lsb(idx);
    }
}

/* ---- Resize ---- */

void FenwickTree8::resize(int rows, int cols)
{
    m_rows = qMax(0, rows);
    m_cols = qMax(0, cols);

    // 1-indexed: need (rows+1) x (cols+1)
    m_tree.assign(m_rows + 1, QVector<double>(m_cols + 1, 0.0));
    m_values.assign(m_rows, QVector<double>(m_cols, 0.0));

    m_stats.rows = m_rows;
    m_stats.cols = m_cols;
}

/* ---- Build from 2D data ---- */

void FenwickTree8::build(const QVector<QVector<double>>& data)
{
    int r = data.size();
    int c = (r > 0) ? data[0].size() : 0;
    resize(r, c);

    for (int i = 0; i < r; ++i)
        for (int j = 0; j < c; ++j) {
            m_values[i][j] = data[i][j];
            // Direct BIT update (1-indexed)
            for (int ii = i + 1; ii <= m_rows; ii += lsb(ii))
                for (int jj = j + 1; jj <= m_cols; jj += lsb(jj))
                    m_tree[ii][jj] += data[i][j];
        }
}

/* ---- Point update ---- */

void FenwickTree8::update(int row, int col, double delta)
{
    if (row < 0 || row >= m_rows || col < 0 || col >= m_cols) return;

    m_values[row][col] += delta;

    // Update BIT: cascade through all affected nodes
    for (int i = row + 1; i <= m_rows; i += lsb(i))
        for (int j = col + 1; j <= m_cols; j += lsb(j))
            m_tree[i][j] += delta;

    m_stats.numUpdates++;
    m_stats.totalOps++;
    emit updated(row, col, delta);
}

/* ---- Set value ---- */

void FenwickTree8::set(int row, int col, double value)
{
    if (row < 0 || row >= m_rows || col < 0 || col >= m_cols) return;
    double delta = value - m_values[row][col];
    update(row, col, delta);
}

/* ---- Prefix sum ---- */

double FenwickTree8::prefixSum(int row, int col) const
{
    if (row < 0 || col < 0) return 0.0;
    row = qMin(row, m_rows - 1);
    col = qMin(col, m_cols - 1);

    double sum = 0.0;
    for (int i = row + 1; i > 0; i -= lsb(i))
        for (int j = col + 1; j > 0; j -= lsb(j))
            sum += m_tree[i][j];

    return sum;
}

/* ---- Range sum ---- */

double FenwickTree8::rangeSum(int r1, int c1, int r2, int c2) const
{
    QElapsedTimer timer;
    timer.start();

    if (r1 > r2 || c1 > c2) return 0.0;
    r1 = qMax(0, r1); c1 = qMax(0, c1);
    r2 = qMin(m_rows - 1, r2); c2 = qMin(m_cols - 1, c2);

    // Inclusion-exclusion for 2D range sum
    double result = prefixSum(r2, c2);
    if (r1 > 0) result -= prefixSum(r1 - 1, c2);
    if (c1 > 0) result -= prefixSum(r2, c1 - 1);
    if (r1 > 0 && c1 > 0) result += prefixSum(r1 - 1, c1 - 1);

    m_stats.numQueries++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit queryCompleted(result, timer.elapsed());
    return result;
}

/* ---- Get value ---- */

double FenwickTree8::get(int row, int col) const
{
    if (row < 0 || row >= m_rows || col < 0 || col >= m_cols) return 0.0;
    return m_values[row][col];
}

/* ---- Dimensions ---- */

int FenwickTree8::rows() const { return m_rows; }
int FenwickTree8::cols() const { return m_cols; }

/* ---- Reset ---- */

void FenwickTree8::resetStatistics()
{
    m_tree.clear(); m_values.clear();
    m_rows = 0; m_cols = 0;
    m_stats = Stats{}; m_timeSum = 0.0;
}
