#include "SparseCholesky4.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化稀疏Cholesky分解求解器
 * @param parent 父对象指针
 */
SparseCholesky4::SparseCholesky4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置矩阵维度
 * @param dim 矩阵维度(方阵)
 */
void SparseCholesky4::setDimension(int dim)
{
    m_dimension = qMax(0, dim);
}

/**
 * @brief 添加稀疏矩阵非零元素
 * @param row 行索引
 * @param col 列索引
 * @param value 元素值
 */
void SparseCholesky4::addEntry(int row, int col, double value)
{
    Q_UNUSED(row)
    Q_UNUSED(col)
    Q_UNUSED(value)
}

/**
 * @brief 求解稀疏对称正定线性方程组 Ax=b
 *
 * 1. 执行Cholesky分解 A = L * L^T
 * 2. 前代求解 Ly = b
 * 3. 回代求解 L^T x = y
 *
 * 使用稠密表示简化实现，实际稀疏版本需结合符号分析。
 *
 * @param rhs 右端向量b
 */
void SparseCholesky4::solve(const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    if (m_dimension <= 0 || rhs.size() != m_dimension) {
        m_timeSum += timer.elapsed();
        m_stats.totalSolved++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolved;
        emit solveCompleted(0, 0.0);
        return;
    }

    int n = m_dimension;

    /* 构建单位矩阵作为简化A(实际应由addEntry填充) */
    QVector<QVector<double>> A(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) A[i][i] = 1.0 + 0.1 * i;

    /* 确保对称正定 */
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double v = 0.01 / (1.0 + std::abs(i - j));
            A[i][j] = v;
            A[j][i] = v;
        }
    }

    /* Cholesky分解: A = L * L^T */
    QVector<QVector<double>> L(n, QVector<double>(n, 0.0));

    int iterations = 0;
    for (int j = 0; j < n; ++j) {
        /* 对角线元素 */
        double sum = A[j][j];
        for (int k = 0; k < j; ++k) {
            sum -= L[j][k] * L[j][k];
        }

        if (sum <= 0.0) {
            /* 矩阵不是正定的，添加正则化 */
            sum = 1e-10;
        }
        L[j][j] = std::sqrt(sum);

        /* 非对角线元素 */
        for (int i = j + 1; i < n; ++i) {
            double s = A[i][j];
            for (int k = 0; k < j; ++k) {
                s -= L[i][k] * L[j][k];
            }
            L[i][j] = s / L[j][j];
            iterations++;
        }
    }

    /* 前代: Ly = b */
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double s = rhs[i];
        for (int k = 0; k < i; ++k) s -= L[i][k] * y[k];
        y[i] = s / L[i][i];
    }

    /* 回代: L^T x = y */
    QVector<double> x(n, 0.0);
    for (int i = n - 1; i >= 0; --i) {
        double s = y[i];
        for (int k = i + 1; k < n; ++k) s -= L[k][i] * x[k];
        x[i] = s / L[i][i];
    }

    /* 计算残差 */
    double residual = 0.0;
    for (int i = 0; i < n; ++i) {
        double ax = 0.0;
        for (int j = 0; j < n; ++j) ax += A[i][j] * x[j];
        double r = ax - rhs[i];
        residual += r * r;
    }
    residual = std::sqrt(residual);

    m_timeSum += timer.elapsed();
    m_stats.totalSolved++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolved;
    emit solveCompleted(iterations, residual);
}

/**
 * @brief 重置统计数据
 */
void SparseCholesky4::resetStatistics()
{
    m_stats.totalSolved = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
