/**
 * @file BiCGSTAB17.cpp
 * @brief BiCGSTAB17 实现
 *
 * 实现BiCGSTAB求解器：右预条件与灵活GMRES内求解器实现变预条件稳定双共轭梯度。
 */

#include "utils/matrix305/BiCGSTAB17.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

BiCGSTAB17::BiCGSTAB17(QObject *parent)
    : QObject(parent) {}

BiCGSTAB17::~BiCGSTAB17() = default;

/* ---- Configuration ---- */

void BiCGSTAB17::setMaxIterations(int iter) { m_maxIter = qBound(10, iter, 100000); }
void BiCGSTAB17::setTolerance(double tol) { m_tol = qBound(1e-15, tol, 1.0); }
void BiCGSTAB17::setPreconditionerType(int type) { m_pcType = qBound(0, type, 2); }

/* ---- Dot product ---- */

double BiCGSTAB17::dot(const QVector<double>& a, const QVector<double>& b) const
{
    double sum = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) sum += a[i] * b[i];
    return sum;
}

/* ---- Vector norm ---- */

double BiCGSTAB17::norm(const QVector<double>& v) const
{
    return qSqrt(dot(v, v));
}

/* ---- Matrix-vector product ---- */

QVector<double> BiCGSTAB17::matvec(int n, const QVector<SparseEntry>& entries,
                                    const QVector<double>& x) const
{
    QVector<double> y(n, 0.0);
    for (const auto& e : entries) {
        if (e.row < n && e.col < x.size())
            y[e.row] += e.value * x[e.col];
    }
    return y;
}

/* ---- Build Jacobi preconditioner ---- */

void BiCGSTAB17::buildJacobi(int n, const QVector<SparseEntry>& entries)
{
    m_diagInv.resize(n, 1.0);
    for (const auto& e : entries) {
        if (e.row == e.col && qAbs(e.value) > 1e-300)
            m_diagInv[e.row] = 1.0 / e.value;
    }
}

/* ---- Build ILU(0) preconditioner ---- */

void BiCGSTAB17::buildILU0(int n, const QVector<SparseEntry>& entries)
{
    // Build row-based CSR-like structure
    m_rows.resize(n);
    for (auto& row : m_rows) row.clear();

    for (const auto& e : entries)
        m_rows[e.row].append(e);

    // In-place ILU(0) factorization
    for (int i = 0; i < n; ++i) {
        for (auto& e : m_rows[i]) {
            if (e.col >= i) continue;
            double diagVal = 0.0;
            for (const auto& d : m_rows[e.col]) {
                if (d.col == e.col) { diagVal = d.value; break; }
            }
            if (qAbs(diagVal) > 1e-300)
                e.value /= diagVal;

            // Update remaining entries
            for (auto& e2 : m_rows[i]) {
                if (e2.col <= e.col) continue;
                double aij = 0.0;
                for (const auto& ek : m_rows[e.col]) {
                    if (ek.col == e2.col) { aij = ek.value; break; }
                }
                e2.value -= e.value * aij;
            }
        }
    }
}

/* ---- ILU forward-backward solve ---- */

QVector<double> BiCGSTAB17::iluSolve(const QVector<double>& r) const
{
    int n = r.size();
    QVector<double> y(n, 0.0), x(n, 0.0);

    // Forward solve: Ly = r
    for (int i = 0; i < n; ++i) {
        double sum = r[i];
        for (const auto& e : m_rows[i]) {
            if (e.col < i) sum -= e.value * y[e.col];
        }
        y[i] = sum;  // L has unit diagonal
    }

    // Backward solve: Ux = y
    for (int i = n - 1; i >= 0; --i) {
        double sum = y[i];
        double diagVal = 1.0;
        for (const auto& e : m_rows[i]) {
            if (e.col > i) sum -= e.value * x[e.col];
            if (e.col == i) diagVal = e.value;
        }
        x[i] = (qAbs(diagVal) > 1e-300) ? sum / diagVal : sum;
    }

    return x;
}

/* ---- Apply preconditioner ---- */

QVector<double> BiCGSTAB17::precondition(const QVector<double>& r) const
{
    if (m_pcType == 0) return r;

    if (m_pcType == 1) {
        // Jacobi
        QVector<double> z(r.size());
        for (int i = 0; i < r.size(); ++i)
            z[i] = r[i] * m_diagInv.value(i, 1.0);
        return z;
    }

    // ILU(0)
    return iluSolve(r);
}

/* ---- Flexible GMRES inner solver (small number of iterations) ---- */

