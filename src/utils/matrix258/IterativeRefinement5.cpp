/**
 * @file IterativeRefinement5.cpp
 * @brief IterativeRefinement5 实现
 *
 * 实现迭代精化：混合精度累加与残差校正近机器精度收敛。
 */

#include "utils/matrix258/IterativeRefinement5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <limits>

/* ---- Construction / Destruction ---- */

IterativeRefinement5::IterativeRefinement5(QObject *parent)
    : QObject(parent) {}
IterativeRefinement5::~IterativeRefinement5() = default;

/* ---- Configuration ---- */

void IterativeRefinement5::setTolerance(double tol)
{
    m_stats.tolerance = qMax(1e-16, tol);
}

void IterativeRefinement5::setMaxIterations(int maxIter)
{
    m_stats.maxIterations = qMax(1, maxIter);
}

/* ---- LU decomposition with partial pivoting ---- */

void IterativeRefinement5::decomposeLU(const QVector<QVector<double>>& A)
{
    int n = A.size();
    m_lu = A;
    m_pivot.resize(n);
    for (int i = 0; i < n; ++i) m_pivot[i] = i;

    for (int k = 0; k < n; ++k) {
        // Find pivot
        double maxVal = qAbs(m_lu[k][k]);
        int maxRow = k;
        for (int i = k + 1; i < n; ++i) {
            if (qAbs(m_lu[i][k]) > maxVal) {
                maxVal = qAbs(m_lu[i][k]);
                maxRow = i;
            }
        }

        // Swap rows
        if (maxRow != k) {
            std::swap(m_lu[k], m_lu[maxRow]);
            std::swap(m_pivot[k], m_pivot[maxRow]);
        }

        if (qFuzzyIsNull(m_lu[k][k])) continue;

        // Eliminate below
        for (int i = k + 1; i < n; ++i) {
            m_lu[i][k] /= m_lu[k][k];
            for (int j = k + 1; j < n; ++j) {
                m_lu[i][j] -= m_lu[i][k] * m_lu[k][j];
            }
        }
    }
}

/* ---- Solve LU x = b ---- */

QVector<double> IterativeRefinement5::solveLU(const QVector<double>& b) const
{
    int n = m_lu.size();
    QVector<double> x(n, 0.0);

    // Forward substitution (Ly = Pb)
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i) {
        y[i] = b[m_pivot[i]];
        for (int j = 0; j < i; ++j)
            y[i] -= m_lu[i][j] * y[j];
    }

    // Back substitution (Ux = y)
    for (int i = n - 1; i >= 0; --i) {
        x[i] = y[i];
        for (int j = i + 1; j < n; ++j)
            x[i] -= m_lu[i][j] * x[j];
        if (!qFuzzyIsNull(m_lu[i][i]))
            x[i] /= m_lu[i][i];
    }

    return x;
}

/* ---- Compute residual with high precision ---- */

QVector<double> IterativeRefinement5::computeResidual(
    const QVector<QVector<double>>& A,
    const QVector<double>& b,
    const QVector<double>& x) const
{
    int n = x.size();
    QVector<double> r(n, 0.0);
    for (int i = 0; i < n; ++i) {
        r[i] = b[i];
        for (int j = 0; j < n; ++j)
            r[i] -= A[i][j] * x[j];
    }
    return r;
}

/* ---- Infinity norm ---- */

double IterativeRefinement5::infNorm(const QVector<double>& v)
{
    double norm = 0.0;
    for (double val : v)
        norm = qMax(norm, qAbs(val));
    return norm;
}

/* ---- Solve Ax = b with iterative refinement ---- */

QVector<double> IterativeRefinement5::solve(
    const QVector<QVector<double>>& A,
    const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    int n = A.size();
    if (n == 0 || b.size() != n) return {};

    m_stats.matrixSize = n;
    m_residualHistory.clear();

    // Step 1: LU decomposition (lower precision)
    decomposeLU(A);

    // Step 2: Initial solve
    QVector<double> x = solveLU(b);

    // Compute initial residual
    QVector<double> r = computeResidual(A, b, x);
    double initRes = infNorm(r);
    m_stats.initialResidual = initRes;
    m_residualHistory.append(initRes);

    // Step 3: Iterative refinement loop
    int iter = 0;
    while (iter < m_stats.maxIterations) {
        // Solve A * dz = r using existing LU decomposition
        QVector<double> dz = solveLU(r);

        // Apply correction
        for (int i = 0; i < n; ++i)
            x[i] += dz[i];

        // Recompute residual with full precision
        r = computeResidual(A, b, x);
        double res = infNorm(r);
        m_residualHistory.append(res);

        iter++;

        // Check convergence
        if (res < m_stats.tolerance || res > initRes * 1e10) break;
    }

    m_stats.numIterations = iter;
    m_stats.finalResidual = m_residualHistory.last();
    m_stats.converged = (m_stats.finalResidual < m_stats.tolerance);
    m_stats.totalOps++;

    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit solveCompleted(iter, m_stats.finalResidual, elapsed);
    return x;
}

/* ---- Residual history ---- */

QVector<double> IterativeRefinement5::residualHistory() const
{
    return m_residualHistory;
}

/* ---- Reset ---- */

void IterativeRefinement5::resetStatistics()
{
    m_lu.clear();
    m_pivot.clear();
    m_residualHistory.clear();
    double tol = m_stats.tolerance;
    int maxIter = m_stats.maxIterations;
    m_stats = Stats{};
    m_stats.tolerance = tol;
    m_stats.maxIterations = maxIter;
    m_timeSum = 0.0;
}
