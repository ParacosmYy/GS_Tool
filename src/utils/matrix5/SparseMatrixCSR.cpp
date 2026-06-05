/**
 * @file SparseMatrixCSR.cpp
 * @brief 压缩稀疏行(CSR)矩阵实现
 */

#include "SparseMatrixCSR.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---------- 构造函数 ---------- */

SparseMatrixCSR::SparseMatrixCSR(int rows, int cols, QObject* parent)
    : QObject(parent), m_rows(rows), m_cols(cols)
{
    /* 初始化行指针数组: m_rows + 1 个元素，全为0 */
    m_rowPtr.resize(m_rows + 1, 0);
}

/* ---------- 从三元组构建 ---------- */

void SparseMatrixCSR::buildFromTriplets(const QVector<Triplet>& triplets)
{
    /* 按行排序，行内按列排序 */
    QVector<Triplet> sorted = triplets;
    std::sort(sorted.begin(), sorted.end(),
              [](const Triplet& a, const Triplet& b) {
                  if (a.row != b.row) return a.row < b.row;
                  return a.col < b.col;
              });

    /* 合并相同位置的元素 */
    QVector<Triplet> merged;
    for (const auto& t : sorted) {
        if (t.row < 0 || t.row >= m_rows || t.col < 0 || t.col >= m_cols) {
            continue; /* 跳过越界元素 */
        }
        if (t.value == 0.0) {
            continue; /* 跳过零值 */
        }
        if (!merged.isEmpty() && merged.last().row == t.row
            && merged.last().col == t.col) {
            merged.last().value += t.value; /* 合并重复位置 */
        } else {
            merged.append(t);
        }
    }

    /* 填充CSR数组 */
    m_values.clear();
    m_colIndices.clear();
    m_rowPtr.assign(m_rows + 1, 0);

    for (const auto& t : merged) {
        m_rowPtr[t.row + 1]++;
    }

    /* 行指针前缀和 */
    for (int i = 1; i <= m_rows; ++i) {
        m_rowPtr[i] += m_rowPtr[i - 1];
    }

    m_values.resize(merged.size());
    m_colIndices.resize(merged.size());

    QVector<int> currentPos = m_rowPtr;
    for (const auto& t : merged) {
        int pos = currentPos[t.row]++;
        m_values[pos] = t.value;
        m_colIndices[pos] = t.col;
    }

    m_stats.totalElementsInserted += merged.size();
}

/* ---------- 设置维度 ---------- */

void SparseMatrixCSR::resize(int rows, int cols)
{
    m_rows = rows;
    m_cols = cols;
    clear();
}

/* ---------- 获取元素 ---------- */

double SparseMatrixCSR::get(int row, int col) const
{
    if (row < 0 || row >= m_rows || col < 0 || col >= m_cols) {
        return 0.0;
    }

    /* 在该行中二分查找列索引 */
    int start = m_rowPtr[row];
    int end = m_rowPtr[row + 1];

    for (int i = start; i < end; ++i) {
        if (m_colIndices[i] == col) {
            return m_values[i];
        }
        if (m_colIndices[i] > col) {
            break;
        }
    }
    return 0.0;
}

/* ---------- SpMV ---------- */

QVector<double> SparseMatrixCSR::multiplyVector(
    const QVector<double>& x)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> y(m_rows, 0.0);

    if (x.size() != m_cols) {
        return y; /* 维度不匹配返回零向量 */
    }

    for (int i = 0; i < m_rows; ++i) {
        double sum = 0.0;
        for (int j = m_rowPtr[i]; j < m_rowPtr[i + 1]; ++j) {
            sum += m_values[j] * x[m_colIndices[j]];
        }
        y[i] = sum;
    }

    m_stats.totalSpMV++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalSpMV > 0)
        ? m_timeSum / m_stats.totalSpMV : 0.0;

    emit spMVCompleted(nonZeroCount(), timer.elapsed());
    return y;
}

/* ---------- 转置 ---------- */

SparseMatrixCSR* SparseMatrixCSR::transpose()
{
    QElapsedTimer timer;
    timer.start();

    SparseMatrixCSR* result = new SparseMatrixCSR(m_cols, m_rows, parent());
    int nnz = nonZeroCount();

    /* 统计每列非零元素数 */
    QVector<int> colCounts(m_cols, 0);
    for (int i = 0; i < nnz; ++i) {
        colCounts[m_colIndices[i]]++;
    }

    /* 构建行指针(转置后的行对应原列) */
    result->m_rowPtr.resize(m_cols + 1, 0);
    for (int i = 0; i < m_cols; ++i) {
        result->m_rowPtr[i + 1] = result->m_rowPtr[i] + colCounts[i];
    }

    /* 填充值和列索引 */
    result->m_values.resize(nnz);
    result->m_colIndices.resize(nnz);

    QVector<int> currentPos = result->m_rowPtr;
    for (int i = 0; i < m_rows; ++i) {
        for (int j = m_rowPtr[i]; j < m_rowPtr[i + 1]; ++j) {
            int col = m_colIndices[j];
            int pos = currentPos[col]++;
            result->m_values[pos] = m_values[j];
            result->m_colIndices[pos] = i; /* 原行号变为列号 */
        }
    }

    m_stats.totalTransposes++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalTransposes > 0)
        ? m_timeSum / m_stats.totalTransposes : 0.0;

    emit transposeCompleted(result->m_rows, result->m_cols);
    return result;
}

/* ---------- 获取行 ---------- */

QVector<QPair<int, double>> SparseMatrixCSR::getRow(int row) const
{
    QVector<QPair<int, double>> result;
    if (row < 0 || row >= m_rows) return result;

    for (int i = m_rowPtr[row]; i < m_rowPtr[row + 1]; ++i) {
        result.append({m_colIndices[i], m_values[i]});
    }
    return result;
}

/* ---------- 清空 ---------- */

void SparseMatrixCSR::clear()
{
    m_values.clear();
    m_colIndices.clear();
    m_rowPtr.assign(m_rows + 1, 0);
}

/* ---------- 稀疏度 ---------- */

double SparseMatrixCSR::sparsity() const
{
    quint64 total = static_cast<quint64>(m_rows) * m_cols;
    if (total == 0) return 0.0;
    return static_cast<double>(nonZeroCount()) / total;
}

/* ---------- 统计 ---------- */

SparseMatrixCSR::Stats SparseMatrixCSR::stats() const { return m_stats; }

void SparseMatrixCSR::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
