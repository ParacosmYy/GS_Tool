/**
 * @file BiCGSTAB2.cpp
 * @brief BiCGSTAB2 实现
 *
 * 实现BiCGSTAB线性方程组求解器：GPBi-CG变体、多项式预处理、改进收敛性。
 */

#include "utils/matrix201/BiCGSTAB2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

BiCGSTAB2::BiCGSTAB2(QObject *parent) : QObject(parent) {}
BiCGSTAB2::~BiCGSTAB2() = default;

/* ---- Configuration ---- */

void BiCGSTAB2::setMaxIterations(int iter) { m_maxIter = qMax(10, iter); }
void BiCGSTAB2::setTolerance(double tol) { m_tolerance = qBound(1e-15, tol, 1.0); }
void BiCGSTAB2::setPreconditionDegree(int deg) { m_precondDeg = qBound(1, deg, 5); }

/* ---- Build CSR ---- */

void BiCGSTAB2::buildCSR(int n, const QVector<SparseEntry>& entries)
{
    m_n = n;

    // Sort by row, then by column
    auto sorted = entries;
    std::sort(sorted.begin(), sorted.end(),
              [](const SparseEntry& a, const SparseEntry& b) {
                  return (a.row != b.row) ? (a.row < b.row) : (a.col < b.col);
              });

    m_values.resize(sorted.size());
    m_colIdx.resize(sorted.size());
    m_rowPtr.resize(n + 1, 0);

    for (int i = 0; i < sorted.size(); ++i) {
        m_values[i] = sorted[i].value;
        m_colIdx[i] = sorted[i].col;
        m_rowPtr[sorted[i].row + 1]++;
    }
    for (int i = 0; i < n; ++i)
        m_rowPtr[i + 1] += m_rowPtr[i];
}

/* ---- Sparse matrix-vector multiply ---- */

QVector<double> BiCGSTAB2::spmv(const QVector<double>& x) const
{
    QVector<double> y(m_n, 0.0);
    for (int i = 0; i < m_n; ++i) {
        double sum = 0.0;
        for (int j = m_rowPtr[i]; j < m_rowPtr[i + 1]; ++j)
            sum += m_values[j] * x[m_colIdx[j]];
        y[i] = sum;
    }
    return y;
}

/* ---- Dot product ---- */

double BiCGSTAB2::dot(const QVector<double>& a, const QVector<double>& b)
{
    double s = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

/* ---- Polynomial preconditioner coefficients ---- */

QVector<double> BiCGSTAB2::precondCoeffs() const
{
    // Chebyshev polynomial coefficients for preconditioning
    QVector<double> coeffs(m_precondDeg + 1, 0.0);
    coeffs[0] = 1.0;
    if (m_precondDeg >= 1) coeffs[1] = 1.0;
    for (int k = 2; k <= m_precondDeg; ++k)
        coeffs[k] = 2.0 * coeffs[k - 1] - coeffs[k - 2];
    return coeffs;
}

/* ---- Apply preconditioner ---- */

QVector<double> BiCGSTAB2::precondition(const QVector<double>& r) const
{
    if (m_precondDeg <= 0) return r;

    auto coeffs = precondCoeffs();
    QVector<double> z(m_n, 0.0);
    for (int k = 0; k <= m_precondDeg && k < coeffs.size(); ++k) {
        double c = coeffs[k];
        for (int i = 0; i < m_n; ++i)
            z[i] += c * r[i];
    }

    // Jacobi-like diagonal scaling
    for (int i = 0; i < m_n; ++i) {
        double diag = 0.0;
        for (int j = m_rowPtr[i]; j < m_rowPtr[i + 1]; ++j)
            if (m_colIdx[j] == i) { diag = m_values[j]; break; }
        if (qAbs(diag) > 1e-15) z[i] /= diag;
    }
    return z;
}

/* ---- Residual norm ---- */

double BiCGSTAB2::residualNorm(const QVector<double>& r) const
{
    return qSqrt(dot(r, r));
}

/* ---- Solve ---- */

QVector<double> BiCGSTAB2::solve(int n, const QVector<SparseEntry>& entries,
                                   const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    if (n <= 0 || b.size() != n) return {};

    buildCSR(n, entries);

    // Initial guess x = 0
    QVector<double> x(n, 0.0);

    // r = b - A*x = b
    QVector<double> r = b;

    // r0~ = r (shadow residual)
    QVector<double> r0tilde = r;

    double rho0 = 1.0, alpha = 1.0, omega0 = 1.0;
    QVector<double> v(n, 0.0), p(n, 0.0);

    bool converged = false;
    int iter = 0;

    for (iter = 0; iter < m_maxIter; ++iter) {
        double rho1 = dot(r0tilde, r);
        if (qAbs(rho1) < 1e-30) break;

        double beta = (rho1 / rho0) * (alpha / omega0);

        // GPBi-CG variant: improved p update
        for (int i = 0; i < n; ++i)
            p[i] = r[i] + beta * (p[i] - omega0 * v[i]);

        // Apply preconditioner
        auto pHat = precondition(p);

        // v = A * p~
        v = spmv(pHat);

        alpha = rho1 / dot(r0tilde, v);

        // s = r - alpha * v
        QVector<double> s(n);
        for (int i = 0; i < n; ++i) s[i] = r[i] - alpha * v[i];

        // Early convergence check
        if (residualNorm(s) < m_tolerance) {
            for (int i = 0; i < n; ++i) x[i] += alpha * pHat[i];
            converged = true;
            break;
        }

        // Apply preconditioner to s
        auto sHat = precondition(s);

        // t = A * s~
        auto t = spmv(sHat);

        // omega = (t, s) / (t, t)
        omega0 = dot(t, s) / dot(t, t);
        if (qAbs(omega0) < 1e-30) omega0 = 1.0;

        // Update solution and residual
        for (int i = 0; i < n; ++i) {
            x[i] += alpha * pHat[i] + omega0 * sHat[i];
            r[i] = s[i] - omega0 * t[i];
        }

        rho0 = rho1;

        if (residualNorm(r) < m_tolerance) {
            converged = true;
            break;
        }
    }

    double finalRes = residualNorm(r);

    m_stats.totalSolves++;
    m_stats.matrixSize = n;
    m_stats.iterations = iter;
    m_stats.finalResidual = finalRes;
    m_stats.converged = converged;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(iter, finalRes, converged, timer.elapsed());
    return x;
}

/* ---- Solve dense ---- */

QVector<double> BiCGSTAB2::solveDense(const QVector<QVector<double>>& A,
                                        const QVector<double>& b)
{
    int n = A.size();
    if (n == 0) return {};

    QVector<SparseEntry> entries;
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < qMin(n, A[i].size()); ++j)
            if (qAbs(A[i][j]) > 1e-15)
                entries.append({i, j, A[i][j]});

    return solve(n, entries, b);
}

/* ---- Reset ---- */

void BiCGSTAB2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_n = 0;
    m_values.clear();
    m_colIdx.clear();
    m_rowPtr.clear();
}
