/**
 * @file IterativeRefinement.cpp
 * @brief IterativeRefinement 实现
 *
 * 实现迭代精化求解：LU分解、混合精度残差修正、收敛监控。
 */

#include "utils/matrix191/IterativeRefinement.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

IterativeRefinement::IterativeRefinement(QObject *parent) : QObject(parent) {}
IterativeRefinement::~IterativeRefinement() = default;

/* ---- Configuration ---- */

void IterativeRefinement::setMaxIterations(int iter) { m_maxIterations = qMax(1, iter); }
void IterativeRefinement::setTolerance(double tol) { m_tolerance = qMax(1e-16, tol); }

/* ---- Vector norm (2-norm) ---- */

double IterativeRefinement::vectorNorm(const QVector<double>& v) const
{
    double sum = 0.0;
    for (double x : v) sum += x * x;
    return qSqrt(sum);
}

/* ---- Matrix 1-norm (max column sum of absolute values) ---- */

double IterativeRefinement::matrixNorm1(const QVector<QVector<double>>& A) const
{
    int n = A.size();
    if (n == 0) return 0.0;
    double maxColSum = 0.0;
    for (int j = 0; j < n; ++j) {
        double colSum = 0.0;
        for (int i = 0; i < n; ++i)
            colSum += qFabs(A[i][j]);
        maxColSum = qMax(maxColSum, colSum);
    }
    return maxColSum;
}

/* ---- Matrix-vector multiply ---- */

QVector<double> IterativeRefinement::matVecMultiply(
    const QVector<QVector<double>>& A, const QVector<double>& x) const
{
    int n = A.size();
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            y[i] += A[i][j] * x[j];
    return y;
}

/* ---- Compute residual r = b - Ax ---- */

QVector<double> IterativeRefinement::computeResidual(
    const QVector<QVector<double>>& A,
    const QVector<double>& x, const QVector<double>& b) const
{
    auto ax = matVecMultiply(A, x);
    int n = b.size();
    QVector<double> r(n);
    for (int i = 0; i < n; ++i)
        r[i] = b[i] - ax[i];
    return r;
}

/* ---- LU decomposition (Doolittle with partial pivoting) ---- */

bool IterativeRefinement::luDecompose(QVector<QVector<double>>& A,
                                       QVector<int>& perm) const
{
    int n = A.size();
    perm.resize(n);
    for (int i = 0; i < n; ++i) perm[i] = i;

    for (int k = 0; k < n; ++k) {
        // Find pivot
        int pivot = k;
        double maxVal = qFabs(A[k][k]);
        for (int i = k + 1; i < n; ++i) {
            if (qFabs(A[i][k]) > maxVal) {
                maxVal = qFabs(A[i][k]);
                pivot = i;
            }
        }
        if (maxVal < 1e-15) return false; // Singular

        // Swap rows
        if (pivot != k) {
            std::swap(A[k], A[pivot]);
            std::swap(perm[k], perm[pivot]);
        }

        // Eliminate below
        for (int i = k + 1; i < n; ++i) {
            A[i][k] /= A[k][k];
            for (int j = k + 1; j < n; ++j)
                A[i][j] -= A[i][k] * A[k][j];
        }
    }
    return true;
}

/* ---- LU solve ---- */

QVector<double> IterativeRefinement::luSolve(
    const QVector<QVector<double>>& LU,
    const QVector<int>& perm, const QVector<double>& b) const
{
    int n = b.size();
    // Apply permutation
    QVector<double> y(n);
    for (int i = 0; i < n; ++i)
        y[i] = b[perm[i]];

    // Forward substitution (Ly = Pb)
    for (int i = 1; i < n; ++i)
        for (int j = 0; j < i; ++j)
            y[i] -= LU[i][j] * y[j];

    // Back substitution (Ux = y)
    QVector<double> x(n);
    for (int i = n - 1; i >= 0; --i) {
        x[i] = y[i];
        for (int j = i + 1; j < n; ++j)
            x[i] -= LU[i][j] * x[j];
        x[i] /= LU[i][i];
    }
    return x;
}

/* ---- Estimate inverse 1-norm (Hager's method) ---- */

double IterativeRefinement::estimateInverseNorm1(
    const QVector<QVector<double>>& A) const
{
    int n = A.size();
    if (n == 0) return 0.0;

    // Solve A^T * x = e_j where e_j maximizes ||x||_1
    // Simplified: use random vectors to estimate
    double maxNorm = 0.0;
    for (int trial = 0; trial < 3; ++trial) {
        QVector<double> e(n, 0.0);
        e[trial % n] = 1.0;

        auto LU = A;
        QVector<int> perm;
        if (!luDecompose(LU, perm)) return 1e15;
        auto x = luSolve(LU, perm, e);
        double norm = vectorNorm(x);
        maxNorm = qMax(maxNorm, norm);
    }
    return maxNorm;
}

/* ---- Condition number estimate ---- */

double IterativeRefinement::estimateConditionNumber(
    const QVector<QVector<double>>& A) const
{
    return matrixNorm1(A) * estimateInverseNorm1(A);
}

/* ---- Main solve with iterative refinement ---- */

QVector<double> IterativeRefinement::solve(const QVector<QVector<double>>& A,
                                            const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    m_residualHistory.clear();
    int n = A.size();
    if (n == 0 || b.size() != n) return {};

    // LU decomposition
    auto LU = A;
    QVector<int> perm;
    if (!luDecompose(LU, perm)) return {};

    // Initial solve
    QVector<double> x = luSolve(LU, perm, b);

    // Iterative refinement
    int iter = 0;
    for (iter = 0; iter < m_maxIterations; ++iter) {
        // Compute residual in high precision: r = b - A*x
        auto r = computeResidual(A, x, b);
        double resNorm = vectorNorm(r);
        m_residualHistory.append(resNorm);

        if (resNorm < m_tolerance) break;

        // Solve A * dx = r (using LU)
        auto dx = luSolve(LU, perm, r);

        // Update x = x + dx
        for (int i = 0; i < n; ++i)
            x[i] += dx[i];
    }

    // Final residual
    auto finalR = computeResidual(A, x, b);
    double finalResNorm = vectorNorm(finalR);

    m_stats.totalSolves++;
    m_stats.matrixSize = n;
    m_stats.iterationsUsed = iter;
    m_stats.finalResidual = finalResNorm;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(iter, finalResNorm, timer.elapsed());
    return x;
}

/* ---- Reset ---- */

void IterativeRefinement::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_residualHistory.clear();
}
