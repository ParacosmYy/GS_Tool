/**
 * @file BiCGSTAB13.cpp
 * @brief BiCGSTAB13 实现
 *
 * 实现双共轭梯度稳定法：右预条件与残差平滑不定线性系统改进收敛。
 */

#include "utils/matrix278/BiCGSTAB13.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

BiCGSTAB13::BiCGSTAB13(QObject *parent)
    : QObject(parent) {}

BiCGSTAB13::~BiCGSTAB13() = default;

/* ---- Configuration ---- */

void BiCGSTAB13::setMaxIterations(int iters) { m_maxIter = qBound(1, iters, 100000); }
void BiCGSTAB13::setTolerance(double tol) { m_tol = qBound(1e-15, tol, 1.0); }

/* ---- Helpers ---- */

double BiCGSTAB13::dot(const QVector<double>& a, const QVector<double>& b) const
{
    double s = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

double BiCGSTAB13::norm(const QVector<double>& v) const
{
    return qSqrt(dot(v, v));
}

/* ---- Dense matrix-vector product ---- */

QVector<double> BiCGSTAB13::matVec(const QVector<QVector<double>>& A,
                                     const QVector<double>& x) const
{
    int n = A.size();
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < qMin(n, A[i].size()); ++j)
            y[i] += A[i][j] * x[j];
    return y;
}

/* ---- Sparse matrix-vector product (CSR) ---- */

QVector<double> BiCGSTAB13::sparseMatVec(int n,
                                           const QVector<int>& rowPtr,
                                           const QVector<int>& colIdx,
                                           const QVector<double>& values,
                                           const QVector<double>& x) const
{
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int j = rowPtr[i]; j < rowPtr[i + 1]; ++j) {
            if (colIdx[j] < x.size())
                y[i] += values[j] * x[colIdx[j]];
        }
    }
    return y;
}

/* ---- Jacobi preconditioner ---- */

QVector<double> BiCGSTAB13::precondition(const QVector<double>& r,
                                           const QVector<double>& diag) const
{
    int n = r.size();
    QVector<double> z(n, 0.0);
    for (int i = 0; i < n; ++i)
        z[i] = (qAbs(diag[i]) > 1e-15) ? r[i] / diag[i] : r[i];
    return z;
}

/* ---- Dense solve ---- */

QVector<double> BiCGSTAB13::solve(const QVector<QVector<double>>& A,
                                    const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    int n = b.size();
    if (n == 0) return {};

    // Extract diagonal for Jacobi preconditioner
    QVector<double> diag(n, 1.0);
    for (int i = 0; i < n; ++i) diag[i] = (i < A.size() && i < A[i].size()) ? A[i][i] : 1.0;

    // Initial guess x0 = 0
    QVector<double> x(n, 0.0);
    QVector<double> r = b;  // r0 = b - A*x0 = b
    QVector<double> r0hat = r;  // Shadow residual (choose r0hat = r0)

    double rho0 = 1.0, alpha = 1.0, omega0 = 1.0;
    QVector<double> v(n, 0.0), p(n, 0.0);

    double bNorm = norm(b);
    if (bNorm < 1e-30) bNorm = 1.0;

    m_residualHistory.clear();
    int iter = 0;

    // Smoothed solution and residual for residual smoothing
    QVector<double> xSmooth = x;
    double rSmoothNorm = norm(r);

    for (; iter < m_maxIter; ++iter) {
        double rho1 = dot(r0hat, r);
        if (qAbs(rho1) < 1e-30) break;

        double beta = (rho1 / rho0) * (alpha / omega0);

        // p = r + beta * (p - omega0 * v)
        for (int i = 0; i < n; ++i)
            p[i] = r[i] + beta * (p[i] - omega0 * v[i]);

        // Right preconditioning: solve M*y = p => y = M^{-1} p
        QVector<double> y = precondition(p, diag);

        // v = A * y
        v = matVec(A, y);

        alpha = rho1 / dot(r0hat, v);
        if (qAbs(alpha) > 1e30) alpha = 1e-10;

        // s = r - alpha * v
        QVector<double> s(n);
        for (int i = 0; i < n; ++i) s[i] = r[i] - alpha * v[i];

        // Right precondition s
        QVector<double> z = precondition(s, diag);

        // t = A * z
        QVector<double> t = matVec(A, z);

        omega0 = dot(t, s) / dot(t, t);
        if (qAbs(omega0) > 1e30) omega0 = 1e-10;

        // Update x
        for (int i = 0; i < n; ++i)
            x[i] += alpha * y[i] + omega0 * z[i];

        // Update residual
        for (int i = 0; i < n; ++i)
            r[i] = s[i] - omega0 * t[i];

        rho0 = rho1;

        double rNorm = norm(r);
        m_residualHistory.append(rNorm / bNorm);

        // Residual smoothing: keep best solution
        if (rNorm < rSmoothNorm) {
            xSmooth = x;
            rSmoothNorm = rNorm;
        }

        if (rNorm / bNorm < m_tol) break;
    }

    // Use smoothed solution if it's better
    if (rSmoothNorm < norm(r)) x = xSmooth;

    double elapsed = timer.elapsed();
    m_stats.matrixSize = n;
    m_stats.numIterations = iter;
    m_stats.residualNorm = norm(r) / bNorm;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit solveDone(iter, m_stats.residualNorm, elapsed);

    return x;
}

