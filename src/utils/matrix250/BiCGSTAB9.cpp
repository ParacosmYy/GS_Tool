/**
 * @file BiCGSTAB9.cpp
 * @brief BiCGSTAB9 实现
 *
 * 实现双共轭梯度稳定法：右预条件与复合步稳定的改进收敛鲁棒性。
 */

#include "utils/matrix250/BiCGSTAB9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

BiCGSTAB9::BiCGSTAB9(QObject *parent) : QObject(parent) {}
BiCGSTAB9::~BiCGSTAB9() = default;

/* ---- Configuration ---- */

void BiCGSTAB9::setTolerance(double tol) { m_tol = qMax(1e-15, tol); }
void BiCGSTAB9::setMaxIterations(int iters) { m_maxIter = qMax(1, iters); }

/* ---- Sparse matrix-vector product ---- */

QVector<double> BiCGSTAB9::spMV(const QVector<double>& val,
                                 const QVector<int>& col,
                                 const QVector<int>& rowPtr,
                                 int n,
                                 const QVector<double>& x)
{
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int j = rowPtr[i]; j < rowPtr[i + 1]; ++j)
            y[i] += val[j] * x[col[j]];
    }
    return y;
}

/* ---- Jacobi preconditioner ---- */

QVector<double> BiCGSTAB9::jacobiPrecond(const QVector<double>& val,
                                          const QVector<int>& col,
                                          const QVector<int>& rowPtr,
                                          int n,
                                          const QVector<double>& r)
{
    // Extract diagonal and apply inverse
    QVector<double> diag(n, 1.0);
    for (int i = 0; i < n; ++i) {
        for (int j = rowPtr[i]; j < rowPtr[i + 1]; ++j) {
            if (col[j] == i) {
                diag[i] = (qAbs(val[j]) > 1e-15) ? 1.0 / val[j] : 1.0;
                break;
            }
        }
    }
    QVector<double> z(n);
    for (int i = 0; i < n; ++i) z[i] = diag[i] * r[i];
    return z;
}

/* ---- Dot product ---- */

double BiCGSTAB9::dot(const QVector<double>& a, const QVector<double>& b)
{
    double s = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

/* ---- Norm ---- */

double BiCGSTAB9::norm(const QVector<double>& v)
{
    return qSqrt(dot(v, v));
}

/* ---- Solve ---- */

QVector<double> BiCGSTAB9::solve(const QVector<double>& values,
                                  const QVector<int>& colIndices,
                                  const QVector<int>& rowPtr,
                                  int n,
                                  const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    m_history.clear();

    // Initial guess x0 = 0
    QVector<double> x(n, 0.0);

    // r0 = b - A*x0 = b
    QVector<double> r = b;
    double r0Norm = norm(r);
    m_stats.initialResidual = r0Norm;

    if (r0Norm < m_tol) {
        m_stats.converged = true;
        m_stats.iterations = 0;
        m_stats.finalResidual = r0Norm;
        m_stats.matrixSize = n;
        m_stats.totalOps++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
        return x;
    }

    // Choose r0_hat = r0
    QVector<double> r0hat = r;

    // p = r
    QVector<double> p = r;

    double rho = dot(r0hat, r);
    double rhoPrev = rho;
    double omega = 1.0;
    double alpha = 1.0;

    QVector<double> v(n, 0.0);
    QVector<double> s(n, 0.0);
    QVector<double> t(n, 0.0);

    bool converged = false;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        // Composite step stabilization: check for near-zero rho
        if (qAbs(rho) < 1e-30) break;

        // Right preconditioned BiCGSTAB:
        // Phase 1: y = M^{-1} p, v = A*y
        QVector<double> y = jacobiPrecond(values, colIndices, rowPtr, n, p);
        v = spMV(values, colIndices, rowPtr, n, y);

        // alpha = rho / (r0hat . v)
        double r0hatDotV = dot(r0hat, v);
        if (qAbs(r0hatDotV) < 1e-30) break;
        alpha = rho / r0hatDotV;

        // s = r - alpha * v
        for (int i = 0; i < n; ++i) s[i] = r[i] - alpha * v[i];

        // Phase 2: z = M^{-1} s, t = A*z
        QVector<double> z = jacobiPrecond(values, colIndices, rowPtr, n, s);
        t = spMV(values, colIndices, rowPtr, n, z);

        // omega = (t . s) / (t . t)
        double tDotT = dot(t, t);
        if (tDotT < 1e-30) { omega = 0.0; }
        else { omega = dot(t, s) / tDotT; }

        // Update solution: x = x + alpha*y + omega*z
        for (int i = 0; i < n; ++i)
            x[i] += alpha * y[i] + omega * z[i];

        // Update residual: r = s - omega * t
        for (int i = 0; i < n; ++i) r[i] = s[i] - omega * t[i];

        double rNorm = norm(r);
        IterationRecord rec;
        rec.iteration = iter + 1;
        rec.residualNorm = rNorm;
        m_history.append(rec);

        if (rNorm < m_tol * r0Norm) { converged = true; break; }

        // Update rho for next iteration
        rhoPrev = rho;
        rho = dot(r0hat, r);
        double beta = (rho / rhoPrev) * (alpha / omega);

        // Update p = r + beta * (p - omega * v)
        for (int i = 0; i < n; ++i)
            p[i] = r[i] + beta * (p[i] - omega * v[i]);
    }

    m_stats.matrixSize = n;
    m_stats.iterations = m_history.size();
    m_stats.finalResidual = m_history.isEmpty() ? r0Norm : m_history.last().residualNorm;
    m_stats.converged = converged;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit solveCompleted(m_stats.iterations, m_stats.finalResidual,
                        converged, timer.elapsed());
    return x;
}

/* ---- History ---- */

QVector<BiCGSTAB9::IterationRecord> BiCGSTAB9::history() const { return m_history; }

/* ---- Reset ---- */

void BiCGSTAB9::resetStatistics()
{
    m_history.clear();
    m_stats = Stats{}; m_timeSum = 0.0;
}
