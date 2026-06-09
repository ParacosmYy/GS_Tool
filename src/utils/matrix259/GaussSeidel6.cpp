/**
 * @file GaussSeidel6.cpp
 * @brief GaussSeidel6 实现
 *
 * 实现高斯-赛德尔迭代：加权Jacobi预处理与Chebyshev加速非对称收敛。
 */

#include "utils/matrix259/GaussSeidel6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

GaussSeidel6::GaussSeidel6(QObject *parent)
    : QObject(parent) {}
GaussSeidel6::~GaussSeidel6() = default;

/* ---- Configuration ---- */

void GaussSeidel6::setMaxIterations(int maxIter) { m_maxIter = qMax(1, maxIter); }
void GaussSeidel6::setTolerance(double tol) { m_tolerance = qMax(1e-15, tol); }
void GaussSeidel6::setJacobiWeight(double w) { m_jacobiWeight = qBound(0.1, w, 1.9); }
void GaussSeidel6::setChebyshevAcceleration(bool enable) { m_useChebyshev = enable; }

/* ---- Compute residual L2 norm ---- */

double GaussSeidel6::residualNorm(const QVector<QVector<double>>& A,
                                   const QVector<double>& x,
                                   const QVector<double>& b) const
{
    int n = b.size();
    double sum = 0.0;
    for (int i = 0; i < n; ++i) {
        double ax = 0.0;
        for (int j = 0; j < n; ++j)
            ax += A[i][j] * x[j];
        double r = b[i] - ax;
        sum += r * r;
    }
    return qSqrt(sum);
}

/* ---- Estimate spectral radius range ---- */

void GaussSeidel6::estimateSpectralRadius(const QVector<QVector<double>>& A,
                                            double& rhoMin, double& rhoMax) const
{
    int n = A.size();
    rhoMin = std::numeric_limits<double>::max();
    rhoMax = 0.0;

    for (int i = 0; i < n; ++i) {
        double rowSum = 0.0;
        double diag = qFabs(A[i][i]);
        for (int j = 0; j < n; ++j) {
            if (j != i) rowSum += qFabs(A[i][j]);
        }
        if (diag > 1e-15) {
            double ratio = rowSum / diag;
            rhoMax = qMax(rhoMax, ratio);
            rhoMin = qMin(rhoMin, ratio);
        }
    }
    rhoMin = qMax(0.0, rhoMin);
    rhoMax = qBound(rhoMin + 0.01, rhoMax, 0.999);
}

/* ---- Gauss-Seidel sweep (forward) ---- */

void GaussSeidel6::gsSweep(const QVector<QVector<double>>& A,
                             const QVector<double>& b,
                             QVector<double>& x) const
{
    int n = x.size();
    for (int i = 0; i < n; ++i) {
        double sigma = 0.0;
        for (int j = 0; j < n; ++j) {
            if (j != i) sigma += A[i][j] * x[j];
        }
        if (qFabs(A[i][i]) > 1e-15)
            x[i] = (b[i] - sigma) / A[i][i];
    }
}

/* ---- Weighted Jacobi preconditioning ---- */

void GaussSeidel6::jacobiPrecond(const QVector<QVector<double>>& A,
                                  const QVector<double>& b,
                                  QVector<double>& x) const
{
    int n = x.size();
    QVector<double> xNew(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double sigma = 0.0;
        for (int j = 0; j < n; ++j) {
            if (j != i) sigma += A[i][j] * x[j];
        }
        if (qFabs(A[i][i]) > 1e-15) {
            double xJacobi = (b[i] - sigma) / A[i][i];
            xNew[i] = (1.0 - m_jacobiWeight) * x[i] + m_jacobiWeight * xJacobi;
        } else {
            xNew[i] = x[i];
        }
    }
    x = xNew;
}

/* ---- Chebyshev acceleration ---- */

void GaussSeidel6::chebyshevAccelerate(QVector<double>& x,
                                         const QVector<double>& xPrev,
                                         int iter, double rhoMin, double rhoMax,
                                         double& delta, double& deltaPrev) const
{
    double c = (rhoMax - rhoMin) / 2.0;
    double d = (rhoMax + rhoMin) / 2.0;
    double deltaNew = 0.0;

    if (iter <= 1) {
        deltaNew = 2.0 / (2.0 * d - c * c);
    } else {
        double denom = d - c * c * deltaPrev / 4.0;
        if (qFabs(denom) > 1e-15)
            deltaNew = 1.0 / denom;
        else
            deltaNew = delta;
    }

    // Apply acceleration: x = xPrev + delta * (x - xPrev)
    for (int i = 0; i < x.size(); ++i) {
        x[i] = xPrev[i] + deltaNew * (x[i] - xPrev[i]);
    }

    deltaPrev = delta;
    delta = deltaNew;
}

/* ---- Main solver ---- */

QVector<double> GaussSeidel6::solve(const QVector<QVector<double>>& A,
                                     const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    int n = b.size();
    if (n == 0 || A.size() != n) return {};

    QVector<double> x(n, 0.0);
    m_residualHistory.clear();

    // Estimate spectral radius for Chebyshev
    double rhoMin = 0.0, rhoMax = 0.95;
    if (m_useChebyshev) {
        estimateSpectralRadius(A, rhoMin, rhoMax);
    }

    double delta = 1.0, deltaPrev = 0.0;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        QVector<double> xPrev = x;

        // Step 1: Weighted Jacobi preconditioning
        jacobiPrecond(A, b, x);

        // Step 2: Gauss-Seidel sweep
        gsSweep(A, b, x);

        // Step 3: Chebyshev acceleration (optional)
        if (m_useChebyshev && iter > 0) {
            chebyshevAccelerate(x, xPrev, iter, rhoMin, rhoMax,
                                delta, deltaPrev);
        }

        // Check convergence
        double res = residualNorm(A, x, b);
        m_residualHistory.append(res);

        if (res < m_tolerance) {
            m_stats.iterationsUsed = iter + 1;
            m_stats.finalResidual = res;
            break;
        }

        m_stats.iterationsUsed = iter + 1;
        m_stats.finalResidual = res;
    }

    m_stats.matrixSize = n;
    m_stats.totalOps++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit solveCompleted(m_stats.iterationsUsed, m_stats.finalResidual, elapsed);
    return x;
}

/* ---- Residual history ---- */

QVector<double> GaussSeidel6::residualHistory() const
{
    return m_residualHistory;
}

/* ---- Reset ---- */

void GaussSeidel6::resetStatistics()
{
    m_residualHistory.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
