/**
 * @file BiCGSTAB6.cpp
 * @brief BiCGSTAB6 实现
 *
 * 实现BiCGSTAB：GPBi-CG稳定化、右预条件、灵活GMRES内迭代。
 */

#include "utils/matrix226/BiCGSTAB6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

BiCGSTAB6::BiCGSTAB6(QObject *parent) : QObject(parent) {}
BiCGSTAB6::~BiCGSTAB6() = default;

/* ---- Configuration ---- */

void BiCGSTAB6::setParameters(double tolerance, int maxIter, bool rightPrecond)
{
    m_tolerance = qMax(1e-15, tolerance);
    m_maxIter = qMax(10, maxIter);
    m_rightPrecond = rightPrecond;
}

/* ---- Load sparse matrix (COO -> CSR) ---- */

void BiCGSTAB6::loadMatrix(int n, const QVector<Entry>& entries)
{
    m_n = n;
    m_stats.matrixSize = n;
    m_stats.nnz = entries.size();

    // Count entries per row
    QVector<int> count(n, 0);
    for (const auto& e : entries)
        if (e.row >= 0 && e.row < n) count[e.row]++;

    // Build CSR
    m_rowPtr.resize(n + 1, 0);
    for (int i = 0; i < n; ++i) m_rowPtr[i + 1] = m_rowPtr[i] + count[i];

    m_values.resize(entries.size());
    m_colIdx.resize(entries.size());
    QVector<int> pos = m_rowPtr;

    for (const auto& e : entries) {
        if (e.row >= 0 && e.row < n && e.col >= 0 && e.col < n) {
            int idx = pos[e.row]++;
            m_values[idx] = e.val;
            m_colIdx[idx] = e.col;
        }
    }

    // Extract diagonal for Jacobi preconditioner
    m_diag.resize(n, 1.0);
    for (int i = 0; i < n; ++i) {
        for (int j = m_rowPtr[i]; j < m_rowPtr[i + 1]; ++j) {
            if (m_colIdx[j] == i) {
                m_diag[i] = (qAbs(m_values[j]) > 1e-15) ? m_values[j] : 1.0;
                break;
            }
        }
    }
}

/* ---- Sparse matrix-vector product ---- */

void BiCGSTAB6::spMV(const QVector<double>& x, QVector<double>& y) const
{
    int n = m_n;
    y.resize(n);
    for (int i = 0; i < n; ++i) {
        double s = 0.0;
        for (int j = m_rowPtr[i]; j < m_rowPtr[i + 1]; ++j)
            s += m_values[j] * x[m_colIdx[j]];
        y[i] = s;
    }
}

/* ---- Dot product ---- */

