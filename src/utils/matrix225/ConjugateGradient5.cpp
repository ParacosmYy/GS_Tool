/**
 * @file ConjugateGradient5.cpp
 * @brief ConjugateGradient5 实现
 *
 * 实现共轭梯度法：IC(0)预处理、CGS平方变体、非对称系统求解。
 */

#include "utils/matrix225/ConjugateGradient5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

ConjugateGradient5::ConjugateGradient5(QObject *parent) : QObject(parent) {}
ConjugateGradient5::~ConjugateGradient5() = default;

/* ---- Configuration ---- */

void ConjugateGradient5::setParameters(double tolerance, int maxIterations)
{
    m_tolerance = qMax(1e-15, tolerance);
    m_maxIterations = qMax(10, maxIterations);
    m_stats.tolerance = m_tolerance;
    m_stats.maxIterations = m_maxIterations;
}

/* ---- Matrix-vector product ---- */

QVector<double> ConjugateGradient5::matVec(
    const QVector<QVector<double>>& A, const QVector<double>& x) const
{
    int n = x.size();
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            y[i] += A[i][j] * x[j];
    return y;
}

/* ---- Dot product ---- */

double ConjugateGradient5::dot(const QVector<double>& a,
                                const QVector<double>& b) const
{
    double s = 0.0;
    for (int i = 0; i < qMin(a.size(), b.size()); ++i)
        s += a[i] * b[i];
    return s;
}

/* ---- IC(0) preconditioner ---- */

QVector<QVector<double>> ConjugateGradient5::buildIC0Preconditioner(
    const QVector<QVector<double>>& A) const
{
    int n = A.size();
    QVector<QVector<double>> L(n);
    for (int i = 0; i < n; ++i) {
        L[i].resize(n, 0.0);
        for (int j = 0; j <= i; ++j) {
            if (j == i) {
                double sum = 0.0;
                for (int k = 0; k < j; ++k)
                    sum += L[j][k] * L[j][k];
                double diag = A[i][i] - sum;
                L[i][j] = qSqrt(qMax(1e-15, diag));
            } else if (A[i][j] != 0.0) {
                double sum = 0.0;
                for (int k = 0; k < j; ++k)
                    sum += L[i][k] * L[j][k];
                L[i][j] = (A[i][j] - sum) / qMax(1e-15, L[j][j]);
            }
        }
    }
    return L;
}

/* ---- Forward solve ---- */

QVector<double> ConjugateGradient5::forwardSolve(
    const QVector<QVector<double>>& L, const QVector<double>& b) const
{
    int n = b.size();
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double sum = b[i];
        for (int j = 0; j < i; ++j)
            sum -= L[i][j] * y[j];
        y[i] = sum / qMax(1e-15, L[i][i]);
    }
    return y;
}

/* ---- Backward solve ---- */

QVector<double> ConjugateGradient5::backwardSolve(
    const QVector<QVector<double>>& L, const QVector<double>& y) const
{
    int n = y.size();
    QVector<double> x(n, 0.0);
    for (int i = n - 1; i >= 0; --i) {
        double sum = y[i];
        for (int j = i + 1; j < n; ++j)
            sum -= L[j][i] * x[j];
        x[i] = sum / qMax(1e-15, L[i][i]);
    }
    return x;
}

/* ---- Apply preconditioner ---- */

QVector<double> ConjugateGradient5::applyPreconditioner(
    const QVector<QVector<double>>& L, const QVector<double>& r) const
{
    QVector<double> y = forwardSolve(L, r);
    return backwardSolve(L, y);
}

/* ---- Residual norm ---- */

double ConjugateGradient5::residualNorm(const QVector<QVector<double>>& A,
                                          const QVector<double>& x,
                                          const QVector<double>& b) const
{
    QVector<double> Ax = matVec(A, x);
    double norm = 0.0;
    for (int i = 0; i < b.size(); ++i) {
        double r = b[i] - Ax[i];
        norm += r * r;
    }
    return qSqrt(norm);
}

/* ---- PCG solve (symmetric) ---- */