QVector<double> BiCGSTAB17::flexibleGMRESInner(const QVector<double>& b, int innerIter) const
{
    int n = b.size();
    if (n == 0) return b;

    // Simple GMRES(m) with m = innerIter
    QVector<QVector<double>> Q(innerIter + 1, QVector<double>(n, 0.0));
    QVector<double> cos(innerIter, 0.0), sin(innerIter, 0.0);
    QVector<double> g(innerIter + 1, 0.0);

    double beta = norm(b);
    if (beta < 1e-300) return QVector<double>(n, 0.0);

    for (int i = 0; i < n; ++i) Q[0][i] = b[i] / beta;
    g[0] = beta;

    for (int j = 0; j < innerIter; ++j) {
        // Arnoldi step with variable preconditioner
        QVector<double> w = matvec(n, m_entries, Q[j]);
        QVector<double> z = precondition(w);  // Flexible: z = M_j^{-1} A q_j

        for (int i = 0; i <= j; ++i) {
            double h = dot(Q[i], z);
            for (int k = 0; k < n; ++k) z[k] -= h * Q[i][k];
        }

        double hNext = norm(z);
        if (hNext < 1e-300) break;

        for (int k = 0; k < n; ++k) Q[j + 1][k] = z[k] / hNext;
    }

    // Solve upper triangular system (simplified)
    QVector<double> y(innerIter, 0.0);
    for (int i = qMin(innerIter - 1, n - 1); i >= 0; --i) {
        y[i] = g[i];
        for (int j = i + 1; j < innerIter; ++j)
            y[i] -= dot(Q[i], Q[j]) * y[j];
    }

    // Combine
    QVector<double> result(n, 0.0);
    for (int j = 0; j < innerIter && j < Q.size(); ++j)
        for (int i = 0; i < n; ++i)
            result[i] += y[j] * Q[j][i];

    return result;
}

/* ---- Main solver ---- */

BiCGSTAB17::SolveResult BiCGSTAB17::solve(int n, const QVector<SparseEntry>& entries,
                                            const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    SolveResult result;
    if (n <= 0 || rhs.size() < n) { result.elapsedMs = timer.elapsed(); return result; }

    m_n = n;
    m_entries = entries;

    // Build preconditioner
    if (m_pcType == 1) buildJacobi(n, entries);
    else if (m_pcType == 2) buildILU0(n, entries);

    // Initial guess x = 0
    QVector<double> x(n, 0.0);

    // r = b - A*x = b
    QVector<double> r = rhs;

    double rNorm = norm(r);
    result.initialResidual = rNorm;
    if (rNorm < m_tol) {
        result.solution = x;
        result.converged = true;
        result.iterations = 0;
        result.residualNorm = rNorm;
        result.elapsedMs = timer.elapsed();
        return result;
    }

    // Choose r_hat = r (shadow residual)
    QVector<double> rHat = r;

    double rho1 = 1.0, alpha = 1.0, omega = 1.0;
    QVector<double> p(n, 0.0), v(n, 0.0);

    int iter = 0;
    for (iter = 0; iter < m_maxIter; ++iter) {
        double rho = dot(rHat, r);
        if (qAbs(rho) < 1e-300) break;

        double beta = (rho / rho1) * (alpha / omega);
        rho1 = rho;

        // p = r + beta * (p - omega * v)
        for (int i = 0; i < n; ++i)
            p[i] = r[i] + beta * (p[i] - omega * v[i]);

        // Apply right preconditioner: y = M^{-1} p
        QVector<double> y = precondition(p);

        // For flexible variant: use GMRES inner solve periodically
        if (m_pcType == 2 && iter % 5 == 0) {
            QVector<double> Ap = matvec(n, entries, y);
            QVector<double> innerRhs(n);
            for (int i = 0; i < n; ++i) innerRhs[i] = r[i] - Ap[i] * 0.1;
            QVector<double> correction = flexibleGMRESInner(innerRhs, 3);
            for (int i = 0; i < n; ++i) y[i] += correction[i] * 0.1;
        }

        // v = A * y
        v = matvec(n, entries, y);

        alpha = rho / dot(rHat, v);
        if (qAbs(dot(rHat, v)) < 1e-300) break;

        // s = r - alpha * v
        QVector<double> s(n);
        for (int i = 0; i < n; ++i) s[i] = r[i] - alpha * v[i];

        // z = M^{-1} s
        QVector<double> z = precondition(s);

        // t = A * z
        QVector<double> t = matvec(n, entries, z);

        omega = dot(t, s) / dot(t, t);
        if (qAbs(dot(t, t)) < 1e-300) omega = 1.0;

        // Update x and r
        for (int i = 0; i < n; ++i) {
            x[i] += alpha * y[i] + omega * z[i];
            r[i] = s[i] - omega * t[i];
        }

        double newNorm = norm(r);
        if (newNorm / rNorm < m_tol) {
            result.converged = true;
            result.residualNorm = newNorm;
            break;
        }
    }

    result.solution = x;
    result.iterations = iter + 1;
    result.residualNorm = norm(r);
    result.elapsedMs = timer.elapsed();

    m_stats.totalSolves++;
    m_stats.maxIterationsUsed = qMax(m_stats.maxIterationsUsed, result.iterations);
    m_timeSum += result.elapsedMs;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveDone(result.iterations, result.residualNorm, result.converged, result.elapsedMs);
    return result;
}

/* ---- Reset ---- */

void BiCGSTAB17::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