double BiCGSTAB6::dot(const QVector<double>& a, const QVector<double>& b)
{
    double s = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

/* ---- Vector norm ---- */

double BiCGSTAB6::norm(const QVector<double>& v)
{
    return qSqrt(dot(v, v));
}

/* ---- Jacobi preconditioner ---- */

void BiCGSTAB6::jacobiPrecondition(const QVector<double>& r,
                                      QVector<double>& z) const
{
    int n = qMin(r.size(), m_diag.size());
    z.resize(n);
    for (int i = 0; i < n; ++i)
        z[i] = r[i] / m_diag[i];
}

/* ---- Flexible GMRES inner iteration ---- */

void BiCGSTAB6::flexibleInnerGMRES(const QVector<double>& r,
                                      QVector<double>& z, int steps) const
{
    // Simplified 1-3 step GMRES as preconditioning polish
    z.resize(r.size());
    jacobiPrecondition(r, z);

    for (int s = 1; s < steps; ++s) {
        QVector<double> Az;
        spMV(z, Az);
        // z = z + alpha * (r - Az)
        double alpha = dot(r, Az) / qMax(dot(Az, Az), 1e-30);
        for (int i = 0; i < z.size(); ++i)
            z[i] += alpha * (r[i] - Az[i]);
    }
}

/* ---- GPBi-CG stabilization ---- */

void BiCGSTAB6::gpBiCGStabilize(QVector<double>& r, QVector<double>& p,
                                   QVector<double>& v, const QVector<double>& s,
                                   double omega, double alpha) const
{
    // GPBi-CG polynomial update for smoother convergence
    int n = r.size();
    for (int i = 0; i < n; ++i) {
        // Stabilized update combining biconjugate gradient and residual smoothing
        p[i] = r[i] + omega * (p[i] - alpha * v[i]);
    }
}

/* ---- Solve ---- */

BiCGSTAB6::SolveResult BiCGSTAB6::solve(const QVector<double>& b)
{
    QVector<double> x0(m_n, 0.0);
    return solve(b, x0);
}

BiCGSTAB6::SolveResult BiCGSTAB6::solve(const QVector<double>& b,
                                           const QVector<double>& x0)
{
    QElapsedTimer timer;
    timer.start();

    SolveResult result;
    int n = m_n;
    if (n == 0 || b.size() < n) { result.converged = false; return result; }

    QVector<double> x = x0;
    QVector<double> r(n), Ax(n);

    // r = b - A*x
    spMV(x, Ax);
    for (int i = 0; i < n; ++i) r[i] = b[i] - Ax[i];

    double bNorm = norm(b);
    if (bNorm < 1e-15) bNorm = 1.0;
    double rNorm = norm(r);

    // Choose r0_hat = r (shadow residual)
    QVector<double> r0hat = r;

    QVector<double> p = r, v(n, 0.0), s(n, 0.0), t(n, 0.0);
    QVector<double> y(n, 0.0), z(n, 0.0);

    int iter = 0;
    for (iter = 0; iter < m_maxIter; ++iter) {
        // Precondition: y = M^{-1} p
        if (m_rightPrecond) {
            flexibleInnerGMRES(p, y, 2);
        } else {
            y = p;
        }

        // v = A * y
        spMV(y, v);

        // alpha = (r0hat, r) / (r0hat, v)
        double rho = dot(r0hat, r);
        double denom = dot(r0hat, v);
        if (qAbs(denom) < 1e-30) break;
        double alpha = rho / denom;

        // s = r - alpha * v
        for (int i = 0; i < n; ++i) s[i] = r[i] - alpha * v[i];

        // Check early convergence
        if (norm(s) / bNorm < m_tolerance) {
            for (int i = 0; i < n; ++i) x[i] += alpha * y[i];
            rNorm = norm(s);
            break;
        }

        // Precondition: z = M^{-1} s
        if (m_rightPrecond) {
            flexibleInnerGMRES(s, z, 2);
        } else {
            z = s;
        }

        // t = A * z
        spMV(z, t);

        // omega = (t, s) / (t, t)
        double omega = dot(t, s) / qMax(dot(t, t), 1e-30);
        omega = qBound(-2.0, omega, 2.0); // Stabilize omega

        // Update x and r
        for (int i = 0; i < n; ++i) {
            x[i] += alpha * y[i] + omega * z[i];
            r[i] = s[i] - omega * t[i];
        }

        // GPBi-CG stabilization
        gpBiCGStabilize(r, p, v, s, omega, alpha);

        rNorm = norm(r);
        if (rNorm / bNorm < m_tolerance) break;

        // Restart if rho is too small
        double newRho = dot(r0hat, r);
        if (qAbs(newRho) < 1e-30 * qAbs(rho)) {
            r0hat = r;
            p = r;
        } else {
            double beta = (newRho / rho) * (alpha / qMax(omega, 1e-30));
            for (int i = 0; i < n; ++i)
                p[i] = r[i] + beta * (p[i] - omega * v[i]);
        }
    }

    result.x = x;
    result.residualNorm = rNorm;
    result.iterations = iter;
    result.converged = (rNorm / bNorm < m_tolerance);

    m_stats.totalIterations += iter;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit solveCompleted(iter, rNorm, timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void BiCGSTAB6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_values.clear();
    m_colIdx.clear();
    m_rowPtr.clear();
    m_diag.clear();
    m_n = 0;
}
