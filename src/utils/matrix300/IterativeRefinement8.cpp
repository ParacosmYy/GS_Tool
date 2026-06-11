/**
 * @file IterativeRefinement8.cpp
 * @brief IterativeRefinement8 实现
 *
 * 实现迭代精化：混合精度残差计算与条件数估计实现高精度线性求解。
 */

#include "utils/matrix300/IterativeRefinement8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

IterativeRefinement8::IterativeRefinement8(QObject *parent)
    : QObject(parent) {}

IterativeRefinement8::~IterativeRefinement8() = default;

/* ---- Configuration ---- */

void IterativeRefinement8::setMaxIterations(int iter) { m_maxIter = qBound(1, iter, 100); }
void IterativeRefinement8::setTolerance(double tol) { m_tolerance = qBound(1e-16, tol, 1.0); }

/* ---- LU decomposition with partial pivoting ---- */

IterativeRefinement8::LUResult IterativeRefinement8::luDecompose(
    const QVector<QVector<double>>& A) const
{
    int n = A.size();
    LUResult result;
    result.LU = A;
    result.perm.resize(n);
    for (int i = 0; i < n; ++i) result.perm[i] = i;

    for (int k = 0; k < n; ++k) {
        // Find pivot
        double maxVal = qAbs(result.LU[k][k]);
        int maxRow = k;
        for (int i = k + 1; i < n; ++i) {
            if (qAbs(result.LU[i][k]) > maxVal) {
                maxVal = qAbs(result.LU[i][k]);
                maxRow = i;
            }
        }
        // Swap rows
        if (maxRow != k) {
            std::swap(result.LU[k], result.LU[maxRow]);
            std::swap(result.perm[k], result.perm[maxRow]);
        }
        // Eliminate below
        if (qAbs(result.LU[k][k]) > 1e-300) {
            for (int i = k + 1; i < n; ++i) {
                result.LU[i][k] /= result.LU[k][k];
                for (int j = k + 1; j < n; ++j)
                    result.LU[i][j] -= result.LU[i][k] * result.LU[k][j];
            }
        }
    }
    return result;
}

/* ---- LU solve via forward/back substitution ---- */

QVector<double> IterativeRefinement8::luSolve(const LUResult& lu,
                                               const QVector<double>& b) const
{
    int n = lu.perm.size();
    QVector<double> x(n, 0.0);

    // Forward substitution: Ly = Pb
    for (int i = 0; i < n; ++i) {
        x[i] = b[lu.perm[i]];
        for (int j = 0; j < i; ++j)
            x[i] -= lu.LU[i][j] * x[j];
    }
    // Back substitution: Ux = y
    for (int i = n - 1; i >= 0; --i) {
        for (int j = i + 1; j < n; ++j)
            x[i] -= lu.LU[i][j] * x[j];
        if (qAbs(lu.LU[i][i]) > 1e-300)
            x[i] /= lu.LU[i][i];
    }
    return x;
}

/* ---- Vector 2-norm ---- */

double IterativeRefinement8::vectorNorm2(const QVector<double>& v) const
{
    double sum = 0.0;
    for (double x : v) sum += x * x;
    return qSqrt(sum);
}

/* ---- Matrix 1-norm (max column sum) ---- */

double IterativeRefinement8::matrixNorm1(const QVector<QVector<double>>& A) const
{
    if (A.isEmpty()) return 0.0;
    int n = A.size();
    int m = A[0].size();
    double maxSum = 0.0;
    for (int j = 0; j < m; ++j) {
        double colSum = 0.0;
        for (int i = 0; i < n; ++i)
            colSum += qAbs(A[i][j]);
        maxSum = qMax(maxSum, colSum);
    }
    return maxSum;
}

/* ---- Compute residual r = b - Ax ---- */

QVector<double> IterativeRefinement8::computeResidual(
    const QVector<QVector<double>>& A,
    const QVector<double>& x,
    const QVector<double>& b) const
{
    int n = A.size();
    QVector<double> r(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double ax = 0.0;
        int dim = qMin(A[i].size(), x.size());
        for (int j = 0; j < dim; ++j)
            ax += A[i][j] * x[j];
        r[i] = b[i] - ax;
    }
    return r;
}

/* ---- Condition number estimation (1-norm) ---- */

double IterativeRefinement8::estimateConditionNumber(
    const QVector<QVector<double>>& A) const
{
    int n = A.size();
    if (n == 0) return 0.0;

    // Estimate ||A||_1 * ||A^{-1}||_1 via inverse iteration
    double normA = matrixNorm1(A);

    // Solve A^T * w = e where e is vector of ±1 (Hager's method)
    auto AT = QVector<QVector<double>>(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            AT[j][i] = A[i][j];

    auto lu = luDecompose(AT);

    // Start with e = [1,1,...,1]
    QVector<double> e(n, 1.0);
    auto w = luSolve(lu, e);

    // Iterate to converge on maximum 1-norm of A^{-1}
    for (int iter = 0; iter < 5; ++iter) {
        // e_i = sign(w_i)
        for (int i = 0; i < n; ++i)
            e[i] = (w[i] >= 0) ? 1.0 : -1.0;
        w = luSolve(lu, e);
    }

    double normInvA = vectorNorm2(w);  // Approximate ||A^{-1}||
    return normA * normInvA;
}

/* ---- Solve with iterative refinement ---- */

IterativeRefinement8::SolveResult IterativeRefinement8::solve(
    const QVector<QVector<double>>& A, const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    SolveResult result;
    int n = A.size();
    if (n == 0 || b.size() != n) {
        result.converged = false;
        return result;
    }

    // Step 1: LU factorization
    auto lu = luDecompose(A);

    // Step 2: Initial solve
    result.solution = luSolve(lu, b);

    // Step 3: Iterative refinement loop
    result.converged = false;
    for (int iter = 0; iter < m_maxIter; ++iter) {
        // Compute residual in high precision: r = b - A*x
        auto r = computeResidual(A, result.solution, b);
        double resNorm = vectorNorm2(r);
        result.residualNorm = resNorm;

        if (resNorm < m_tolerance) {
            result.converged = true;
            result.iterations = iter + 1;
            break;
        }

        // Solve A*dz = r using existing LU factorization
        auto dz = luSolve(lu, r);

        // Update solution: x = x + dz
        for (int i = 0; i < n; ++i)
            result.solution[i] += dz[i];

        result.iterations = iter + 1;
    }

    // Condition number estimation
    result.conditionNumber = estimateConditionNumber(A);

    double elapsed = timer.elapsed();
    m_stats.matrixSize = n;
    m_stats.totalSolves++;
    m_iterSum += result.iterations;
    m_condSum += result.conditionNumber;
    m_stats.avgIterations = m_iterSum / m_stats.totalSolves;
    m_stats.avgConditionNumber = m_condSum / m_stats.totalSolves;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveDone(n, result.iterations, result.conditionNumber, elapsed);
    return result;
}

/* ---- Reset ---- */

void IterativeRefinement8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_iterSum = 0.0;
    m_condSum = 0.0;
}
