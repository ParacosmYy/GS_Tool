/**
 * @file JacobiSolver.cpp
 * @brief Jacobi迭代求解实现
 */

#include "utils/jacobi_iter/JacobiSolver.h"

#include <QElapsedTimer>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
JacobiSolver::JacobiSolver(QObject* parent)
    : QObject(parent)
{
}

/** @brief Jacobi迭代求解 */
QVector<double> JacobiSolver::solve(const QVector<QVector<double>>& A,
                                      const QVector<double>& b,
                                      double tol, int maxIter)
{
    QElapsedTimer timer;
    timer.start();

    int n = b.size();
    if (n == 0 || A.size() != n) return {};

    QVector<double> x(n, 0.0);
    QVector<double> xNew(n, 0.0);
    int iter = 0;

    for (iter = 0; iter < maxIter; ++iter) {
        double maxDiff = 0.0;

        for (int i = 0; i < n; ++i) {
            double sigma = 0.0;
            for (int j = 0; j < n; ++j) {
                if (j != i) sigma += A[i][j] * x[j];
            }

            if (std::abs(A[i][i]) < 1e-300) {
                xNew[i] = x[i];
                continue;
            }

            xNew[i] = (b[i] - sigma) / A[i][i];
            double diff = std::abs(xNew[i] - x[i]);
            if (diff > maxDiff) maxDiff = diff;
        }

        x = xNew;
        if (maxDiff < tol) break;
    }

    double residual = 0.0;
    for (int i = 0; i < n; ++i) {
        double ax = 0.0;
        for (int j = 0; j < n; ++j) ax += A[i][j] * x[j];
        residual += (ax - b[i]) * (ax - b[i]);
    }
    residual = std::sqrt(residual);

    m_stats.totalSolves++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(iter + 1, residual);
    return x;
}

/** @brief 重置统计 */
void JacobiSolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
