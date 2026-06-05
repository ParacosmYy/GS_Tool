/**
 * @file ConjugateGradient.cpp
 * @brief 共轭梯度法实现 — 对称正定线性系统求解
 */

#include "utils/conjugate/ConjugateGradient.h"

#include <QElapsedTimer>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
ConjugateGradient::ConjugateGradient(QObject* parent)
    : QObject(parent)
{
}

/** @brief 求解对称正定线性系统 */
QVector<double> ConjugateGradient::solve(const QVector<QVector<double>>& A,
                                          const QVector<double>& b,
                                          int maxIter, double tol)
{
    QElapsedTimer timer;
    timer.start();

    int n = b.size();
    if (n == 0 || A.size() != n) return {};

    /* 初始猜测 x = 0 */
    QVector<double> x(n, 0.0);

    /* r = b - A*x = b */
    QVector<double> r = b;

    /* p = r */
    QVector<double> p = r;

    double rsOld = dotProduct(r, r);

    for (int iter = 0; iter < maxIter; ++iter) {
        QVector<double> Ap = matVecMul(A, p);
        double pAp = dotProduct(p, Ap);

        if (std::abs(pAp) < 1e-300) break;

        double alpha = rsOld / pAp;

        for (int i = 0; i < n; ++i) {
            x[i] += alpha * p[i];
            r[i] -= alpha * Ap[i];
        }

        double rsNew = dotProduct(r, r);

        /* 收敛判断 */
        if (std::sqrt(rsNew) < tol) {
            m_stats.totalIterations += iter + 1;
            break;
        }

        if (iter == maxIter - 1) {
            m_stats.totalIterations += maxIter;
        }

        double beta = rsNew / rsOld;
        for (int i = 0; i < n; ++i)
            p[i] = r[i] + beta * p[i];

        rsOld = rsNew;
    }

    double residual = std::sqrt(dotProduct(r, r));

    m_stats.totalSolves++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(static_cast<int>(m_stats.totalIterations), residual);
    return x;
}

/** @brief 重置统计 */
void ConjugateGradient::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 向量内积 */
double ConjugateGradient::dotProduct(const QVector<double>& a,
                                      const QVector<double>& b) const
{
    double sum = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i)
        sum += a[i] * b[i];
    return sum;
}

/** @brief 矩阵-向量乘法 */
QVector<double> ConjugateGradient::matVecMul(
    const QVector<QVector<double>>& A,
    const QVector<double>& x) const
{
    int n = A.size();
    QVector<double> result(n, 0.0);
    for (int i = 0; i < n; ++i) {
        int m = qMin(A[i].size(), x.size());
        for (int j = 0; j < m; ++j)
            result[i] += A[i][j] * x[j];
    }
    return result;
}
