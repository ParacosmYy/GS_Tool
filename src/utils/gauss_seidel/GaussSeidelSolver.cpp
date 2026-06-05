/**
 * @file GaussSeidelSolver.cpp
 * @brief Gauss-Seidel迭代求解实现
 */

#include "utils/gauss_seidel/GaussSeidelSolver.h"

#include <QElapsedTimer>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
GaussSeidelSolver::GaussSeidelSolver(QObject* parent)
    : QObject(parent)
{
}

/** @brief Gauss-Seidel迭代求解 */
QVector<double> GaussSeidelSolver::solve(const QVector<QVector<double>>& A,
                                           const QVector<double>& b,
                                           double tol, int maxIter)
{
    QElapsedTimer timer;
    timer.start();

    int n = b.size();
    if (n == 0 || A.size() != n) return {};

    QVector<double> x(n, 0.0);
    int iter = 0;

    for (iter = 0; iter < maxIter; ++iter) {
        double maxDiff = 0.0;

        for (int i = 0; i < n; ++i) {
            double sigma = 0.0;
            for (int j = 0; j < n; ++j) {
                if (j != i) sigma += A[i][j] * x[j];
            }

            if (std::abs(A[i][i]) < 1e-300) continue;

            double xNew = (b[i] - sigma) / A[i][i];
            double diff = std::abs(xNew - x[i]);
            if (diff > maxDiff) maxDiff = diff;
            x[i] = xNew;
        }

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
void GaussSeidelSolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
