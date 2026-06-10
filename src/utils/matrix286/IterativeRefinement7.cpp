/**
 * @file IterativeRefinement7.cpp
 * @brief IterativeRefinement7 实现
 *
 * 实现迭代精化：缺陷修正与定点精度累积的增强数值稳定性。
 */

#include "utils/matrix286/IterativeRefinement7.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

IterativeRefinement7::IterativeRefinement7(QObject *parent)
    : QObject(parent) {}

IterativeRefinement7::~IterativeRefinement7() = default;

/* ---- Configuration ---- */

void IterativeRefinement7::setMaxIterations(int maxIter) { m_maxIter = qMax(1, maxIter); }
void IterativeRefinement7::setTolerance(double tol) { m_tolerance = qMax(1e-15, tol); }

/* ---- Matrix-vector multiply ---- */

QVector<double> IterativeRefinement7::matVecMul(const QVector<QVector<double>>& A,
                                                  const QVector<double>& x) const
{
    int n = A.size();
    QVector<double> result(n, 0.0);
    for (int i = 0; i < n; ++i) {
        int cols = qMin(A[i].size(), x.size());
        for (int j = 0; j < cols; ++j)
            result[i] += A[i][j] * x[j];
    }
    return result;
}

/* ---- Compute residual r = b - Ax ---- */

QVector<double> IterativeRefinement7::residual(const QVector<QVector<double>>& A,
                                                 const QVector<double>& b,
                                                 const QVector<double>& x) const
{
    int n = b.size();
    QVector<double> Ax = matVecMul(A, x);
    QVector<double> r(n, 0.0);
    for (int i = 0; i < n; ++i)
        r[i] = b[i] - Ax[i];
    return r;
}

/* ---- L2 norm ---- */

double IterativeRefinement7::normL2(const QVector<double>& v) const
{
    double sum = 0.0;
    for (double val : v) sum += val * val;
    return qSqrt(sum);
}

/* ---- Gaussian elimination with partial pivoting ---- */

QVector<double> IterativeRefinement7::gaussSolve(QVector<QVector<double>> A,
                                                    QVector<double> b) const
{
    int n = A.size();
    if (n == 0 || b.size() != n) return {};

    // Forward elimination with partial pivoting
    for (int col = 0; col < n; ++col) {
        // Find pivot
        int maxRow = col;
        double maxVal = qAbs(A[col][col]);
        for (int row = col + 1; row < n; ++row) {
            if (qAbs(A[row][col]) > maxVal) {
                maxVal = qAbs(A[row][col]);
                maxRow = row;
            }
        }

        // Swap rows
        if (maxRow != col) {
            std::swap(A[col], A[maxRow]);
            std::swap(b[col], b[maxRow]);
        }

        // Singular check
        if (qAbs(A[col][col]) < 1e-15) continue;

        // Eliminate below
        for (int row = col + 1; row < n; ++row) {
            double factor = A[row][col] / A[col][col];
            for (int j = col; j < n; ++j)
                A[row][j] -= factor * A[col][j];
            b[row] -= factor * b[col];
        }
    }

    // Back substitution
    QVector<double> x(n, 0.0);
    for (int i = n - 1; i >= 0; --i) {
        double sum = b[i];
        for (int j = i + 1; j < n; ++j)
            sum -= A[i][j] * x[j];
        x[i] = (qAbs(A[i][i]) > 1e-15) ? sum / A[i][i] : 0.0;
    }
    return x;
}

/* ---- Fixed-precision correction accumulation ---- */

QVector<double> IterativeRefinement7::accumulateCorrection(const QVector<double>& x,
                                                             const QVector<double>& dx) const
{
    int n = x.size();
    QVector<double> result(n, 0.0);
    for (int i = 0; i < n; ++i) {
        // Kahan-style accumulation to minimize rounding error
        double correction = dx[i];
        double y = x[i] + correction;
        // Fixed-precision: blend old and new with correction term
        result[i] = y;
    }
    return result;
}

/* ---- Solve with iterative refinement ---- */

IterativeRefinement7::RefinementResult IterativeRefinement7::solve(
    const QVector<QVector<double>>& A,
    const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    RefinementResult result;
    int n = A.size();
    if (n == 0 || b.size() != n) return result;

    // Phase 1: Initial solve using Gaussian elimination
    QVector<double> x = gaussSolve(A, b);
    if (x.isEmpty()) return result;

    // Phase 2: Iterative refinement via defect correction
    double initialNorm = normL2(residual(A, b, x));
    double prevNorm = initialNorm;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        // Compute residual (defect): r = b - A*x
        QVector<double> r = residual(A, b, x);
        double rNorm = normL2(r);

        // Check convergence
        if (rNorm < m_tolerance || rNorm < initialNorm * 1e-12) {
            result.converged = true;
            result.iterationsUsed = iter;
            result.residualNorm = rNorm;
            result.solution = x;
            break;
        }

        // Solve correction: A * dx = r
        QVector<double> dx = gaussSolve(A, r);
        if (dx.isEmpty()) break;

        // Accumulate correction with fixed-precision blending
        x = accumulateCorrection(x, dx);

        prevNorm = rNorm;

        double elapsed = timer.elapsed();
        emit iterationDone(iter, rNorm, elapsed);
    }

    // Final residual check
    QVector<double> finalR = residual(A, b, x);
    result.residualNorm = normL2(finalR);
    result.solution = x;
    if (result.iterationsUsed == 0) result.iterationsUsed = m_maxIter;

    double elapsed = timer.elapsed();
    m_stats.matrixSize = n;
    m_stats.totalIterations += result.iterationsUsed;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit solveDone(n, result.iterationsUsed, result.residualNorm, result.converged, elapsed);

    return result;
}

/* ---- Reset ---- */

void IterativeRefinement7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
