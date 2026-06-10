/**
 * @file GMRES7.cpp
 * @brief GMRES7 实现
 *
 * 实现GMRES：重启Arnoldi过程与Givens旋转QR更新Krylov子空间最小二乘求解。
 */

#include "utils/matrix274/GMRES7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

GMRES7::GMRES7(QObject *parent)
    : QObject(parent) {}

GMRES7::~GMRES7() = default;

/* ---- Configuration ---- */

void GMRES7::setKrylovDimension(int m)
{
    m_krylovDim = qBound(5, m, 500);
}

void GMRES7::setTolerance(double tol)
{
    m_tol = qBound(1e-14, tol, 1.0);
}

void GMRES7::setMaxRestarts(int maxR)
{
    m_maxRestarts = qBound(1, maxR, 10000);
}

/* ---- Basic vector operations ---- */

QVector<double> GMRES7::matVec(const QVector<QVector<double>>& A,
                                const QVector<double>& x) const
{
    int n = A.size();
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i) {
        int cols = qMin(A[i].size(), x.size());
        for (int j = 0; j < cols; ++j)
            y[i] += A[i][j] * x[j];
    }
    return y;
}

double GMRES7::dot(const QVector<double>& a, const QVector<double>& b) const
{
    double s = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i)
        s += a[i] * b[i];
    return s;
}

double GMRES7::norm(const QVector<double>& v) const
{
    return qSqrt(dot(v, v));
}

/* ---- Apply accumulated Givens rotations ---- */

void GMRES7::applyGivens(QVector<double>& col,
                          const QVector<QPair<double, double>>& rotations,
                          int startRow) const
{
    for (int i = 0; i < startRow && i < rotations.size(); ++i) {
        double c = rotations[i].first;
        double s = rotations[i].second;
        double h1 = col[i];
        double h2 = col[i + 1];
        col[i] = c * h1 + s * h2;
        col[i + 1] = -s * h1 + c * h2;
    }
}

/* ---- Back-substitution ---- */

QVector<double> GMRES7::backSubstitute(const QVector<QVector<double>>& R,
                                        const QVector<double>& rhs, int k) const
{
    QVector<double> y(k, 0.0);
    for (int i = k - 1; i >= 0; --i) {
        y[i] = rhs[i];
        for (int j = i + 1; j < k; ++j)
            y[i] -= R[i][j] * y[j];
        if (qAbs(R[i][i]) > 1e-15)
            y[i] /= R[i][i];
    }
    return y;
}

/* ---- Main GMRES solve ---- */

QVector<double> GMRES7::solve(const QVector<QVector<double>>& A,
                               const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    int n = b.size();
    if (n == 0 || A.isEmpty()) return b;

    m_iterations = 0;
    m_residual = 0.0;

    // Initial guess x0 = 0
    QVector<double> x(n, 0.0);

    double bNorm = norm(b);
    if (bNorm < 1e-15) return x;

    for (int restart = 0; restart < m_maxRestarts; ++restart) {
        // Compute residual r = b - A*x
        QVector<double> r = b;
        QVector<double> Ax = matVec(A, x);
        for (int i = 0; i < n; ++i) r[i] -= Ax[i];

        double rNorm = norm(r);
        m_residual = rNorm / bNorm;

        if (m_residual < m_tol) break;

        // Normalize residual to get v1
        int m = qMin(m_krylovDim, n);
        QVector<QVector<double>> V(m + 1, QVector<double>(n, 0.0));
        for (int i = 0; i < n; ++i)
            V[0][i] = r[i] / rNorm;

        // Hessenberg matrix and Givens rotations
        QVector<QVector<double>> H(m + 1, QVector<double>(m, 0.0));
        QVector<QPair<double, double>> givens; // (cos, sin)
        QVector<double> gRhs(m + 1, 0.0);
        gRhs[0] = rNorm;

        int k = 0;
        for (k = 0; k < m; ++k) {
            m_iterations++;

            // Arnoldi step: w = A * V[k]
            QVector<double> w = matVec(A, V[k]);

            // Modified Gram-Schmidt orthogonalization
            for (int j = 0; j <= k; ++j) {
                H[j][k] = dot(w, V[j]);
                for (int i = 0; i < n; ++i)
                    w[i] -= H[j][k] * V[j][i];
            }
            H[k + 1][k] = norm(w);

            if (H[k + 1][k] < 1e-15) {
                // Lucky breakdown
                k++;
                break;
            }

            for (int i = 0; i < n; ++i)
                V[k + 1][i] = w[i] / H[k + 1][k];

            // Apply previous Givens rotations to new column of H
            QVector<double> hCol(k + 2);
            for (int j = 0; j <= k + 1; ++j) hCol[j] = H[j][k];
            applyGivens(hCol, givens, k);

            // Compute new Givens rotation
            double hBot = hCol[k + 1];
            double hTop = hCol[k];
            double rH = qSqrt(hTop * hTop + hBot * hBot);
            double c = (rH > 1e-15) ? hTop / rH : 1.0;
            double s = (rH > 1e-15) ? hBot / rH : 0.0;
            givens.append({c, s});

            hCol[k] = c * hTop + s * hBot;
            hCol[k + 1] = 0.0;

            // Update R column and rhs
            for (int j = 0; j <= k; ++j) H[j][k] = hCol[j];
            gRhs[k + 1] = -s * gRhs[k];
            gRhs[k] = c * gRhs[k];

            // Check convergence
            m_residual = qAbs(gRhs[k + 1]) / bNorm;
            if (m_residual < m_tol) {
                k++;
                break;
            }
        }

        // Solve upper triangular system R * y = gRhs
        QVector<double> y = backSubstitute(H, gRhs, k);

        // Update solution x = x + V * y
        for (int j = 0; j < k; ++j)
            for (int i = 0; i < n; ++i)
                x[i] += V[j][i] * y[j];

        if (m_residual < m_tol) break;
    }

    double elapsed = timer.elapsed();
    m_stats.matrixSize = n;
    m_stats.krylovDim = m_krylovDim;
    m_stats.iterations = m_iterations;
    m_stats.residual = m_residual;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit solveCompleted(m_iterations, m_residual, elapsed);

    return x;
}

/* ---- Accessors ---- */

double GMRES7::residual() const { return m_residual; }
int GMRES7::iterations() const { return m_iterations; }

/* ---- Reset ---- */

void GMRES7::resetStatistics()
{
    m_residual = 0.0;
    m_iterations = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
