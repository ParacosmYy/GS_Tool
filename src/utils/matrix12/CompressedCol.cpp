/**
 * @file CompressedCol.cpp
 * @brief CSC稀疏矩阵实现 — SpMV与Gauss-Seidel求解器
 */

#include "utils/matrix12/CompressedCol.h"

#include <QtMath>
#include <QtGlobal>
#include <algorithm>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
CompressedCol::CompressedCol(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 从COO格式构建CSC矩阵
 * COO -> CSC: 按列排序，构建colPtr压缩索引
 */
void CompressedCol::buildFromCOO(const QVector<int>& rows,
                                  const QVector<int>& cols,
                                  const QVector<double>& vals,
                                  int numRows, int numCols)
{
    m_rows = qMax(1, numRows);
    m_cols = qMax(1, numCols);
    int nnz = qMin(qMin(rows.size(), cols.size()), vals.size());

    if (nnz == 0) {
        m_values.clear();
        m_rowIdx.clear();
        m_colPtr.assign(m_cols + 1, 0);
        return;
    }

    /* 按列分组存储 */
    QVector<QVector<QPair<int, double>>> colData(m_cols);
    for (int k = 0; k < nnz; ++k) {
        int r = qBound(0, rows[k], m_rows - 1);
        int c = qBound(0, cols[k], m_cols - 1);
        colData[c].append({r, vals[k]});
    }

    /* 对每列内按行索引排序并去重(合并重复) */
    m_colPtr.resize(m_cols + 1);
    m_colPtr[0] = 0;
    m_values.clear();
    m_rowIdx.clear();

    for (int c = 0; c < m_cols; ++c) {
        std::sort(colData[c].begin(), colData[c].end(),
            [](const QPair<int, double>& a, const QPair<int, double>& b) {
                return a.first < b.first;
            });

        /* 合并相同行索引 */
        int prev = -1;
        for (auto& entry : colData[c]) {
            if (entry.first == prev && !m_rowIdx.isEmpty()) {
                m_values.back() += entry.second;
            } else {
                m_rowIdx.append(entry.first);
                m_values.append(entry.second);
                prev = entry.first;
            }
        }
        m_colPtr[c + 1] = m_values.size();
    }

    m_stats.totalNonZeros += m_values.size();
}

/**
 * @brief 从稠密矩阵构建CSC
 */
void CompressedCol::buildFromDense(const QVector<QVector<double>>& dense,
                                    double dropTolerance)
{
    m_rows = dense.size();
    m_cols = (m_rows > 0) ? dense[0].size() : 0;
    if (m_rows == 0 || m_cols == 0) {
        m_colPtr.assign(1, 0);
        return;
    }

    m_colPtr.resize(m_cols + 1);
    m_colPtr[0] = 0;
    m_values.clear();
    m_rowIdx.clear();

    double absTol = qAbs(dropTolerance);

    for (int c = 0; c < m_cols; ++c) {
        for (int r = 0; r < m_rows && r < static_cast<int>(dense.size()); ++r) {
            if (c < static_cast<int>(dense[r].size())) {
                double val = dense[r][c];
                if (qAbs(val) > absTol) {
                    m_rowIdx.append(r);
                    m_values.append(val);
                }
            }
        }
        m_colPtr[c + 1] = m_values.size();
    }

    m_stats.totalNonZeros += m_values.size();
}

/**
 * @brief 稀疏矩阵-向量乘法 y = A * x
 * CSC格式天然适合列访问: 对每列c，将vals * x[c]累加到y的各行
 */
QVector<double> CompressedCol::spMV(const QVector<double>& x) const
{
    QVector<double> y(m_rows, 0.0);
    if (x.size() != m_cols) return y;

    for (int c = 0; c < m_cols; ++c) {
        double xVal = x[c];
        if (xVal == 0.0) continue;

        int start = m_colPtr[c];
        int end = m_colPtr[c + 1];
        for (int k = start; k < end; ++k) {
            y[m_rowIdx[k]] += m_values[k] * xVal;
        }
    }

    return y;
}

/**
 * @brief Gauss-Seidel迭代求解 Ax = b
 * 逐行迭代: x_i = (b_i - sum_{j!=i} A_{ij}*x_j) / A_{ii}
 * 使用CSC格式需要按行遍历，先提取CSR索引
 */
QPair<QVector<double>, QPair<int, double>>
CompressedCol::gaussSeidel(const QVector<double>& b,
                            int maxIter,
                            double tolerance) const
{
    m_timer.start();

    int n = m_rows;
    QVector<double> x(n, 0.0);
    if (n == 0 || b.size() != n) return {x, {0, 0.0}};

    /* 提取对角线 */
    QVector<double> diag = diagonal();

    /* 构建按行索引: rowEntries[i] = [(col, val), ...] */
    QVector<QVector<QPair<int, double>>> rowEntries(n);
    for (int c = 0; c < m_cols; ++c) {
        int start = m_colPtr[c];
        int end = m_colPtr[c + 1];
        for (int k = start; k < end; ++k) {
            int r = m_rowIdx[k];
            rowEntries[r].append({c, m_values[k]});
        }
    }

    /* 迭代求解 */
    int iter = 0;
    double residual = 0.0;

    for (iter = 0; iter < maxIter; ++iter) {
        double maxDiff = 0.0;

        for (int i = 0; i < n; ++i) {
            double sum = 0.0;
            for (const auto& entry : rowEntries[i]) {
                if (entry.first != i) {
                    sum += entry.second * x[entry.first];
                }
            }

            double d = (qAbs(diag[i]) > 1e-15) ? diag[i] : 1e-15;
            double xNew = (b[i] - sum) / d;
            double diff = qAbs(xNew - x[i]);
            if (diff > maxDiff) maxDiff = diff;
            x[i] = xNew;
        }

        residual = maxDiff;
        if (residual < tolerance) break;
    }

    /* 更新统计 */
    m_timeSum += m_timer.elapsed();
    ++m_stats.totalSolves;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalSolves);

    emit solveCompleted(iter, residual);
    return {x, {iter, residual}};
}

