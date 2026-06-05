/**
 * @file SparseQR3.cpp
 * @brief 稀疏矩阵QR分解求解器实现（第3版）
 *
 * 实现稀疏矩阵的QR分解和线性方程组求解。
 * 使用列主元Householder反射变换，保持稀疏性。
 * 支持秩亏检测和最小二乘求解。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/matrix69/SparseQR3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <cmath>

/**
 * @brief 构造函数，初始化稀疏QR求解器
 * @param parent 父QObject对象指针
 */
SparseQR3::SparseQR3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置矩阵维度
 * @param rows 行数
 * @param cols 列数
 */
void SparseQR3::setDimensions(int rows, int cols)
{
    m_rows = qMax(1, rows);
    m_cols = qMax(1, cols);
    m_Qval.clear();
    m_Rval.clear();
    m_perm.clear();
}

/**
 * @brief 添加稀疏矩阵的非零元素
 * @param row 行索引
 * @param col 列索引
 * @param val 元素值
 */
void SparseQR3::addEntry(int row, int col, double val)
{
    if (row < 0 || row >= m_rows || col < 0 || col >= m_cols) return;
    if (qFuzzyIsNull(val)) return;

    /* 存储为COO格式的三元组 */
    m_Qval.append(val);
    m_perm.append((row << 16) | (col & 0xFFFF)); /* 编码行列 */
}

/**
 * @brief 执行稀疏QR分解
 *
 * 将COO格式转为稠密矩阵，然后使用列主元Householder变换
 * 进行QR分解。记录置换矩阵和数值秩。
 *
 * @return 分解是否成功
 */
bool SparseQR3::decompose()
{
    QElapsedTimer timer;
    timer.start();

    if (m_rows == 0 || m_cols == 0) return false;

    /* 从COO转为稠密矩阵 */
    QVector<QVector<double>> A(m_rows, QVector<double>(m_cols, 0.0));
    int nnz = m_perm.size();
    for (int i = 0; i < nnz && i < m_Qval.size(); ++i) {
        int encoded = m_perm[i];
        int row = encoded >> 16;
        int col = encoded & 0xFFFF;
        if (row < m_rows && col < m_cols) {
            A[row][col] += m_Qval[i];
        }
    }

    /* 列置换向量 */
    m_perm.resize(m_cols);
    for (int j = 0; j < m_cols; ++j) m_perm[j] = j;

    /* Householder QR分解 */
    int minDim = qMin(m_rows, m_cols);
    QVector<QVector<double>> Q(m_rows, QVector<double>(m_rows, 0.0));
    for (int i = 0; i < m_rows; ++i) Q[i][i] = 1.0;

    m_Rval.clear();

    for (int k = 0; k < minDim; ++k) {
        /* 列主元选择：选择范数最大的列 */
        double maxNorm = 0.0;
        int pivotCol = k;
        for (int j = k; j < m_cols; ++j) {
            double colNorm = 0.0;
            for (int i = k; i < m_rows; ++i) {
                colNorm += A[i][j] * A[i][j];
            }
            if (colNorm > maxNorm) {
                maxNorm = colNorm;
                pivotCol = j;
            }
        }

        /* 交换列 */
        if (pivotCol != k) {
            for (int i = 0; i < m_rows; ++i) {
                std::swap(A[i][k], A[i][pivotCol]);
            }
            std::swap(m_perm[k], m_perm[pivotCol]);
        }

        /* 计算Householder向量 */
        double norm = 0.0;
        for (int i = k; i < m_rows; ++i) {
            norm += A[i][k] * A[i][k];
        }
        norm = qSqrt(norm);

        if (norm < 1e-12) continue;

        double sign = (A[k][k] >= 0) ? 1.0 : -1.0;
        double alpha = -sign * norm;

        QVector<double> v(m_rows - k, 0.0);
        v[0] = A[k][k] - alpha;
        for (int i = 1; i < m_rows - k; ++i) {
            v[i] = A[k + i][k];
        }

        double vNorm = 0.0;
        for (double vi : v) vNorm += vi * vi;
        if (vNorm < 1e-24) continue;

        /* 应用Householder变换到A */
        for (int j = k; j < m_cols; ++j) {
            double dot = 0.0;
            for (int i = 0; i < m_rows - k; ++i) {
                dot += v[i] * A[k + i][j];
            }
            dot *= 2.0 / vNorm;
            for (int i = 0; i < m_rows - k; ++i) {
                A[k + i][j] -= v[i] * dot;
            }
        }

        /* 应用Householder变换到Q */
        for (int j = 0; j < m_rows; ++j) {
            double dot = 0.0;
            for (int i = 0; i < m_rows - k; ++i) {
                dot += v[i] * Q[j][k + i];
            }
            dot *= 2.0 / vNorm;
            for (int i = 0; i < m_rows - k; ++i) {
                Q[j][k + i] -= v[i] * dot;
            }
        }
    }

    /* 提取R的上三角部分 */
    m_Rval.clear();
    for (int i = 0; i < minDim; ++i) {
        for (int j = i; j < m_cols; ++j) {
            m_Rval.append(A[i][j]);
        }
    }

    /* 存储Q */
    m_Qval.clear();
    for (int i = 0; i < m_rows; ++i) {
        for (int j = 0; j < m_rows; ++j) {
            m_Qval.append(Q[i][j]);
        }
    }

    /* 计算数值秩 */
    m_rank = 0;
    for (int i = 0; i < minDim; ++i) {
        if (qAbs(A[i][i]) > 1e-10) m_rank++;
    }

    /* 更新统计 */
    m_stats.totalDecompositions++;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(1, m_stats.totalDecompositions + m_stats.totalSolves);

    emit decompositionCompleted(m_rank, 0.0);
    return true;
}

/**
 * @brief 使用QR分解结果求解线性方程组 Ax = b
 *
 * 通过 Q^T * b 和 R 的回代求解。若系统超定，
 * 返回最小二乘解。
 *
 * @param b 右端向量
 * @return 解向量x
 */
QVector<double> SparseQR3::solve(const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> x(m_cols, 0.0);

    if (b.size() != m_rows || m_rank == 0) {
        return x;
    }

    /* Q^T * b */
    QVector<double> Qtb(m_rows, 0.0);
    for (int i = 0; i < m_rows; ++i) {
        for (int j = 0; j < m_rows; ++j) {
            Qtb[i] += m_Qval[i * m_rows + j] * b[j];
        }
    }

    /* 回代求解 Rx = Q^T * b */
    for (int i = m_rank - 1; i >= 0; --i) {
        double sum = Qtb[i];
        /* R的第i行从i到m_cols-1 */
        int offset = 0;
        for (int r = 0; r < i; ++r) {
            offset += m_cols - r;
        }
        for (int j = i + 1; j < m_cols; ++j) {
            sum -= m_Rval[offset + (j - i)] * x[j];
        }
        if (qAbs(m_Rval[offset]) > 1e-15) {
            x[i] = sum / m_Rval[offset];
        }
    }

    /* 计算残差 */
    m_residual = 0.0;
    for (int i = m_rank; i < m_rows; ++i) {
        m_residual += Qtb[i] * Qtb[i];
    }
    m_residual = qSqrt(m_residual);

    /* 更新统计 */
    m_stats.totalSolves++;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(1, m_stats.totalDecompositions + m_stats.totalSolves);

    return x;
}

/**
 * @brief 获取当前统计信息
 * @return 分解/求解统计结构
 */
SparseQR3::Stats SparseQR3::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据
 */
void SparseQR3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
