/**
 * @file SparseCholesky2.cpp
 * @brief 稀疏Cholesky分解实现 — 稀疏正定矩阵的LDL^T分解
 *
 * 使用不完全Cholesky分解处理稀疏对称正定矩阵，
 * 支持逐步构建稀疏矩阵并进行分解求解，计算对数行列式。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/matrix66/SparseCholesky2.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject指针
 */
SparseCholesky2::SparseCholesky2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置矩阵维度
 * @param n 矩阵行/列数(方阵)，必须 >= 1
 */
void SparseCholesky2::setDimension(int n)
{
    m_n = qMax(1, n);
    m_rowPtr.clear();
    m_colIdx.clear();
    m_values.clear();
    m_L.clear();
    m_logDet = 0.0;
}

/**
 * @brief 添加稀疏矩阵元素
 *
 * 重复添加同一位置的值会累加。
 *
 * @param row 行索引 (0-based)
 * @param col 列索引 (0-based)
 * @param val 元素值
 */
void SparseCholesky2::addEntry(int row, int col, double val)
{
    if (row < 0 || row >= m_n || col < 0 || col >= m_n) return;
    /* 存储为COO格式，factorize时转换为CSR */
    m_rowPtr.append(row);
    m_colIdx.append(col);
    m_values.append(val);
    /* 对称填充 */
    if (row != col) {
        m_rowPtr.append(col);
        m_colIdx.append(row);
        m_values.append(val);
    }
}

/**
 * @brief 执行稀疏Cholesky分解
 *
 * 将COO格式转换为稠密表示后执行LDL^T分解。
 * 分解完成后可调用solve()求解线性系统。
 *
 * @return true如果分解成功(正定)，false否则
 */
bool SparseCholesky2::factorize()
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0) return false;

    /* 构建稠密矩阵用于分解 */
    QVector<QVector<double>> A(m_n, QVector<double>(m_n, 0.0));
    for (int idx = 0; idx < m_rowPtr.size(); ++idx) {
        int r = m_rowPtr[idx];
        int c = m_colIdx[idx];
        A[r][c] += m_values[idx];
    }

    /* 检查正定性 */
    if (!isPositiveDefinite()) {
        return false;
    }

    /* Cholesky分解: A = L * L^T */
    m_L.resize(m_n * m_n);
    std::fill(m_L.begin(), m_L.end(), 0.0);
    m_logDet = 0.0;

    for (int j = 0; j < m_n; ++j) {
        /* 计算对角元素 */
        double sum = 0.0;
        for (int k = 0; k < j; ++k) {
            double lkj = m_L[j * m_n + k];
            sum += lkj * lkj;
        }
        double diag = A[j][j] - sum;
        if (diag <= 0.0) {
            m_logDet = -1e18;
            return false;
        }
        m_L[j * m_n + j] = qSqrt(diag);
        m_logDet += qLn(diag); /* log(det) = 2 * sum(log(L_ii)) */

        /* 计算下三角元素 */
        for (int i = j + 1; i < m_n; ++i) {
            sum = 0.0;
            for (int k = 0; k < j; ++k) {
                sum += m_L[i * m_n + k] * m_L[j * m_n + k];
            }
            m_L[i * m_n + j] = (A[i][j] - sum) / m_L[j * m_n + j];
        }
    }
    m_logDet *= 0.5; /* log(det(A)) = 2 * sum(log(L_ii)) */

    /* 更新统计信息 */
    m_stats.totalFactorizations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalFactorizations + m_stats.totalSolves);

    emit factorizationCompleted(m_n, m_logDet);
    return true;
}

/**
 * @brief 求解线性系统 Ax = b
 *
 * 使用已分解的L矩阵进行前代和回代。
 * 必须在factorize()成功后调用。
 *
 * @param b 右端向量
 * @return 解向量x
 */
QVector<double> SparseCholesky2::solve(const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0 || b.size() != m_n || m_L.isEmpty()) {
        return QVector<double>(m_n, 0.0);
    }

    /* 前代: L * y = b */
    QVector<double> y(m_n, 0.0);
    for (int i = 0; i < m_n; ++i) {
        double sum = 0.0;
        for (int k = 0; k < i; ++k) {
            sum += m_L[i * m_n + k] * y[k];
        }
        y[i] = (b[i] - sum) / m_L[i * m_n + i];
    }

    /* 回代: L^T * x = y */
    QVector<double> x(m_n, 0.0);
    for (int i = m_n - 1; i >= 0; --i) {
        double sum = 0.0;
        for (int k = i + 1; k < m_n; ++k) {
            sum += m_L[k * m_n + i] * x[k];
        }
        x[i] = (y[i] - sum) / m_L[i * m_n + i];
    }

    /* 更新统计信息 */
    m_stats.totalSolves++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalFactorizations + m_stats.totalSolves);
    return x;
}

/**
 * @brief 重置所有统计数据
 */
void SparseCholesky2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 检查矩阵是否正定
 *
 * 通过检查对角元素是否全为正值来近似判断正定性。
 *
 * @return true如果可能正定，false如果有非正对角元素
 */
bool SparseCholesky2::isPositiveDefinite() const
{
    /* 构建对角元素检查 */
    QVector<double> diag(m_n, 0.0);
    for (int idx = 0; idx < m_rowPtr.size(); ++idx) {
        int r = m_rowPtr[idx];
        int c = m_colIdx[idx];
        if (r == c) {
            diag[r] += m_values[idx];
        }
    }
    for (int i = 0; i < m_n; ++i) {
        if (diag[i] <= 0.0) return false;
    }
    return true;
}
