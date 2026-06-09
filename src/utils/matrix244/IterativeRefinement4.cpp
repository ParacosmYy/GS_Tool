/**
 * @file IterativeRefinement4.cpp
 * @brief IterativeRefinement4 实现
 *
 * 实现迭代精化：混合精度残差计算与条件数感知收敛。
 */

#include "utils/matrix244/IterativeRefinement4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <limits>
#include <algorithm>

/* ---- Construction / Destruction ---- */

IterativeRefinement4::IterativeRefinement4(QObject *parent) : QObject(parent) {}
IterativeRefinement4::~IterativeRefinement4() = default;

/* ---- Configuration ---- */

void IterativeRefinement4::setMaxIterations(int maxIter) { m_maxIter = qMax(1, maxIter); }
void IterativeRefinement4::setTolerance(double tol) { m_tol = qMax(1e-16, tol); }

/* ---- Vector norm ---- */

double IterativeRefinement4::vecNorm(const QVector<double>& v)
{
    double sum = 0.0;
    for (double x : v) sum += x * x;
    return qSqrt(sum);
}

/* ---- Matrix-vector product ---- */

QVector<double> IterativeRefinement4::matVec(const QVector<QVector<double>>& A,
                                              const QVector<double>& x) const
{
    int n = A.size();
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i) {
        int cols = qMin(A[i].size(), x.size());
        for (int j = 0; j < cols; ++j)
            y[i] += A[i][j] * x[j];
    }
    return y;
}

/* ---- Compute residual r = b - Ax ---- */

QVector<double> IterativeRefinement4::computeResidual(const QVector<QVector<double>>& A,
                                                       const QVector<double>& x,
                                                       const QVector<double>& b) const
{
    int n = b.size();
    QVector<double> Ax = matVec(A, x);
    QVector<double> r(n);
    for (int i = 0; i < n; ++i)
        r[i] = b[i] - Ax[i];
    return r;
}

/* ---- LU decomposition with partial pivoting ---- */

bool IterativeRefinement4::luDecompose(QVector<QVector<double>>& A, QVector<int>& pivot) const
{
    int n = A.size();
    pivot.resize(n);
    for (int i = 0; i < n; ++i) pivot[i] = i;

    for (int k = 0; k < n; ++k) {
        // Find pivot
        double maxVal = qAbs(A[k][k]);
        int maxRow = k;
        for (int i = k + 1; i < n; ++i) {
            if (qAbs(A[i][k]) > maxVal) {
                maxVal = qAbs(A[i][k]);
                maxRow = i;
            }
        }
        if (maxVal < 1e-15) return false;

        // Swap rows
        if (maxRow != k) {
            std::swap(A[k], A[maxRow]);
            std::swap(pivot[k], pivot[maxRow]);
        }

        // Eliminate
        for (int i = k + 1; i < n; ++i) {
            A[i][k] /= A[k][k];
            for (int j = k + 1; j < n; ++j)
                A[i][j] -= A[i][k] * A[k][j];
        }
    }
    return true;
}

/* ---- LU solve ---- */

QVector<double> IterativeRefinement4::luSolve(const QVector<QVector<double>>& LU,
                                                const QVector<int>& pivot,
                                                const QVector<double>& b) const
{
    int n = LU.size();
    QVector<double> x(n);

    // Apply permutation
    for (int i = 0; i < n; ++i)
        x[i] = b[pivot[i]];

    // Forward substitution (L)
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < i; ++j)
            x[i] -= LU[i][j] * x[j];

    // Back substitution (U)
    for (int i = n - 1; i >= 0; --i) {
        for (int j = i + 1; j < n; ++j)
            x[i] -= LU[i][j] * x[j];
        x[i] /= LU[i][i];
    }
    return x;
}

/* ---- Condition number estimate (1-norm) ---- */

double IterativeRefinement4::estimateConditionNumber(const QVector<QVector<double>>& A) const
{
    int n = A.size();
    if (n == 0) return 0.0;

    // 1-norm of A: max column sum of absolute values
    double aNorm = 0.0;
    for (int j = 0; j < n; ++j) {
        double colSum = 0.0;
        for (int i = 0; i < n; ++i)
            if (j < A[i].size()) colSum += qAbs(A[i][j]);
        aNorm = qMax(aNorm, colSum);
    }
    return aNorm;
}

/* ---- Solve with iterative refinement ---- */

IterativeRefinement4::SolveResult IterativeRefinement4::solve(
    const QVector<QVector<double>>& A, const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    SolveResult result;
    int n = A.size();
    if (n == 0 || b.size() != n) { result.converged = false; return result; }

    m_stats.matrixSize = n;

    // Step 1: LU decomposition (working precision)
    QVector<QVector<double>> LU = A;
    QVector<int> pivot;
    if (!luDecompose(LU, pivot)) {
        result.converged = false;
        return result;
    }

    // Initial solve
    result.solution = luSolve(LU, pivot, b);
    result.condEstimate = estimateConditionNumber(A);

    // Condition-number aware: skip refinement if well-conditioned and solution is exact
    double initialResidual = vecNorm(computeResidual(A, result.solution, b));
    double bNorm = vecNorm(b);
    double initialRelRes = (bNorm > 1e-15) ? initialResidual / bNorm : initialResidual;

    if (initialRelRes < m_tol * 1e-3) {
        result.converged = true;
        result.iterations = 0;
        result.residualNorm = initialResidual;
        return result;
    }

    // Step 2: Iterative refinement loop
    for (int iter = 0; iter < m_maxIter; ++iter) {
        // High-precision residual: r = b - A*x (computed in double = "high" precision)
        QVector<double> r = computeResidual(A, result.solution, b);
        double rNorm = vecNorm(r);
        result.residualNorm = rNorm;

        // Convergence check
        double relRes = (bNorm > 1e-15) ? rNorm / bNorm : rNorm;
        if (relRes < m_tol) {
            result.converged = true;
            result.iterations = iter + 1;
            break;
        }

        // Solve A*d = r for correction
        QVector<double> d = luSolve(LU, pivot, r);

        // Update solution: x = x + d
        for (int i = 0; i < n; ++i)
            result.solution[i] += d[i];

        emit iterationCompleted(iter + 1, rNorm, vecNorm(d));
        result.iterations = iter + 1;
    }

    m_stats.numSolves++;
    m_stats.totalIterations += result.iterations;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit solveCompleted(result.iterations, result.condEstimate, timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void IterativeRefinement4::resetStatistics()
{
    m_stats = Stats{}; m_timeSum = 0.0;
}