/* ---- Sparse solve ---- */

QVector<double> BiCGSTAB13::solveSparse(int n,
                                          const QVector<int>& rowPtr,
                                          const QVector<int>& colIdx,
                                          const QVector<double>& values,
                                          const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    if (n == 0) return {};

    // Extract diagonal
    QVector<double> diag(n, 1.0);
    for (int i = 0; i < n; ++i) {
        for (int j = rowPtr[i]; j < rowPtr[i + 1]; ++j) {
            if (colIdx[j] == i) { diag[i] = values[j]; break; }
        }
    }

    QVector<double> x(n, 0.0);
    QVector<double> r = b;
    QVector<double> r0hat = r;

    double rho0 = 1.0, alpha = 1.0, omega0 = 1.0;
    QVector<double> v(n, 0.0), p(n, 0.0);

    double bNorm = norm(b);
    if (bNorm < 1e-30) bNorm = 1.0;

    m_residualHistory.clear();
    QVector<double> xSmooth = x;
    double rSmoothNorm = norm(r);

    int iter = 0;
    for (; iter < m_maxIter; ++iter) {
        double rho1 = dot(r0hat, r);
        if (qAbs(rho1) < 1e-30) break;
        double beta = (rho1 / rho0) * (alpha / omega0);

        for (int i = 0; i < n; ++i)
            p[i] = r[i] + beta * (p[i] - omega0 * v[i]);

        QVector<double> y = precondition(p, diag);
        v = sparseMatVec(n, rowPtr, colIdx, values, y);

        alpha = rho1 / dot(r0hat, v);
        if (qAbs(alpha) > 1e30) alpha = 1e-10;

        QVector<double> s(n);
        for (int i = 0; i < n; ++i) s[i] = r[i] - alpha * v[i];

        QVector<double> z = precondition(s, diag);
        QVector<double> t = sparseMatVec(n, rowPtr, colIdx, values, z);

        omega0 = dot(t, s) / dot(t, t);
        if (qAbs(omega0) > 1e30) omega0 = 1e-10;

        for (int i = 0; i < n; ++i)
            x[i] += alpha * y[i] + omega0 * z[i];
        for (int i = 0; i < n; ++i)
            r[i] = s[i] - omega0 * t[i];

        rho0 = rho1;
        double rNorm = norm(r);
        m_residualHistory.append(rNorm / bNorm);

        if (rNorm < rSmoothNorm) { xSmooth = x; rSmoothNorm = rNorm; }
        if (rNorm / bNorm < m_tol) break;
    }

    if (rSmoothNorm < norm(r)) x = xSmooth;

    double elapsed = timer.elapsed();
    m_stats.matrixSize = n;
    m_stats.numIterations = iter;
    m_stats.residualNorm = norm(r) / bNorm;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit solveDone(iter, m_stats.residualNorm, elapsed);

    return x;
}

/* ---- Accessors ---- */

QVector<double> BiCGSTAB13::residualHistory() const { return m_residualHistory; }

/* ---- Reset ---- */

void BiCGSTAB13::resetStatistics()
{
    m_residualHistory.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
