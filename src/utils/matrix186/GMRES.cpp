/**
 * @file GMRES.cpp
 * @brief GMRES 实现
 *
 * 实现GMRES迭代求解器：Arnoldi正交化、重启GMRES(m)、柔性预处理。
 */

#include "utils/matrix186/GMRES.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

GMRES::GMRES(QObject *parent) : QObject(parent) {}
GMRES::~GMRES() = default;

/* ---- Configuration ---- */

void GMRES::setMaxIterations(int iter) { m_maxIterations = qMax(1, iter); }
void GMRES::setTolerance(double tol) { m_tolerance = qMax(1e-15, tol); }
void GMRES::setRestartInterval(int m) { m_restartM = qMax(1, m); }
void GMRES::setFlexiblePreconditioning(bool enabled) { m_flexible = enabled; }

/* ---- Vector operations ---- */

double GMRES::dot(const QVector<double>& a, const QVector<double>& b) const
{
    double s = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

double GMRES::norm(const QVector<double>& v) const
{
    return qSqrt(dot(v, v));
}

QVector<double> GMRES::matVec(const QVector<QVector<double>>& A,
                                const QVector<double>& x) const
{
    int n = A.size();
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            y[i] += A[i][j] * x[j];
    return y;
}

QVector<double> GMRES::sparseMatVec(const QVector<int>& rowPtr,
                                      const QVector<int>& colIdx,
                                      const QVector<double>& values,
                                      const QVector<double>& x) const
{
    int n = rowPtr.size() - 1;
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int k = rowPtr[i]; k < rowPtr[i + 1]; ++k)
            y[i] += values[k] * x[colIdx[k]];
    }
    return y;
}

/* ---- Preconditioner (Jacobi) ---- */

QVector<double> GMRES::precondition(const QVector<QVector<double>>& A,
                                      const QVector<double>& r) const
{
    int n = r.size();
    QVector<double> z(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double diag = qAbs(A[i][i]);
        z[i] = (diag > 1e-15) ? r[i] / diag : r[i];
    }
    return z;
}

/* ---- Back-substitution ---- */

QVector<double> GMRES::backSolve(const QVector<QVector<double>>& H,
                                   const QVector<double>& g, int k) const
{
    QVector<double> y(k + 1, 0.0);
    for (int i = k; i >= 0; --i) {
        y[i] = g[i];
        for (int j = i + 1; j <= k; ++j)
            y[i] -= H[i][j] * y[j];
        if (qAbs(H[i][i]) > 1e-15)
            y[i] /= H[i][i];
    }
    return y;
}

/* ---- Main solve (dense) ---- */

QVector<double> GMRES::solve(const QVector<QVector<double>>& A,
                               const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    int n = b.size();
    if (n == 0) return {};

    // Initial guess x0 = 0
    QVector<double> x(n, 0.0);
    QVector<double> r = b; // r = b - A*x0 = b
    double beta0 = norm(r);
    if (beta0 < 1e-15) {
        m_stats.totalSolves++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
        return x;
    }

    double initialRes = beta0;
    int totalIter = 0;

    // Restarted GMRES
    for (int restart = 0; restart < m_maxIterations; ++restart) {
        // Recompute residual
        r = b;
        QVector<double> ax = matVec(A, x);
        for (int i = 0; i < n; ++i) r[i] -= ax[i];

        double beta = norm(r);
        if (beta / initialRes < m_tolerance) break;

        // Krylov subspace dimension
        int m = qMin(m_restartM, n);

        // Arnoldi process
        QVector<QVector<double>> V(m + 1, QVector<double>(n, 0.0));
        QVector<QVector<double>> H(m + 1, QVector<double>(m, 0.0));

        // V[0] = r / ||r||
        for (int i = 0; i < n; ++i) V[0][i] = r[i] / beta;

        QVector<double> g(m + 1, 0.0);
        g[0] = beta;

        // Givens rotation cos/sin
        QVector<double> cs(m, 0.0), sn(m, 0.0);

        int k = 0;
        for (k = 0; k < m; ++k) {
            // w = A * V[k]
            QVector<double> w;
            if (m_flexible) {
                QVector<double> z = precondition(A, V[k]);
                w = matVec(A, z);
            } else {
                w = matVec(A, V[k]);
            }

            // Modified Gram-Schmidt orthogonalization
            for (int j = 0; j <= k; ++j) {
                H[j][k] = dot(w, V[j]);
                for (int i = 0; i < n; ++i)
                    w[i] -= H[j][k] * V[j][i];
            }
            H[k + 1][k] = norm(w);

            if (H[k + 1][k] < 1e-15) { k++; break; }

            for (int i = 0; i < n; ++i)
                V[k + 1][i] = w[i] / H[k + 1][k];

            // Apply previous Givens rotations
            for (int j = 0; j < k; ++j) {
                double temp = cs[j] * H[j][k] + sn[j] * H[j + 1][k];
                H[j + 1][k] = -sn[j] * H[j][k] + cs[j] * H[j + 1][k];
                H[j][k] = temp;
            }

            // New Givens rotation
            double rr = qSqrt(H[k][k] * H[k][k] + H[k + 1][k] * H[k + 1][k]);
            cs[k] = H[k][k] / rr;
            sn[k] = H[k + 1][k] / rr;
            H[k][k] = rr;
            H[k + 1][k] = 0.0;

            g[k + 1] = -sn[k] * g[k];
            g[k] = cs[k] * g[k];

            totalIter++;
            if (qAbs(g[k + 1]) / initialRes < m_tolerance) { k++; break; }
        }

        // Solve least-squares: y = H\g
        QVector<double> y = backSolve(H, g, k - 1);

        // Update x: x = x + V * y
        for (int j = 0; j < k; ++j)
            for (int i = 0; i < n; ++i)
                x[i] += V[j][i] * y[j];
    }

    double finalRes = residual(A, x, b);
    m_stats.totalSolves++;
    m_stats.iterationsUsed = totalIter;
    m_stats.initialResidual = initialRes;
    m_stats.finalResidual = finalRes;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(totalIter, finalRes);
    return x;
}

/* ---- Sparse solve ---- */

QVector<double> GMRES::solveSparse(const QVector<int>& rowPtr,
                                     const QVector<int>& colIdx,
                                     const QVector<double>& values,
                                     const QVector<double>& b)
{
    int n = b.size();
    if (n == 0) return {};

    // Convert to dense for simplicity (sparse path delegates to same GMRES core)
    QVector<QVector<double>> A(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        for (int k = rowPtr[i]; k < rowPtr[i + 1]; ++k)
            A[i][colIdx[k]] = values[k];

    return solve(A, b);
}

/* ---- Residual ---- */

double GMRES::residual(const QVector<QVector<double>>& A,
                         const QVector<double>& x,
                         const QVector<double>& b) const
{
    QVector<double> ax = matVec(A, x);
    int n = b.size();
    double sum = 0.0;
    for (int i = 0; i < n; ++i) {
        double d = ax[i] - b[i];
        sum += d * d;
    }
    return qSqrt(sum);
}

/* ---- Reset ---- */

void GMRES::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
