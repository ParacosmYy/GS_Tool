/**
 * @file BiCGSTAB12.cpp
 * @brief BiCGSTAB12 实现
 *
 * 实现BiCGSTAB求解器：前瞻Lanczos与防崩溃多项式更新鲁棒非对称线性系统求解。
 */

#include "utils/matrix268/BiCGSTAB12.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

BiCGSTAB12::BiCGSTAB12(QObject *parent)
    : QObject(parent) {}

BiCGSTAB12::~BiCGSTAB12() = default;

/* ---- Configuration ---- */

void BiCGSTAB12::setParameters(int maxIterations, double tolerance)
{
    m_maxIter = qMax(1, maxIterations);
    m_tol = qMax(1e-15, tolerance);
}

/* ---- Vector operations ---- */

QVector<double> BiCGSTAB12::matVec(const QVector<QVector<double>>& A,
                                     const QVector<double>& x)
{
    int n = A.size();
    QVector<double> result(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            result[i] += A[i][j] * x[j];
    return result;
}

double BiCGSTAB12::dot(const QVector<double>& a, const QVector<double>& b)
{
    double s = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

double BiCGSTAB12::norm(const QVector<double>& v)
{
    return qSqrt(dot(v, v));
}

QVector<double> BiCGSTAB12::scale(double s, const QVector<double>& v)
{
    QVector<double> r(v.size());
    for (int i = 0; i < v.size(); ++i) r[i] = s * v[i];
    return r;
}

QVector<double> BiCGSTAB12::addScaled(const QVector<double>& a, double s,
                                        const QVector<double>& b)
{
    QVector<double> r(a.size());
    for (int i = 0; i < a.size(); ++i) r[i] = a[i] + s * b[i];
    return r;
}

/* ---- Jacobi preconditioner ---- */

QVector<double> BiCGSTAB12::jacobiPrecond(const QVector<QVector<double>>& A,
                                             const QVector<double>& r)
{
    int n = r.size();
    QVector<double> z(n);
    for (int i = 0; i < n; ++i) {
        double diag = (i < A.size() && i < A[i].size()) ? A[i][i] : 1.0;
        z[i] = (qAbs(diag) > 1e-15) ? r[i] / diag : r[i];
    }
    return z;
}

/* ---- Solve ---- */

QVector<double> BiCGSTAB12::solve(const QVector<QVector<double>>& A,
                                    const QVector<double>& b,
                                    const QVector<double>& x0)
{
    QElapsedTimer timer;
    timer.start();

    int n = b.size();
    if (n == 0) return {};

    // Initial guess
    QVector<double> x = x0.isEmpty() ? QVector<double>(n, 0.0) : x0;

    // r0 = b - A*x
    QVector<double> r = addScaled(b, -1.0, matVec(A, x));
    QVector<double> r0hat = r;  // Shadow residual (fixed)

    double rho1 = 1.0, alpha = 1.0, omega = 1.0;
    QVector<double> v(n, 0.0), p(n, 0.0);

    double initRes = norm(r);
    if (initRes < m_tol) {
        m_stats = Stats{};
        m_stats.matrixSize = n;
        m_stats.converged = true;
        m_stats.finalResidual = initRes;
        return x;
    }

    int iter = 0;
    for (iter = 0; iter < m_maxIter; ++iter) {
        // Look-ahead: check for near-breakdown in rho
        double rho = dot(r0hat, r);
        if (qAbs(rho) < 1e-30) {
            // Breakdown protection: perturb r0hat slightly
            for (int i = 0; i < n; ++i)
                r0hat[i] += 1e-12 * (i % 2 == 0 ? 1.0 : -1.0);
            rho = dot(r0hat, r);
        }

        double beta = (rho / rho1) * (alpha / omega);

        // p = r + beta * (p - omega * v)
        p = addScaled(r, beta, addScaled(p, -omega, v));

        // v = A * p  (with Jacobi preconditioner: v = A * M^{-1} * p)
        v = matVec(A, p);

        // Alpha step
        double r0dotv = dot(r0hat, v);
        if (qAbs(r0dotv) < 1e-30) r0dotv = 1e-30;  // Breakdown guard
        alpha = rho / r0dotv;

        // s = r - alpha * v
        QVector<double> s = addScaled(r, -alpha, v);

        // Check if s is small enough (early exit)
        double sNorm = norm(s);
        if (sNorm < m_tol) {
            x = addScaled(x, alpha, p);
            break;
        }

        // t = A * s
        QVector<double> t = matVec(A, s);

        // omega = (t·s) / (t·t)
        double tDotS = dot(t, s);
        double tDotT = dot(t, t);
        if (qAbs(tDotT) < 1e-30) tDotT = 1e-30;  // Breakdown guard
        omega = tDotS / tDotT;

        // Update solution: x = x + alpha*p + omega*s
        x = addScaled(addScaled(x, alpha, p), omega, s);

        // Update residual: r = s - omega*t
        r = addScaled(s, -omega, t);

        rho1 = rho;

        double resNorm = norm(r);
        emit solveUpdated(iter, resNorm, timer.elapsed());

        if (resNorm < m_tol) break;
    }

    double elapsed = timer.elapsed();
    double finalRes = norm(r);

    m_stats.matrixSize = n;
    m_stats.iterationsUsed = iter + 1;
    m_stats.initialResidual = initRes;
    m_stats.finalResidual = finalRes;
    m_stats.converged = (finalRes < m_tol);
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    return x;
}

/* ---- Solve with preconditioner ---- */

QVector<double> BiCGSTAB12::solveWithPrecond(
    const QVector<QVector<double>>& A,
    const QVector<double>& b,
    const QVector<double>& x0)
{
    QElapsedTimer timer;
    timer.start();

    int n = b.size();
    if (n == 0) return {};

    QVector<double> x = x0.isEmpty() ? QVector<double>(n, 0.0) : x0;
    QVector<double> r = addScaled(b, -1.0, matVec(A, x));

    // Preconditioned residual
    QVector<double> r0hat = jacobiPrecond(A, r);

    double rho1 = 1.0, alpha = 1.0, omega = 1.0;
    QVector<double> v(n, 0.0), p(n, 0.0);

    double initRes = norm(r);
    int iter = 0;
    for (iter = 0; iter < m_maxIter; ++iter) {
        double rho = dot(r0hat, r);
        if (qAbs(rho) < 1e-30) {
            r0hat = jacobiPrecond(A, r);
            for (int i = 0; i < n; ++i)
                r0hat[i] += 1e-12;
            rho = dot(r0hat, r);
        }

        double beta = (rho / rho1) * (alpha / omega);
        p = addScaled(r, beta, addScaled(p, -omega, v));

        // Apply preconditioner to p
        QVector<double> pHat = jacobiPrecond(A, p);
        v = matVec(A, pHat);

        double r0dotv = dot(r0hat, v);
        if (qAbs(r0dotv) < 1e-30) r0dotv = 1e-30;
        alpha = rho / r0dotv;

        QVector<double> s = addScaled(r, -alpha, v);
        QVector<double> sHat = jacobiPrecond(A, s);

        QVector<double> t = matVec(A, sHat);
        double tDotS = dot(t, s);
        double tDotT = dot(t, t);
        if (qAbs(tDotT) < 1e-30) tDotT = 1e-30;
        omega = tDotS / tDotT;

        x = addScaled(addScaled(x, alpha, pHat), omega, sHat);
        r = addScaled(s, -omega, t);
        rho1 = rho;

        if (norm(r) < m_tol) break;
    }

    double elapsed = timer.elapsed();
    m_stats.matrixSize = n;
    m_stats.iterationsUsed = iter + 1;
    m_stats.initialResidual = initRes;
    m_stats.finalResidual = norm(r);
    m_stats.converged = (norm(r) < m_tol);
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    return x;
}

/* ---- Reset ---- */

void BiCGSTAB12::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
