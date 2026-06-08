/**
 * @file BiCGSTAB5.cpp
 * @brief BiCGSTAB5 实现
 *
 * 实现稳定双共轭梯度法：稀疏矩阵向量乘、多项式预处理、复合步稳定化。
 */

#include "utils/matrix222/BiCGSTAB5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

BiCGSTAB5::BiCGSTAB5(QObject *parent) : QObject(parent) {}
BiCGSTAB5::~BiCGSTAB5() = default;

/* ---- Configuration ---- */

void BiCGSTAB5::setParameters(int maxIter, double tolerance,
                                int precondDegree)
{
    m_maxIter = qMax(10, maxIter);
    m_tol = qMax(1e-15, tolerance);
    m_precondDegree = qMax(1, precondDegree);
}

/* ---- Sparse matrix-vector product ---- */

QVector<double> BiCGSTAB5::spmv(const QVector<double>& values,
                                  const QVector<int>& colIdx,
                                  const QVector<int>& rowPtr,
                                  const QVector<double>& x) const
{
    int n = rowPtr.size() - 1;
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int j = rowPtr[i]; j < rowPtr[i + 1]; ++j)
            sum += values[j] * x[colIdx[j]];
        y[i] = sum;
    }
    return y;
}

/* ---- Dot product ---- */

double BiCGSTAB5::dot(const QVector<double>& a,
                       const QVector<double>& b) const
{
    double s = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

/* ---- Estimate preconditioner coefficients ---- */

void BiCGSTAB5::estimatePrecondCoeffs(const QVector<double>& values,
                                        const QVector<int>& colIdx,
                                        const QVector<int>& rowPtr)
{
    // Polynomial preconditioner coefficients estimated from diagonal
    // No-op: use Jacobi-like diagonal in polyPrecondition
    Q_UNUSED(values);
    Q_UNUSED(colIdx);
    Q_UNUSED(rowPtr);
}

/* ---- Polynomial preconditioner ---- */

QVector<double> BiCGSTAB5::polyPrecondition(
    const QVector<double>& values,
    const QVector<int>& colIdx,
    const QVector<int>& rowPtr,
    const QVector<double>& r) const
{
    // Degree-d polynomial preconditioner: M^{-1} = I + (I - D^{-1}A) + ...
    // Using Jacobi (diagonal) preconditioner as base
    int n = r.size();
    QVector<double> d(n, 0.0);

    // Extract diagonal
    for (int i = 0; i < n; ++i) {
        for (int j = rowPtr[i]; j < rowPtr[i + 1]; ++j) {
            if (colIdx[j] == i) {
                d[i] = qMax(qAbs(values[j]), 1e-12);
                break;
            }
        }
    }

    // Apply polynomial preconditioner: z = sum_{k=0}^{degree} (I - D^{-1}A)^k * D^{-1} * r
    QVector<double> z(n, 0.0);
    QVector<double> dk = r;

    for (int k = 0; k <= m_precondDegree; ++k) {
        for (int i = 0; i < n; ++i) z[i] += dk[i] / d[i];
        if (k < m_precondDegree) {
            // dk = (I - D^{-1}A) * dk
            QVector<double> Adk = spmv(values, colIdx, rowPtr, dk);
            for (int i = 0; i < n; ++i)
                dk[i] = dk[i] - Adk[i] / d[i];
        }
    }

    return z;
}

/* ---- Solve ---- */

QVector<double> BiCGSTAB5::solve(const QVector<double>& values,
                                   const QVector<int>& colIdx,
                                   const QVector<int>& rowPtr,
                                   const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    int n = rhs.size();
    QVector<double> x(n, 0.0);

    // r0 = b - Ax
    QVector<double> r = spmv(values, colIdx, rowPtr, x);
    for (int i = 0; i < n; ++i) r[i] = rhs[i] - r[i];

    double r0Norm = qSqrt(dot(r, r));
    if (r0Norm < m_tol) {
        m_stats.converged = true;
        m_stats.iterations = 0;
        m_stats.finalResidual = r0Norm;
        m_stats.initialResidual = r0Norm;
        return x;
    }

    // Choose r0_hat = r
    QVector<double> rHat = r;
    QVector<double> p = r;

    double rho = dot(rHat, r);
    double initRes = r0Norm;

    bool converged = false;
    int iter = 0;

    for (iter = 0; iter < m_maxIter; ++iter) {
        // Apply preconditioner
        QVector<double> pHat = polyPrecondition(values, colIdx, rowPtr, p);

        // v = A * p_hat
        QVector<double> v = spmv(values, colIdx, rowPtr, pHat);
        double alpha = rho / dot(rHat, v);

        // s = r - alpha * v
        QVector<double> s(n);
        for (int i = 0; i < n; ++i) s[i] = r[i] - alpha * v[i];

        // Check early convergence
        double sNorm = qSqrt(dot(s, s));
        if (sNorm < m_tol) {
            for (int i = 0; i < n; ++i) x[i] += alpha * pHat[i];
            converged = true;
            break;
        }

        // Apply preconditioner to s
        QVector<double> sHat = polyPrecondition(values, colIdx, rowPtr, s);

        // t = A * s_hat
        QVector<double> t = spmv(values, colIdx, rowPtr, sHat);
        double omega = dot(t, s) / dot(t, t);

        // Composite step stabilization: if omega is unstable, use alternative
        if (qAbs(omega) < 1e-30) omega = 1e-10;

        // x = x + alpha * p_hat + omega * s_hat
        for (int i = 0; i < n; ++i)
            x[i] += alpha * pHat[i] + omega * sHat[i];

        // r = s - omega * t
        for (int i = 0; i < n; ++i) r[i] = s[i] - omega * t[i];

        double rNorm = qSqrt(dot(r, r));
        if (rNorm / initRes < m_tol) { converged = true; iter++; break; }

        // Update rho and p
        double rhoNew = dot(rHat, r);
        if (qAbs(rhoNew) < 1e-30) break; // Breakdown

        double beta = (rhoNew / rho) * (alpha / omega);
        for (int i = 0; i < n; ++i)
            p[i] = r[i] + beta * (p[i] - omega * v[i]);

        rho = rhoNew;
        iter++;
    }

    double finalRes = qSqrt(dot(r, r));
    m_stats.totalSolves++;
    m_stats.matrixSize = n;
    m_stats.iterations = iter;
    m_stats.finalResidual = finalRes;
    m_stats.initialResidual = initRes;
    m_stats.converged = converged;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(iter, finalRes, converged, timer.elapsed());
    return x;
}

/* ---- Residual norm ---- */

double BiCGSTAB5::residualNorm(const QVector<double>& values,
                                 const QVector<int>& colIdx,
                                 const QVector<int>& rowPtr,
                                 const QVector<double>& x,
                                 const QVector<double>& rhs) const
{
    QVector<double> ax = spmv(values, colIdx, rowPtr, x);
    double sum = 0.0;
    for (int i = 0; i < rhs.size(); ++i) {
        double d = ax[i] - rhs[i];
        sum += d * d;
    }
    return qSqrt(sum);
}

/* ---- Reset ---- */

void BiCGSTAB5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
