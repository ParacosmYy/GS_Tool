/**
 * @file CglsSolver.cpp
 * @brief 共轭梯度最小二乘求解器实现 — CGLS迭代
 */

#include "utils/cgls/CglsSolver.h"

#include <QElapsedTimer>
#include <cmath>

CglsSolver::CglsSolver(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 计算 A^T * v */
static QVector<double> matTransVec(const QVector<QVector<double>>& A,
                                   const QVector<double>& v)
{
    int m = A.size();
    if (m == 0) return {};
    int n = A[0].size();
    QVector<double> y(n, 0.0);
    for (int j = 0; j < n; ++j) {
        double s = 0.0;
        for (int i = 0; i < m; ++i) s += A[i][j] * v[i];
        y[j] = s;
    }
    return y;
}

/** @brief 计算 A * v */
static QVector<double> matVec(const QVector<QVector<double>>& A,
                              const QVector<double>& v)
{
    int m = A.size();
    int n = A[0].size();
    QVector<double> y(m, 0.0);
    for (int i = 0; i < m; ++i) {
        double s = 0.0;
        for (int j = 0; j < n; ++j) s += A[i][j] * v[j];
        y[i] = s;
    }
    return y;
}

/**
 * @brief CGLS核心迭代循环
 * @return {x, iterations, residual}
 */
static QPair<QVector<double>, QPair<int, double>>
    cglsIterate(const QVector<QVector<double>>& A,
                const QVector<double>& b, int n,
                double tol, int maxIter)
{
    QVector<double> x(n, 0.0);
    QVector<double> r = b;
    QVector<double> p = matTransVec(A, r);

    double gamma = 0.0;
    for (double v : p) gamma += v * v;

    double bNormSq = 0.0;
    for (double v : b) bNormSq += v * v;
    if (bNormSq < 1e-30) return {x, {0, 0.0}};

    int iter = 0;
    double residual = 1.0;

    for (iter = 0; iter < maxIter; ++iter) {
        QVector<double> q = matVec(A, p);

        double qNormSq = 0.0;
        for (double v : q) qNormSq += v * v;
        if (qNormSq < 1e-30) break;

        double alpha = gamma / qNormSq;

        for (int j = 0; j < n; ++j) x[j] += alpha * p[j];
        for (int i = 0; i < q.size(); ++i) r[i] -= alpha * q[i];

        QVector<double> s = matTransVec(A, r);
        double gammaNew = 0.0;
        for (double v : s) gammaNew += v * v;

        double rNormSq = 0.0;
        for (double v : r) rNormSq += v * v;
        residual = std::sqrt(rNormSq / bNormSq);
        if (residual < tol) { ++iter; break; }

        double beta = gammaNew / gamma;
        for (int j = 0; j < n; ++j) p[j] = s[j] + beta * p[j];
        gamma = gammaNew;
    }
    return {x, {iter, residual}};
}

QVector<double> CglsSolver::solve(
    const QVector<QVector<double>>& matA,
    const QVector<double>& vecB,
    double tol, int maxIter)
{
    QElapsedTimer timer;
    timer.start();

    int m = matA.size();
    if (m == 0) { emit solveCompleted(0, 0.0); return {}; }
    int n = matA[0].size();
    if (n == 0 || vecB.size() != m) {
        emit solveCompleted(0, 0.0);
        return QVector<double>(n, 0.0);
    }

    auto [x, info] = cglsIterate(matA, vecB, n, tol, maxIter);
    int iter = info.first;
    double residual = info.second;

    ++m_stats.totalSolves;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(iter, residual);
    return x;
}

void CglsSolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
