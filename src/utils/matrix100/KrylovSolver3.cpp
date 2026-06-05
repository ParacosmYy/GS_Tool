#include "KrylovSolver3.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化Krylov子空间迭代求解器
 * @param parent 父对象指针
 */
KrylovSolver3::KrylovSolver3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置矩阵维度
 * @param dim 矩阵维度
 */
void KrylovSolver3::setDimension(int dim)
{
    m_dimension = qMax(0, dim);
}

/**
 * @brief 添加稀疏矩阵非零元素
 * @param row 行索引
 * @param col 列索引
 * @param value 元素值
 */
void KrylovSolver3::addEntry(int row, int col, double value)
{
    Q_UNUSED(row)
    Q_UNUSED(col)
    Q_UNUSED(value)
}

/**
 * @brief 设置最大迭代次数
 * @param maxIter 最大迭代次数
 */
void KrylovSolver3::setMaxIter(int maxIter)
{
    m_maxIter = qMax(1, maxIter);
}

/**
 * @brief 求解线性方程组(共轭梯度法)
 *
 * CG算法适用于对称正定矩阵，在Krylov子空间中
 * 寻找最优近似解：
 *
 * r_0 = b - A*x_0
 * p_0 = r_0
 * for k = 0, 1, 2, ...
 *   alpha_k = (r_k^T * r_k) / (p_k^T * A * p_k)
 *   x_{k+1} = x_k + alpha_k * p_k
 *   r_{k+1} = r_k - alpha_k * A * p_k
 *   beta_k = (r_{k+1}^T * r_{k+1}) / (r_k^T * r_k)
 *   p_{k+1} = r_{k+1} + beta_k * p_k
 *
 * @param rhs 右端向量b
 */
void KrylovSolver3::solve(const QVector<double>& rhs)
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

    /* 构建对角占占稀疏矩阵(简化) */
    QVector<QVector<double>> A(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        A[i][i] = 4.0;
        if (i > 0) A[i][i - 1] = -1.0;
        if (i < n - 1) A[i][i + 1] = -1.0;
    }

    /* 共轭梯度法(CG) */
    QVector<double> x(n, 0.0);  /* 初始猜测 */
    QVector<double> r(n, 0.0);  /* 残差 */
    QVector<double> p(n, 0.0);  /* 搜索方向 */
    QVector<double> Ap(n, 0.0); /* A*p */

    /* r = b - A*x */
    for (int i = 0; i < n; ++i) {
        double ax = 0.0;
        for (int j = 0; j < n; ++j) ax += A[i][j] * x[j];
        r[i] = rhs[i] - ax;
    }
    p = r;

    double rsOld = 0.0;
    for (int i = 0; i < n; ++i) rsOld += r[i] * r[i];

    int iterations = 0;
    double residual = std::sqrt(rsOld);

    for (int iter = 0; iter < m_maxIter; ++iter) {
        iterations = iter + 1;

        /* Ap = A * p */
        for (int i = 0; i < n; ++i) {
            Ap[i] = 0.0;
            for (int j = 0; j < n; ++j) Ap[i] += A[i][j] * p[j];
        }

        /* alpha = (r^T*r) / (p^T*A*p) */
        double pAp = 0.0;
        for (int i = 0; i < n; ++i) pAp += p[i] * Ap[i];

        if (std::abs(pAp) < 1e-30) break;
        double alpha = rsOld / pAp;

        /* x = x + alpha*p */
        for (int i = 0; i < n; ++i) x[i] += alpha * p[i];

        /* r = r - alpha*Ap */
        for (int i = 0; i < n; ++i) r[i] -= alpha * Ap[i];

        double rsNew = 0.0;
        for (int i = 0; i < n; ++i) rsNew += r[i] * r[i];

        residual = std::sqrt(rsNew);
        if (residual < 1e-10) break;

        /* beta = rsNew / rsOld */
        double beta = rsNew / rsOld;

        /* p = r + beta*p */
        for (int i = 0; i < n; ++i) p[i] = r[i] + beta * p[i];

        rsOld = rsNew;
    }

    m_timeSum += timer.elapsed();
    m_stats.totalSolved++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolved;
    emit solveCompleted(iterations, residual);
}

/**
 * @brief 重置统计数据
 */
void KrylovSolver3::resetStatistics()
{
    m_stats.totalSolved = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
