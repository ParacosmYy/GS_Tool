/**
 * @file TridiagonalSolver.cpp
 * @brief Thomas 算法实现
 */

#include "utils/tridiag/TridiagonalSolver.h"

#include <QElapsedTimer>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
TridiagonalSolver::TridiagonalSolver(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief Thomas 算法求解三对角方程组
 *
 * 分两步:
 *   1) 前消: 将上对角线归一化, 修改右端项
 *   2) 回代: 从最后一行向上递推求解
 * 时间复杂度 O(n), 空间复杂度 O(n)。
 */
QVector<double> TridiagonalSolver::solve(QVector<double> lower,
                                         QVector<double> mainDiag,
                                         QVector<double> upper,
                                         QVector<double> rhs)
{
    QElapsedTimer timer;
    timer.start();

    const int n = mainDiag.size();
    QVector<double> c(upper);       /* 修改用的上对角线副本 */
    QVector<double> d(rhs);         /* 修改用的右端项副本 */
    QVector<double> x(n, 0.0);     /* 解向量 */

    /* ---- 前消(Forward sweep) ---- */
    c[0] /= mainDiag[0];
    d[0] /= mainDiag[0];

    for (int i = 1; i < n; ++i) {
        double pivot = mainDiag[i] - lower[i - 1] * c[i - 1];
        if (std::abs(pivot) < 1e-300)
            break;  /* 奇异或近奇异 */

        if (i < n - 1)
            c[i] /= pivot;
        d[i] = (d[i] - lower[i - 1] * d[i - 1]) / pivot;
    }

    /* ---- 回代(Back substitution) ---- */
    x[n - 1] = d[n - 1];
    for (int i = n - 2; i >= 0; --i) {
        x[i] = d[i] - c[i] * x[i + 1];
    }

    m_stats.totalSolves++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
    emit solveCompleted(n);
    return x;
}

/** @brief 重置统计 */
void TridiagonalSolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
