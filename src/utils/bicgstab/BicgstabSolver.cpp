/**
 * @file BicgstabSolver.cpp
 * @brief BiCGSTAB求解器实现 — 稳定双共轭梯度迭代
 */

#include "utils/bicgstab/BicgstabSolver.h"

#include <QElapsedTimer>
#include <cmath>

BicgstabSolver::BicgstabSolver(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 向量点积 */
static double dotProd(const QVector<double>& a, const QVector<double>& b)
{
    double s = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

/** @brief 矩阵向量乘 */
static QVector<double> matVec(const QVector<QVector<double>>& A,
                              const QVector<double>& v)
{
    int n = A.size();
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double s = 0.0;
        for (int j = 0; j < n; ++j) s += A[i][j] * v[j];
        y[i] = s;
    }
    return y;
}

/** @brief 向量二范数 */
static double vecNorm(const QVector<double>& v)
{
    double s = 0.0;
    for (double x : v) s += x * x;
    return std::sqrt(s);
}

/**
 * @brief BiCGSTAB核心迭代
 * @return {x, {iterations, residual}}
 */
static QPair<QVector<double>, QPair<int, double>>
    bicgstabIterate(const QVector<QVector<double>>& A,
                    const QVector<double>& b,
                    double tol, int maxIter)
{
    int n = A.size();
    QVector<double> x(n, 0.0);
    QVector<double> r = b;
    QVector<double> rHat = r;

    double rhoOld = 1.0, alpha = 1.0, omega = 1.0;
    QVector<double> v(n, 0.0), p(n, 0.0);

    double bNorm = vecNorm(b);
    double residual = vecNorm(r) / bNorm;
    int iter = 0;

    for (iter = 0; iter < maxIter; ++iter) {
        double rho = dotProd(rHat, r);
        if (std::abs(rho) < 1e-30) break;

        double beta = (rho / rhoOld) * (alpha / omega);
        for (int i = 0; i < n; ++i)
            p[i] = r[i] + beta * (p[i] - omega * v[i]);

        v = matVec(A, p);
        double rHatDotV = dotProd(rHat, v);
        if (std::abs(rHatDotV) < 1e-30) break;
        alpha = rho / rHatDotV;

        QVector<double> s(n);
        for (int i = 0; i < n; ++i) s[i] = r[i] - alpha * v[i];

        QVector<double> t = matVec(A, s);
        double tDotT = dotProd(t, t);
        omega = (tDotT > 1e-30) ? dotProd(t, s) / tDotT : 0.0;

        for (int i = 0; i < n; ++i)
            x[i] += alpha * p[i] + omega * s[i];
        for (int i = 0; i < n; ++i)
            r[i] = s[i] - omega * t[i];

        residual = vecNorm(r) / bNorm;
        if (residual < tol) { ++iter; break; }
        rhoOld = rho;
    }
    return {x, {iter, residual}};
}

QVector<double> BicgstabSolver::solve(
    const QVector<QVector<double>>& matA,
    const QVector<double>& vecB,
    double tol, int maxIter)
{
    QElapsedTimer timer;
    timer.start();

    int n = matA.size();
    if (n == 0 || vecB.size() != n) {
        emit solveCompleted(0, 0.0);
        return {};
    }

    double bNorm = vecNorm(vecB);
    if (bNorm < 1e-30) {
        emit solveCompleted(0, 0.0);
        return QVector<double>(n, 0.0);
    }

    auto [x, info] = bicgstabIterate(matA, vecB, tol, maxIter);

    ++m_stats.totalSolves;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(info.first, info.second);
    return x;
}

void BicgstabSolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