ConjugateGradient5::SolveResult ConjugateGradient5::solve(
    const QVector<QVector<double>>& A, const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    SolveResult result;
    int n = b.size();
    m_stats.matrixSize = n;

    // Initial guess x = 0
    result.solution.resize(n, 0.0);

    // Build IC(0) preconditioner
    auto L = buildIC0Preconditioner(A);

    // r = b - A*x = b
    QVector<double> r = b;
    QVector<double> z = applyPreconditioner(L, r);
    QVector<double> p = z;
    double rz = dot(r, z);

    for (int iter = 0; iter < m_maxIterations; ++iter) {
        QVector<double> Ap = matVec(A, p);
        double pAp = dot(p, Ap);

        if (qAbs(pAp) < 1e-30) break;

        double alpha = rz / pAp;

        for (int i = 0; i < n; ++i)
            result.solution[i] += alpha * p[i];

        double rNorm = 0.0;
        for (int i = 0; i < n; ++i) {
            r[i] -= alpha * Ap[i];
            rNorm += r[i] * r[i];
        }
        rNorm = qSqrt(rNorm);

        if (iter % 50 == 0)
            emit iterationCompleted(iter, rNorm);

        if (rNorm < m_tolerance) {
            result.iterations = iter + 1;
            result.residual = rNorm;
            result.converged = true;
            break;
        }

        z = applyPreconditioner(L, r);
        double rzNew = dot(r, z);
        double beta = rzNew / qMax(1e-30, rz);

        for (int i = 0; i < n; ++i)
            p[i] = z[i] + beta * p[i];

        rz = rzNew;
        result.iterations = iter + 1;
        result.residual = rNorm;
    }

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit solveCompleted(result.iterations, result.residual, timer.elapsed());
    return result;
}

/* ---- CGS solve (non-symmetric) ---- */

ConjugateGradient5::SolveResult ConjugateGradient5::solveCGS(
    const QVector<QVector<double>>& A, const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    SolveResult result;
    int n = b.size();
    m_stats.matrixSize = n;

    result.solution.resize(n, 0.0);

    // CGS: r = b - Ax = b, choose r~ = r
    QVector<double> r = b;
    QVector<double> rtilde = b;
    QVector<double> p = r;
    QVector<double> u = r;

    double rho = dot(rtilde, r);

    for (int iter = 0; iter < m_maxIterations; ++iter) {
        if (qAbs(rho) < 1e-30) break;

        QVector<double> q(n, 0.0);
        QVector<double> Ap = matVec(A, p);

        double sigma = dot(rtilde, Ap);
        if (qAbs(sigma) < 1e-30) break;

        double alpha = rho / sigma;

        // q = u - alpha * Ap
        for (int i = 0; i < n; ++i)
            q[i] = u[i] - alpha * Ap[i];

        // x += alpha * (u + q)
        for (int i = 0; i < n; ++i)
            result.solution[i] += alpha * (u[i] + q[i]);

        // r -= alpha * A*(u + q)
        QVector<double> uq(n);
        for (int i = 0; i < n; ++i) uq[i] = u[i] + q[i];
        QVector<double> Auq = matVec(A, uq);

        double rNorm = 0.0;
        for (int i = 0; i < n; ++i) {
            r[i] -= alpha * Auq[i];
            rNorm += r[i] * r[i];
        }
        rNorm = qSqrt(rNorm);

        if (iter % 50 == 0)
            emit iterationCompleted(iter, rNorm);

        if (rNorm < m_tolerance) {
            result.iterations = iter + 1;
            result.residual = rNorm;
            result.converged = true;
            break;
        }

        double rhoNew = dot(rtilde, r);
        double beta = rhoNew / qMax(1e-30, rho);

        // u = r + beta * q
        // p = u + beta * (q + beta * p)
        QVector<double> newU(n), newP(n);
        for (int i = 0; i < n; ++i) {
            newU[i] = r[i] + beta * q[i];
            newP[i] = newU[i] + beta * (q[i] + beta * p[i]);
        }
        u = newU;
        p = newP;
        rho = rhoNew;

        result.iterations = iter + 1;
        result.residual = rNorm;
    }

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit solveCompleted(result.iterations, result.residual, timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void ConjugateGradient5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
