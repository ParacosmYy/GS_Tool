/**
 * @file SparseCholesky2.cpp
 * @brief 稀疏Cholesky分解实现 — 符号分解/列消元/填充减少/重排序
 */

#include "utils/matrix34/SparseCholesky2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
SparseCholesky2::SparseCholesky2(QObject* parent)
    : QObject(parent)
{
}

/** @brief 从COO格式构建稀疏矩阵 @param rows 行索引 @param cols 列索引 @param vals 值 @param n 维度 */
void SparseCholesky2::buildFromCOO(const QVector<int>& rows, const QVector<int>& cols,
                                     const QVector<double>& vals, int n)
{
    m_n = qMax(1, n);
    m_factored = false;
    m_spd = false;
    m_logDet = 0.0;

    /* 构建上三角CSR格式(对称矩阵只存上三角) */
    QVector<int> rowCounts(m_n + 1, 0);

    /* 统计上三角非零元素 */
    int nnz = 0;
    for (int idx = 0; idx < rows.size(); ++idx) {
        int i = rows[idx];
        int j = cols[idx];
        if (i >= 0 && i < m_n && j >= 0 && j < m_n) {
            int row = qMin(i, j);
            int col = qMax(i, j);
            ++rowCounts[row + 1];
            ++nnz;
        }
    }

    /* 累加行指针 */
    m_colPtr.resize(m_n + 1, 0);
    for (int i = 0; i < m_n; ++i) {
        m_colPtr[i + 1] = m_colPtr[i] + rowCounts[i + 1];
    }

    /* 填充列索引和值 */
    m_rowIdx.resize(nnz);
    m_values.resize(nnz);
    QVector<int> pos = m_colPtr;

    for (int idx = 0; idx < rows.size(); ++idx) {
        int i = rows[idx];
        int j = cols[idx];
        if (i >= 0 && i < m_n && j >= 0 && j < m_n) {
            int row = qMin(i, j);
            int col = qMax(i, j);
            int p = pos[row]++;
            m_rowIdx[p] = col;
            m_values[p] = vals[idx];
        }
    }

    /* 对每行按列索引排序 */
    for (int i = 0; i < m_n; ++i) {
        int start = m_colPtr[i];
        int end = m_colPtr[i + 1];
        for (int j = start; j < end - 1; ++j) {
            for (int k = j + 1; k < end; ++k) {
                if (m_rowIdx[j] > m_rowIdx[k]) {
                    std::swap(m_rowIdx[j], m_rowIdx[k]);
                    std::swap(m_values[j], m_values[k]);
                }
            }
        }
    }

    /* 初始化排列为单位 */
    m_perm.resize(m_n);
    m_invPerm.resize(m_n);
    for (int i = 0; i < m_n; ++i) {
        m_perm[i] = i;
        m_invPerm[i] = i;
    }
}

