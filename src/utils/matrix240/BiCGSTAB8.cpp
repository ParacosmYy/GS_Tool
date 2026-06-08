/**
 * @file BiCGSTAB8.cpp
 * @brief BiCGSTAB8 实现
 *
 * 实现BiCGSTAB：多项式预处理与前瞻停滞恢复不定系统。
 */

#include "utils/matrix240/BiCGSTAB8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

BiCGSTAB8::BiCGSTAB8(QObject *parent) : QObject(parent) {}
BiCGSTAB8::~BiCGSTAB8() = default;

/* ---- Configuration ---- */

void BiCGSTAB8::setMaxIterations(int iter) { m_maxIter = qMax(10, iter); }
void BiCGSTAB8::setTolerance(double tol) { m_tol = qMax(1e-15, tol); }
void BiCGSTAB8::setPreconditionerDegree(int degree) { m_precondDegree = qBound(0, degree, 3); }
void BiCGSTAB8::setStagnationThreshold(double threshold) { m_stagnationThresh = qMax(1e-16, threshold); }

/* ---- Vector operations ---- */

double BiCGSTAB8::dot(const QVector<double>& a, const QVector<double>& b)
{
    double s = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

double BiCGSTAB8::norm(const QVector<double>& v)
{
    return qSqrt(qMax(0.0, dot(v, v)));
}

QVector<double> BiCGSTAB8::scale(double s, const QVector<double>& v)
{
    QVector<double> r(v.size());
    for (int i = 0; i < v.size(); ++i) r[i] = s * v[i];
    return r;
}

QVector<double> BiCGSTAB8::add(const QVector<double>& a, const QVector<double>& b)
{
    int n = qMin(a.size(), b.size());
    QVector<double> r(n);
    for (int i = 0; i < n; ++i) r[i] = a[i] + b[i];
    return r;
}

QVector<double> BiCGSTAB8::sub(const QVector<double>& a, const QVector<double>& b)
{
    int n = qMin(a.size(), b.size());
    QVector<double> r(n);
    for (int i = 0; i < n; ++i) r[i] = a[i] - b[i];
    return r;
}

/* ---- Matrix-vector multiply ---- */

QVector<double> BiCGSTAB8::matVecMul(const QVector<QVector<double>>& A,
                                       const QVector<double>& x) const
{
    int n = A.size();
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            y[i] += A[i][j] * x[j];
    return y;
}

/* ---- Jacobi diagonal ---- */

QVector<double> BiCGSTAB8::jacobiDiag(const QVector<QVector<double>>& A) const
{
    int n = A.size();
    QVector<double> diag(n);
    for (int i = 0; i < n; ++i)
        diag[i] = (qAbs(A[i][i]) > 1e-15) ? 1.0 / A[i][i] : 1.0;
    return diag;
}

/* ---- Polynomial preconditioner ---- */

QVector<double> BiCGSTAB8::applyPreconditioner(const QVector<QVector<double>>& A,
                                                 const QVector<double>& r) const
{
    int n = r.size();
    if (m_precondDegree == 0) return r;  // no preconditioning

    // Jacobi (diagonal) preconditioner as base
    QVector<double> diag = jacobiDiag(A);

    if (m_precondDegree == 1) {
        // M^{-1} r = diag^{-1} * r
        QVector<double> z(n);
        for (int i = 0; i < n; ++i) z[i] = diag[i] * r[i];
        return z;
    }

    // Polynomial preconditioning: z = (I + c1*M^{-1}*A + c2*(M^{-1}*A)^2 + ...) * M^{-1} * r
    QVector<double> z0(n);
    for (int i = 0; i < n; ++i) z0[i] = diag[i] * r[i];

    QVector<double> z = z0;

    if (m_precondDegree >= 2) {
        // z += M^{-1} * A * z0
        QVector<double> Az = matVecMul(A, z0);
        for (int i = 0; i < n; ++i) z[i] += diag[i] * Az[i] * 0.5;
    }

    if (m_precondDegree >= 3) {
        QVector<double> Az = matVecMul(A, z);
        for (int i = 0; i < n; ++i) z[i] += diag[i] * Az[i] * 0.25;
    }

    return z;
}

/* ---- Solve with initial guess ---- */

BiCGSTAB8::SolveResult BiCGSTAB8::solveWithGuess(const QVector<QVector<double>>& A,
                                                    const QVector<double>& b,
                                                    const QVector<double>& x0)
{
    QElapsedTimer timer;
    timer.start();
    SolveResult result;

    int n = b.size();
    if (n == 0 || A.size() != n) {
        result.converged = false;
        return result;
    }

    m_stats.matrixSize = n;

    // r0 = b - A*x0
    QVector<double> r = sub(b, matVecMul(A, x0));
    double bNorm = norm(b);
    if (bNorm < 1e-15) bNorm = 1.0;

    // Choose r0_hat = r
    QVector<double> rHat = r;

    QVector<double> x = x0;
    QVector<double> p = r;
    double rho = dot(rHat, r);

    double prevRes = norm(r) / bNorm;
    int stagnationCount = 0;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        // Apply preconditioner to p
        QVector<double> pHat = applyPreconditioner(A, p);

        // v = A * p_hat
        QVector<double> v = matVecMul(A, pHat);

        // alpha = rho / (r_hat . v)
        double rHatDotV = dot(rHat, v);
        if (qAbs(rHatDotV) < 1e-30) {
            // Look-ahead: stagnation recovery
            stagnationCount++;
            // Perturb rHat slightly
            for (int i = 0; i < n; ++i)
                rHat[i] += 1e-8 * (static_cast<double>(qrand()) / RAND_MAX - 0.5);
            rHatDotV = dot(rHat, v);
            if (qAbs(rHatDotV) < 1e-30) break;
        }

        double alpha = rho / rHatDotV;

        // s = r - alpha * v
        QVector<double> s = sub(r, scale(alpha, v));

        // Check for early convergence
        if (norm(s) / bNorm < m_tol) {
            x = add(x, scale(alpha, pHat));
            result.converged = true;
            result.iterations = iter + 1;
            result.solution = x;
            result.residualNorm = norm(s);
            result.stagnationRecoveries = stagnationCount;
            break;
        }

        // Apply preconditioner to s
        QVector<double> sHat = applyPreconditioner(A, s);

        // t = A * s_hat
        QVector<double> t = matVecMul(A, sHat);

        // omega = (t . s) / (t . t)
        double omega = dot(t, s) / (dot(t, t) + 1e-30);

        // x = x + alpha * p_hat + omega * s_hat
        x = add(add(x, scale(alpha, pHat)), scale(omega, sHat));

        // r = s - omega * t
        r = sub(s, scale(omega, t));

        double resNorm = norm(r) / bNorm;
        result.residualNorm = resNorm;

        // Stagnation detection
        if (qAbs(prevRes - resNorm) < m_stagnationThresh) {
            stagnationCount++;
            // Recovery: restart with current residual
            rHat = r;
            p = r;
            rho = dot(rHat, r);
            prevRes = resNorm;
            continue;
        }
        prevRes = resNorm;

        // Check convergence
        if (resNorm < m_tol) {
            result.converged = true;
            result.iterations = iter + 1;
            result.solution = x;
            result.stagnationRecoveries = stagnationCount;
            break;
        }

        // New rho
        double rhoNew = dot(rHat, r);
        if (qAbs(rhoNew) < 1e-30) break;

        // beta = (rhoNew / rho) * (alpha / omega)
        double beta = (rhoNew / rho) * (alpha / (omega + 1e-30));

        // p = r + beta * (p - omega * v)
        p = add(r, scale(beta, sub(p, scale(omega, v))));
        rho = rhoNew;

        result.iterations = iter + 1;
        emit iterationCompleted(iter + 1, resNorm);
    }

    if (!result.converged) {
        result.solution = x;
    }

    m_stats.totalIterations += result.iterations;
    m_stats.totalSolves++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
    m_stats.avgResidualNorm = (m_stats.avgResidualNorm * (m_stats.totalSolves - 1) + result.residualNorm) / m_stats.totalSolves;

    emit solveCompleted(result.iterations, result.residualNorm, elapsed);
    return result;
}

/* ---- Solve ---- */

BiCGSTAB8::SolveResult BiCGSTAB8::solve(const QVector<QVector<double>>& A,
                                          const QVector<double>& b)
{
    QVector<double> x0(b.size(), 0.0);
    return solveWithGuess(A, b, x0);
}

/* ---- Reset ---- */

void BiCGSTAB8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
