/**
 * @file ConjugateGradient.cpp
 * @brief ConjugateGradient 实现
 *
 * 实现共轭梯度法：Jacobi预条件、残差范数监控、迭代历史记录。
 */

#include "utils/matrix184/ConjugateGradient.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

ConjugateGradient::ConjugateGradient(QObject *parent) : QObject(parent) {}
ConjugateGradient::~ConjugateGradient() = default;

/* ---- Configuration ---- */

void ConjugateGradient::setMaxIterations(int iter) { m_maxIter = qMax(1, iter); }
void ConjugateGradient::setTolerance(double tol) { m_tol = qMax(1e-15, tol); }
void ConjugateGradient::setPreconditioner(bool enabled) { m_usePrecond = enabled; }

/* ---- Utility: dot product ---- */

double ConjugateGradient::dot(const QVector<double>& a,
                                const QVector<double>& b) const
{
    double sum = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) sum += a[i] * b[i];
    return sum;
}

/* ---- Utility: vector norm ---- */

double ConjugateGradient::norm(const QVector<double>& v) const
{
    return qSqrt(dot(v, v));
}

/* ---- Diagonal extraction ---- */

QVector<double> ConjugateGradient::diagonal(
    const QVector<QVector<double>>& A) const
{
    int n = A.size();
    QVector<double> diag(n);
    for (int i = 0; i < n; ++i) diag[i] = A[i][i];
    return diag;
}

/* ---- Jacobi preconditioner: M^{-1} r ---- */

QVector<double> ConjugateGradient::jacobiPrecond(
    const QVector<double>& r, const QVector<double>& diag) const
{
    int n = r.size();
    QVector<double> z(n);
    for (int i = 0; i < n; ++i)
        z[i] = (qAbs(diag[i]) > 1e-15) ? r[i] / diag[i] : r[i];
    return z;
}

/* ---- Matrix-vector multiply ---- */

QVector<double> ConjugateGradient::matVecMultiply(
    const QVector<QVector<double>>& A, const QVector<double>& x) const
{
    int n = A.size();
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            y[i] += A[i][j] * x[j];
    return y;
}

/* ---- Residual computation ---- */

QVector<double> ConjugateGradient::residual(
    const QVector<QVector<double>>& A, const QVector<double>& x,
    const QVector<double>& b) const
{
    int n = b.size();
    auto Ax = matVecMultiply(A, x);
    QVector<double> r(n);
    for (int i = 0; i < n; ++i) r[i] = b[i] - Ax[i];
    return r;
}

/* ---- Main solve (dense format) ---- */

ConjugateGradient::SolveResult ConjugateGradient::solve(
    const QVector<QVector<double>>& A, const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    SolveResult result;
    int n = b.size();
    if (n == 0 || A.size() != n) {
        result.converged = false;
        return result;
    }

    result.solution.resize(n, 0.0);
    result.residualHistory.reserve(m_maxIter);

    // r0 = b - A*x0 (x0 = 0)
    QVector<double> r = b;
    double rNorm0 = norm(r);
    result.initialResidual = rNorm0;

    if (rNorm0 < m_tol) {
        result.converged = true;
        result.iterations = 0;
        result.residualNorm = rNorm0;
        return result;
    }

    // Apply preconditioner
    QVector<double> diag = diagonal(A);
    QVector<double> z = m_usePrecond ? jacobiPrecond(r, diag) : r;

    // p0 = z0
    QVector<double> p = z;
    double rzOld = dot(r, z);

    for (int iter = 0; iter < m_maxIter; ++iter) {
        // alpha = (r^T z) / (p^T A p)
        auto Ap = matVecMultiply(A, p);
        double pAp = dot(p, Ap);
        if (qAbs(pAp) < 1e-30) break;
        double alpha = rzOld / pAp;

        // Update solution and residual
        for (int i = 0; i < n; ++i) {
            result.solution[i] += alpha * p[i];
            r[i] -= alpha * Ap[i];
        }

        double rNorm = norm(r);
        result.residualHistory.append(rNorm);

        if (rNorm < m_tol * rNorm0 || rNorm < m_tol) {
            result.converged = true;
            result.iterations = iter + 1;
            result.residualNorm = rNorm;
            break;
        }

        // Apply preconditioner
        z = m_usePrecond ? jacobiPrecond(r, diag) : r;
        double rzNew = dot(r, z);

        // Update search direction
        double beta = rzNew / (rzOld + 1e-30);
        for (int i = 0; i < n; ++i)
            p[i] = z[i] + beta * p[i];

        rzOld = rzNew;
        result.iterations = iter + 1;
        result.residualNorm = rNorm;
    }

    m_stats.totalSolves++;
    m_stats.matrixSize = n;
    m_stats.iterationsUsed = result.iterations;
    m_stats.finalResidual = result.residualNorm;
    m_stats.initialResidual = rNorm0;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(result.iterations, result.residualNorm, result.converged);
    return result;
}

/* ---- Solve sparse format ---- */

ConjugateGradient::SolveResult ConjugateGradient::solveSparse(
    const QVector<QVector<int>>& colIdx,
    const QVector<QVector<double>>& values,
    const QVector<double>& b, int n)
{
    // Convert sparse to dense for simplicity
    QVector<QVector<double>> A(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < colIdx[i].size() && j < values[i].size(); ++j) {
            int col = colIdx[i][j];
            if (col >= 0 && col < n)
                A[i][col] = values[i][j];
        }
    }
    return solve(A, b);
}

/* ---- Reset ---- */

void ConjugateGradient::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