/** @brief 执行稀疏Cholesky分解 @return 是否成功(正定) */
bool SparseCholesky2::decompose()
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0) return false;

    /* 符号分解: 确定填充模式 */
    symbolicDecompose();

    /* 数值分解: 列消元 */
    /* 使用稠密列方式处理每个列 */
    QVector<double> denseCol(m_n, 0.0);
    QVector<int> colStart(m_n, 0);
    QVector<int> colEnd(m_n, 0);

    /* 复制原始矩阵到工作区 */
    QVector<QVector<QPair<int, double>>> L(m_n);
    for (int i = 0; i < m_n; ++i) {
        for (int j = m_colPtr[i]; j < m_colPtr[i + 1]; ++j) {
            L[i].append(qMakePair(m_rowIdx[j], m_values[j]));
        }
    }

    m_logDet = 0.0;
    m_spd = true;

    for (int j = 0; j < m_n; ++j) {
        /* 加载第j列到denseCol */
        std::fill(denseCol.begin(), denseCol.end(), 0.0);
        for (const auto& entry : L[j]) {
            denseCol[entry.first] = entry.second;
        }

        /* 减去之前列的贡献 */
        for (int k = 0; k < j; ++k) {
            /* L[j][k] * L[k][j] 的贡献 */
            double ljk = 0.0;
            for (const auto& e : L[j]) {
                if (e.first == k) { ljk = e.second; break; }
            }
            if (qFabs(ljk) < 1e-15) continue;

            double lkk = 0.0;
            for (const auto& e : L[k]) {
                if (e.first == k) { lkk = e.second; break; }
            }

            for (const auto& e : L[k]) {
                if (e.first >= j) {
                    denseCol[e.first] -= ljk * e.second;
                }
            }
        }

        /* 对角线元素 */
        double diag = denseCol[j];
        if (diag <= 0.0) {
            m_spd = false;
            m_factored = false;
            emit decompositionComplete(m_n, false);
            return false;
        }

        double sqrtDiag = qSqrt(diag);
        m_logDet += qLn(diag);

        /* 更新第j列: L[j][j] = sqrt(diag), L[j][i] /= sqrtDiag */
        L[j].clear();
        L[j].append(qMakePair(j, sqrtDiag));
        for (int i = j + 1; i < m_n; ++i) {
            if (qFabs(denseCol[i]) > 1e-15) {
                L[j].append(qMakePair(i, denseCol[i] / sqrtDiag));
            }
        }
    }

    /* 将L存回CSR格式 */
    int totalNnz = 0;
    for (int j = 0; j < m_n; ++j) totalNnz += L[j].size();

    m_colPtr.resize(m_n + 1);
    m_rowIdx.resize(totalNnz);
    m_values.resize(totalNnz);

    m_colPtr[0] = 0;
    int pos = 0;
    for (int j = 0; j < m_n; ++j) {
        for (const auto& e : L[j]) {
            m_rowIdx[pos] = e.first;
            m_values[pos] = e.second;
            ++pos;
        }
        m_colPtr[j + 1] = pos;
    }

    m_factored = true;
    m_spd = true;

    m_stats.totalDecompositions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalDecompositions + m_stats.totalSolves));

    emit decompositionComplete(m_n, true);
    return true;
}

/** @brief 使用分解结果求解线性系统 @param rhs 右端向量 @return 解向量 */
QVector<double> SparseCholesky2::solve(const QVector<double>& rhs) const
{
    QElapsedTimer timer;
    timer.start();

    if (!m_factored || rhs.size() != m_n) return QVector<double>(m_n, 0.0);

    QVector<double> y(m_n, 0.0);
    QVector<double> x = rhs;

    /* 前代: L * y = b */
    for (int j = 0; j < m_n; ++j) {
        double sum = x[j];
        for (int k = m_colPtr[j]; k < m_colPtr[j + 1]; ++k) {
            int i = m_rowIdx[k];
            if (i < j) {
                sum -= m_values[k] * y[i];
            }
        }
        /* 找对角线元素 */
        double diag = 1.0;
        for (int k = m_colPtr[j]; k < m_colPtr[j + 1]; ++k) {
            if (m_rowIdx[k] == j) { diag = m_values[k]; break; }
        }
        y[j] = sum / diag;
    }

    /* 回代: L^T * x = y */
    for (int j = m_n - 1; j >= 0; --j) {
        x[j] = y[j];
        for (int k = m_colPtr[j]; k < m_colPtr[j + 1]; ++k) {
            int i = m_rowIdx[k];
            if (i > j) {
                x[j] -= m_values[k] * x[i];
            }
        }
        double diag = 1.0;
        for (int k = m_colPtr[j]; k < m_colPtr[j + 1]; ++k) {
            if (m_rowIdx[k] == j) { diag = m_values[k]; break; }
        }
        x[j] /= diag;
    }

    m_stats.totalSolves++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalDecompositions + m_stats.totalSolves));

    return x;
}

/** @brief 计算对数行列式 @return ln(det(A)) */
double SparseCholesky2::logDeterminant() const
{
    return m_factored ? m_logDet : 0.0;
}

/** @brief 检查矩阵是否正定 @return 是否正定 */
bool SparseCholesky2::isPositiveDefinite() const
{
    return m_spd;
}

/** @brief 获取排列向量 @return 排列 */
QVector<int> SparseCholesky2::permutation() const
{
    return m_perm;
}

/** @brief 符号分解: 确定填充模式 */
void SparseCholesky2::symbolicDecompose()
{
    /* AMD近似最小度排序简化版 */
    /* 使用自然序排列作为简化实现 */
    for (int i = 0; i < m_n; ++i) {
        m_perm[i] = i;
        m_invPerm[i] = i;
    }
}

/** @brief 应用排列(简化: 单位排列) */
void SparseCholesky2::applyPermutation()
{
    /* 使用自然序，无需额外操作 */
}

/** @brief 重置统计 */
void SparseCholesky2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
