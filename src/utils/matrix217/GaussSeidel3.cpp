/**
 * @file GaussSeidel3.cpp
 * @brief GaussSeidel3 实现
 *
 * 实现Gauss-Seidel迭代：SOR松弛、红黑排序、收敛监测。
 */

#include "utils/matrix217/GaussSeidel3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

GaussSeidel3::GaussSeidel3(QObject *parent) : QObject(parent) {}
GaussSeidel3::~GaussSeidel3() = default;

/* ---- Configuration ---- */

void GaussSeidel3::setOmega(double omega)
{
    m_omega = qBound(0.0, omega, 2.0);
}

void GaussSeidel3::setConvergence(double tolerance, int maxIterations)
{
    m_tolerance = qMax(1e-15, tolerance);
    m_maxIter = qMax(1, maxIterations);
}

/* ---- Single SOR sweep ---- */

void GaussSeidel3::sweepSOR(QVector<double>& x,
                             const QVector<QVector<double>>& A,
                             const QVector<double>& b,
                             double omega)
{
    int n = x.size();
    for (int i = 0; i < n; ++i) {
        double sigma = 0.0;
        for (int j = 0; j < n; ++j) {
            if (j != i && i < A.size() && j < A[i].size())
                sigma += A[i][j] * x[j];
        }
        double diag = (i < A.size() && i < A[i].size()) ? A[i][i] : 1.0;
        if (qAbs(diag) > 1e-15)
            x[i] = (1.0 - omega) * x[i] + omega * (b[i] - sigma) / diag;
    }
}

/* ---- Red sweep (even-indexed unknowns) ---- */

void GaussSeidel3::sweepRed(QVector<double>& x,
                             const QVector<QVector<double>>& A,
                             const QVector<double>& b,
                             double omega)
{
    int n = x.size();
    for (int i = 0; i < n; i += 2) {  // Red = even indices
        double sigma = 0.0;
        for (int j = 0; j < n; ++j) {
            if (j != i && i < A.size() && j < A[i].size())
                sigma += A[i][j] * x[j];
        }
        double diag = (i < A.size() && i < A[i].size()) ? A[i][i] : 1.0;
        if (qAbs(diag) > 1e-15)
            x[i] = (1.0 - omega) * x[i] + omega * (b[i] - sigma) / diag;
    }
}

/* ---- Black sweep (odd-indexed unknowns) ---- */

void GaussSeidel3::sweepBlack(QVector<double>& x,
                               const QVector<QVector<double>>& A,
                               const QVector<double>& b,
                               double omega)
{
    int n = x.size();
    for (int i = 1; i < n; i += 2) {  // Black = odd indices
        double sigma = 0.0;
        for (int j = 0; j < n; ++j) {
            if (j != i && i < A.size() && j < A[i].size())
                sigma += A[i][j] * x[j];
        }
        double diag = (i < A.size() && i < A[i].size()) ? A[i][i] : 1.0;
        if (qAbs(diag) > 1e-15)
            x[i] = (1.0 - omega) * x[i] + omega * (b[i] - sigma) / diag;
    }
}

/* ---- Compute residual ---- */

double GaussSeidel3::residual(const QVector<QVector<double>>& A,
                               const QVector<double>& x,
                               const QVector<double>& b)
{
    int n = x.size();
    double norm = 0.0;
    for (int i = 0; i < n; ++i) {
        double ax = 0.0;
        for (int j = 0; j < n; ++j) {
            if (i < A.size() && j < A[i].size())
                ax += A[i][j] * x[j];
        }
        double r = ax - b[i];
        norm += r * r;
    }
    return qSqrt(norm);
}

/* ---- Solve (standard Gauss-Seidel with SOR) ---- */

QVector<double> GaussSeidel3::solve(const QVector<QVector<double>>& A,
                                     const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    int n = b.size();
    QVector<double> x(n, 0.0);
    if (n == 0 || A.size() < static_cast<size_t>(n)) return x;

    int iter = 0;
    double res = std::numeric_limits<double>::max();
    bool converged = false;

    for (iter = 0; iter < m_maxIter; ++iter) {
        sweepSOR(x, A, b, m_omega);
        res = residual(A, x, b);
        if (res < m_tolerance) {
            converged = true;
            break;
        }
    }

    m_stats.totalSolves++;
    m_stats.matrixSize = n;
    m_stats.iterations = iter;
    m_stats.omega = m_omega;
    m_stats.finalResidual = res;
    m_stats.converged = converged;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
    emit solveCompleted(iter, res, timer.elapsed());

    return x;
}

/* ---- Solve with red-black ordering ---- */

QVector<double> GaussSeidel3::solveRedBlack(const QVector<QVector<double>>& A,
                                             const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    int n = b.size();
    QVector<double> x(n, 0.0);
    if (n == 0 || A.size() < static_cast<size_t>(n)) return x;

    int iter = 0;
    double res = std::numeric_limits<double>::max();
    bool converged = false;

    for (iter = 0; iter < m_maxIter; ++iter) {
        // Red sweep: update all even-indexed unknowns (independent of each other)
        sweepRed(x, A, b, m_omega);
        // Black sweep: update all odd-indexed unknowns
        sweepBlack(x, A, b, m_omega);

        res = residual(A, x, b);
        if (res < m_tolerance) {
            converged = true;
            break;
        }
    }

    m_stats.totalSolves++;
    m_stats.matrixSize = n;
    m_stats.iterations = iter;
    m_stats.omega = m_omega;
    m_stats.finalResidual = res;
    m_stats.converged = converged;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
    emit solveCompleted(iter, res, timer.elapsed());

    return x;
}

/* ---- Reset ---- */

void GaussSeidel3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
