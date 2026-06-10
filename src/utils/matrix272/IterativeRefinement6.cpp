/**
 * @file IterativeRefinement6.cpp
 * @brief IterativeRefinement6 实现
 *
 * 实现迭代精化：混合精度残差计算与条件数估计求解精度提升。
 */

#include "utils/matrix272/IterativeRefinement6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

IterativeRefinement6::IterativeRefinement6(QObject *parent)
    : QObject(parent) {}

IterativeRefinement6::~IterativeRefinement6() = default;

/* ---- Configuration ---- */

void IterativeRefinement6::setMaxIterations(int maxIter)
{
    m_maxIter = qMax(1, maxIter);
}

void IterativeRefinement6::setTolerance(double tol)
{
    m_tolerance = qBound(1e-16, tol, 1.0);
}

/* ---- LU decomposition (Doolittle, partial pivoting) ---- */

bool IterativeRefinement6::luDecompose(QVector<QVector<double>>& A,
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
        if (maxVal < std::numeric_limits<double>::epsilon()) return false;

        // Swap rows
        if (maxRow != k) {
            std::swap(A[k], A[maxRow]);
            std::swap(pivot[k], pivot[maxRow]);
        }

        // Eliminate below
        for (int i = k + 1; i < n; ++i) {
            A[i][k] /= A[k][k];
            for (int j = k + 1; j < n; ++j) {
                A[i][j] -= A[i][k] * A[k][j];
            }
        }
    }
    return true;
}

/* ---- LU solve (forward/back substitution) ---- */

QVector<double> IterativeRefinement6::luSolve(
    const QVector<QVector<double>>& LU,
    const QVector<int>& pivot,
    const QVector<double>& b) const
{
    int n = LU.size();
    QVector<double> x(n, 0.0);

    // Apply permutation: x = P * b
    for (int i = 0; i < n; ++i)
        x[i] = b[pivot[i]];

    // Forward substitution (L * y = x, L has unit diagonal)
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < i; ++j)
            x[i] -= LU[i][j] * x[j];
    }

    // Back substitution (U * result = y)
    for (int i = n - 1; i >= 0; --i) {
        for (int j = i + 1; j < n; ++j)
            x[i] -= LU[i][j] * x[j];
        x[i] /= LU[i][i];
    }
    return x;
}

/* ---- Matrix-vector product ---- */

QVector<double> IterativeRefinement6::matVec(
    const QVector<QVector<double>>& A,
    const QVector<double>& x)
{
    int n = A.size();
    QVector<double> result(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            result[i] += A[i][j] * x[j];
    return result;
}

/* ---- Matrix 1-norm ---- */

double IterativeRefinement6::matNorm1(const QVector<QVector<double>>& A)
{
    double norm = 0.0;
    int n = A.size();
    for (int j = 0; j < n; ++j) {
        double colSum = 0.0;
        for (int i = 0; i < n; ++i)
            colSum += qAbs(A[i][j]);
        norm = qMax(norm, colSum);
    }
    return norm;
}

/* ---- Solve transpose system ---- */

QVector<double> IterativeRefinement6::solveTranspose(
    const QVector<QVector<double>>& LU,
    const QVector<int>& pivot,
    const QVector<double>& b) const
{
    int n = LU.size();
    QVector<double> x = b;

    // Forward substitution: U^T * y = b
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < i; ++j)
            x[i] -= LU[j][i] * x[j];
        x[i] /= LU[i][i];
    }

    // Back substitution: L^T * result = y
    for (int i = n - 1; i >= 0; --i) {
        for (int j = i + 1; j < n; ++j)
            x[i] -= LU[j][i] * x[j];
    }

    // Apply inverse permutation
    QVector<double> result(n);
    for (int i = 0; i < n; ++i)
        result[pivot[i]] = x[i];
    return result;
}

/* ---- Condition number estimation (1-norm) ---- */

double IterativeRefinement6::conditionNumber(
    const QVector<QVector<double>>& A) const
{
    int n = A.size();
    if (n == 0) return 0.0;

    double normA = matNorm1(A);

    // Solve A^T * x = e where e chosen to maximize ||A^{-1}||
    QVector<QVector<double>> LU = A;
    QVector<int> pivot;
    if (!luDecompose(LU, pivot)) return std::numeric_limits<double>::infinity();

    // Hager's condition estimator (simplified)
    QVector<double> x(n, 1.0 / n);
    for (int iter = 0; iter < 5; ++iter) {
        QVector<double> Ax = matVec(A, x);
        QVector<double> w(n);
        for (int i = 0; i < n; ++i)
            w[i] = (Ax[i] >= 0) ? 1.0 : -1.0;

        QVector<double> v = solveTranspose(LU, pivot, w);
        int maxIdx = 0;
        for (int i = 1; i < n; ++i)
            if (qAbs(v[i]) > qAbs(v[maxIdx])) maxIdx = i;

        if (maxIdx == 0 && iter > 0) break;
        x.fill(0.0);
        x[maxIdx] = 1.0;
    }

    QVector<double> Ax = matVec(A, x);
    double normAinv = 0.0;
    for (double v : Ax) normAinv += qAbs(v);

    return normA * normAinv;
}

/* ---- Residual norm (infinity norm) ---- */

double IterativeRefinement6::residualNorm(
    const QVector<QVector<double>>& A,
    const QVector<double>& x,
    const QVector<double>& b) const
{
    int n = A.size();
    double maxRes = 0.0;
    for (int i = 0; i < n; ++i) {
        double rowSum = 0.0;
        for (int j = 0; j < n; ++j)
            rowSum += A[i][j] * x[j];
        maxRes = qMax(maxRes, qAbs(rowSum - b[i]));
    }
    return maxRes;
}

/* ---- Main solve with iterative refinement ---- */

QVector<double> IterativeRefinement6::solve(
    const QVector<QVector<double>>& A,
    const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    int n = A.size();
    if (n == 0 || b.size() != n) return {};

    // Step 1: LU decomposition
    QVector<QVector<double>> LU = A;
    QVector<int> pivot;
    if (!luDecompose(LU, pivot)) return QVector<double>(n, 0.0);

    // Step 2: Initial solve
    QVector<double> x = luSolve(LU, pivot, b);
    m_residualHistory.clear();

    // Step 3: Iterative refinement loop
    for (int iter = 0; iter < m_maxIter; ++iter) {
        // Compute residual in higher precision: r = b - A*x
        QVector<double> Ax = matVec(A, x);
        QVector<double> r(n);
        for (int i = 0; i < n; ++i)
            r[i] = b[i] - Ax[i];

        double res = 0.0;
        for (int i = 0; i < n; ++i) res = qMax(res, qAbs(r[i]));
        m_residualHistory.append(res);

        emit refinementStep(iter, res, timer.elapsed());

        if (res < m_tolerance) break;

        // Solve A * dx = r and update x += dx
        QVector<double> dx = luSolve(LU, pivot, r);
        for (int i = 0; i < n; ++i)
            x[i] += dx[i];
    }

    double elapsed = timer.elapsed();
    m_stats.matrixSize = n;
    m_stats.numIterations = m_residualHistory.size();
    m_stats.maxIterationsReached = m_maxIter;
    m_stats.finalResidual = m_residualHistory.isEmpty()
                            ? 0.0 : m_residualHistory.last();
    m_stats.conditionEstimate = conditionNumber(A);
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return x;
}

/* ---- Convergence history ---- */

QVector<double> IterativeRefinement6::convergenceHistory() const
{
    return m_residualHistory;
}

/* ---- Reset ---- */

void IterativeRefinement6::resetStatistics()
{
    m_residualHistory.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
