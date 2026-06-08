/**
 * @file IterativeRefinement3.cpp
 * @brief IterativeRefinement3 实现
 *
 * 实现混合精度迭代精化：LU分解(工作精度)+超精度残差Kahan补偿累加。
 */

#include "utils/matrix230/IterativeRefinement3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cmath>
#include <limits>

/* ---- Construction / Destruction ---- */

IterativeRefinement3::IterativeRefinement3(QObject *parent) : QObject(parent) {}
IterativeRefinement3::~IterativeRefinement3() = default;

/* ---- Configuration ---- */

void IterativeRefinement3::setParameters(double tolerance, int maxIterations)
{
    m_tolerance = qMax(1e-15, tolerance);
    m_maxIterations = qMax(1, maxIterations);
    m_stats.tolerance = m_tolerance;
    m_stats.maxIterations = m_maxIterations;
}

/* ---- Infinity norm ---- */

double IterativeRefinement3::normInf(const QVector<double>& v) const
{
    double maxVal = 0.0;
    for (double x : v) maxVal = qMax(maxVal, qAbs(x));
    return maxVal;
}

/* ---- Kahan compensated dot product ---- */

double IterativeRefinement3::compensatedDot(const QVector<double>& row,
                                             const QVector<double>& x) const
{
    double sum = 0.0;
    double comp = 0.0; // compensation accumulator
    int n = qMin(row.size(), x.size());
    for (int i = 0; i < n; ++i) {
        double y = row[i] * x[i] - comp;
        double t = sum + y;
        comp = (t - sum) - y;
        sum = t;
    }
    return sum;
}

/* ---- Matrix-vector multiply (extra precision) ---- */

QVector<double> IterativeRefinement3::matVec(const QVector<QVector<double>>& A,
                                              const QVector<double>& x) const
{
    int n = A.size();
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i)
        y[i] = compensatedDot(A[i], x);
    return y;
}

/* ---- Compute residual r = b - Ax (extra precision) ---- */

QVector<double> IterativeRefinement3::computeResidual(
    const QVector<QVector<double>>& A,
    const QVector<double>& x,
    const QVector<double>& b) const
{
    QVector<double> ax = matVec(A, x);
    int n = b.size();
    QVector<double> r(n);
    for (int i = 0; i < n; ++i)
        r[i] = b[i] - ax[i];
    return r;
}

/* ---- LU decomposition with partial pivoting ---- */

bool IterativeRefinement3::luDecompose(QVector<QVector<double>>& A,
                                        QVector<int>& pivot) const
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
        if (maxVal < 1e-15) return false; // singular

        // Swap rows
        if (maxRow != k) {
            std::swap(A[k], A[maxRow]);
            std::swap(pivot[k], pivot[maxRow]);
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

QVector<double> IterativeRefinement3::luSolve(const QVector<QVector<double>>& LU,
                                                const QVector<int>& pivot,
                                                const QVector<double>& rhs) const
{
    int n = LU.size();
    QVector<double> y(n);

    // Apply permutation
    for (int i = 0; i < n; ++i)
        y[i] = rhs[pivot[i]];

    // Forward substitution (L * z = y)
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < i; ++j)
            y[i] -= LU[i][j] * y[j];

    // Backward substitution (U * x = z)
    for (int i = n - 1; i >= 0; --i) {
        for (int j = i + 1; j < n; ++j)
            y[i] -= LU[i][j] * y[j];
        y[i] /= LU[i][i];
    }
    return y;
}

/* ---- Solve with iterative refinement ---- */

IterativeRefinement3::RefinementResult IterativeRefinement3::solve(
    const QVector<QVector<double>>& A, const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    RefinementResult result;
    int n = A.size();
    if (n < 1 || b.size() != n) return result;

    m_stats.matrixSize = n;

    // Working-precision LU decomposition
    QVector<QVector<double>> LU = A;
    QVector<int> pivot;
    if (!luDecompose(LU, pivot)) return result;

    // Initial solve in working precision
    result.solution = luSolve(LU, pivot, b);
    result.initialResidual = normInf(computeResidual(A, result.solution, b));

    // Iterative refinement loop
    for (int iter = 0; iter < m_maxIterations; ++iter) {
        // Compute residual in extra precision
        QVector<double> r = computeResidual(A, result.solution, b);
        double resNorm = normInf(r);

        if (iter == 0) result.initialResidual = resNorm;

        // Check convergence
        if (resNorm < m_tolerance || resNorm < m_tolerance * normInf(b)) {
            result.converged = true;
            result.finalResidual = resNorm;
            result.iterations = iter + 1;
            break;
        }

        // Solve for correction in working precision
        QVector<double> dz = luSolve(LU, pivot, r);

        // Apply correction
        for (int i = 0; i < n; ++i)
            result.solution[i] += dz[i];

        result.finalResidual = resNorm;
        result.iterations = iter + 1;
    }

    m_stats.totalRefinements++;
    if (result.converged) m_stats.totalConverged++;
    m_stats.totalOps++;
    double totalIter = m_stats.avgIterations * (m_stats.totalRefinements - 1) + result.iterations;
    m_stats.avgIterations = totalIter / m_stats.totalRefinements;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit refinementCompleted(result.iterations, result.finalResidual, timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void IterativeRefinement3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
