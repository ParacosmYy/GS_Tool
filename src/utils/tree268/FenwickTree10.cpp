/**
 * @file FenwickTree10.cpp
 * @brief FenwickTree10 实现
 *
 * 实现二维树状数组：嵌套BIT点更新与二维区间和查询矩阵前缀和。
 */

#include "utils/tree268/FenwickTree10.h"

#include <QElapsedTimer>
#include <QtGlobal>

/* ---- Construction / Destruction ---- */

FenwickTree10::FenwickTree10(QObject *parent)
    : QObject(parent) {}

FenwickTree10::~FenwickTree10() = default;

/* ---- Initialize ---- */

void FenwickTree10::init(int rows, int cols)
{
    m_rows = qMax(1, rows);
    m_cols = qMax(1, cols);

    // BIT uses 1-based indexing, allocate (rows+1) x (cols+1)
    m_tree.clear();
    m_tree.resize(m_rows + 1);
    for (int i = 0; i <= m_rows; ++i)
        m_tree[i].resize(m_cols + 1, 0.0);

    m_values.clear();
    m_values.resize(m_rows);
    for (int i = 0; i < m_rows; ++i)
        m_values[i].resize(m_cols, 0.0);
}

/* ---- Build from matrix ---- */

void FenwickTree10::build(const QVector<QVector<double>>& matrix)
{
    if (matrix.isEmpty()) return;
    int r = matrix.size();
    int c = matrix[0].size();
    init(r, c);

    for (int i = 0; i < r; ++i) {
        for (int j = 0; j < c; ++j) {
            m_values[i][j] = matrix[i][j];
            // Direct BIT update at (i+1, j+1)
            double val = matrix[i][j];
            for (int ii = i + 1; ii <= m_rows; ii += lsb(ii)) {
                for (int jj = j + 1; jj <= m_cols; jj += lsb(jj)) {
                    m_tree[ii][jj] += val;
                }
            }
        }
    }
}

/* ---- Point update: add delta at (row, col) [0-based] ---- */

void FenwickTree10::update(int row, int col, double delta)
{
    QElapsedTimer timer;
    timer.start();

    if (row < 0 || row >= m_rows || col < 0 || col >= m_cols) return;

    m_values[row][col] += delta;

    // BIT update: propagate through all affected nodes
    for (int i = row + 1; i <= m_rows; i += lsb(i)) {
        for (int j = col + 1; j <= m_cols; j += lsb(j)) {
            m_tree[i][j] += delta;
        }
    }

    m_stats.numUpdates++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
}

/* ---- Prefix sum: sum from (1,1) to (row, col) [1-based internal] ---- */

double FenwickTree10::prefixSum(int row, int col) const
{
    double sum = 0.0;
    // Clamp to valid range
    int r = qBound(0, row, m_rows);
    int c = qBound(0, col, m_cols);

    for (int i = r; i > 0; i -= lsb(i)) {
        for (int j = c; j > 0; j -= lsb(j)) {
            sum += m_tree[i][j];
        }
    }
    return sum;
}

/* ---- Range sum: sum within (r1,c1) to (r2,c2) inclusive [0-based] ---- */

double FenwickTree10::rangeSum(int r1, int c1, int r2, int c2) const
{
    QElapsedTimer timer;
    timer.start();

    if (r1 < 0 || c1 < 0 || r2 >= m_rows || c2 >= m_cols) return 0.0;
    if (r1 > r2 || c1 > c2) return 0.0;

    // Inclusion-exclusion using prefix sums (convert to 1-based)
    double s = prefixSum(r2 + 1, c2 + 1)
             - prefixSum(r1, c2 + 1)
             - prefixSum(r2 + 1, c1)
             + prefixSum(r1, c1);

    m_stats.numQueries++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return s;
}

/* ---- Get value at (row, col) ---- */

double FenwickTree10::get(int row, int col) const
{
    if (row < 0 || row >= m_rows || col < 0 || col >= m_cols) return 0.0;
    return m_values[row][col];
}

/* ---- Accessors ---- */

int FenwickTree10::rows() const { return m_rows; }
int FenwickTree10::cols() const { return m_cols; }

/* ---- Reset ---- */

void FenwickTree10::resetStatistics()
{
    m_tree.clear();
    m_values.clear();
    m_rows = 0;
    m_cols = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