/**
 * @brief 矩阵转置
 * CSC转置 = CSR转CSC: 交换行列索引
 */
CompressedCol* CompressedCol::transpose() const
{
    CompressedCol* result = new CompressedCol(parent());
    result->m_rows = m_cols;
    result->m_cols = m_rows;

    /* 构建转置的CSC: 原来按列存储，转置后按行存储 */
    int newNnz = m_values.size();
    result->m_colPtr.resize(result->m_cols + 1, 0);
    result->m_rowIdx.resize(newNnz);
    result->m_values.resize(newNnz);

    /* 统计每行(新列)非零数 */
    for (int k = 0; k < newNnz; ++k) {
        result->m_colPtr[m_rowIdx[k] + 1]++;
    }

    /* 前缀和 */
    for (int c = 0; c < result->m_cols; ++c) {
        result->m_colPtr[c + 1] += result->m_colPtr[c];
    }

    /* 填入数据 */
    QVector<int> pos = result->m_colPtr;
    for (int c = 0; c < m_cols; ++c) {
        int start = m_colPtr[c];
        int end = m_colPtr[c + 1];
        for (int k = start; k < end; ++k) {
            int newCol = m_rowIdx[k];
            int dest = pos[newCol]++;
            result->m_rowIdx[dest] = c;
            result->m_values[dest] = m_values[k];
        }
    }

    return result;
}

/**
 * @brief 提取对角线元素
 */
QVector<double> CompressedCol::diagonal() const
{
    int n = qMin(m_rows, m_cols);
    QVector<double> diag(n, 0.0);

    for (int c = 0; c < n; ++c) {
        int start = m_colPtr[c];
        int end = m_colPtr[c + 1];
        for (int k = start; k < end; ++k) {
            if (m_rowIdx[k] == c) {
                diag[c] = m_values[k];
                break;
            }
        }
    }

    return diag;
}

/**
 * @brief 计算稀疏度
 */
double CompressedCol::sparsity() const
{
    if (m_rows == 0 || m_cols == 0) return 1.0;
    return 1.0 - static_cast<double>(m_values.size())
        / static_cast<double>(m_rows * m_cols);
}

/** @brief 重置统计 */
void CompressedCol::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 对列内元素按行索引排序(插入排序)
 */
void CompressedCol::sortColumnEntries(QVector<int>& rowIdx,
                                       QVector<double>& vals,
                                       int start, int end) const
{
    for (int i = start + 1; i < end; ++i) {
        int key = rowIdx[i];
        double val = vals[i];
        int j = i - 1;
        while (j >= start && rowIdx[j] > key) {
            rowIdx[j + 1] = rowIdx[j];
            vals[j + 1] = vals[j];
            --j;
        }
        rowIdx[j + 1] = key;
        vals[j + 1] = val;
    }
}
