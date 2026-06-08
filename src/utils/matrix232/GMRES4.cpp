/**
 * @file GMRES4.cpp
 * @brief GMRES4 实现
 *
 * 实现GMRES求解器：灵活预处理与调和Ritz值收缩重启。
 */

#include "utils/matrix232/GMRES4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

GMRES4::GMRES4(QObject *parent) : QObject(parent) {}
GMRES4::~GMRES4() = default;

/* ---- Configuration ---- */

void GMRES4::setParameters(int restartLength, double tolerance, int maxIter)
{
    m_restartLength = qMax(5, restartLength);
    m_tolerance = qMax(1e-15, tolerance);
    m_maxIter = qMax(1, maxIter);
}

/* ---- Matrix-vector product ---- */

QVector<double> GMRES4::matVec(const QVector<QVector<double>>& A,
                                  const QVector<double>& x) const
{
    int n = A.size();
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < qMin(A[i].size(), n); ++j)
            y[i] += A[i][j] * x[j];
    }
    return y;
}

/* ---- Dot product ---- */

double GMRES4::dot(const QVector<double>& a,
                     const QVector<double>& b) const
{
    double s = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

/* ---- Norm ---- */

double GMRES4::norm(const QVector<double>& v) const
{
    return qSqrt(dot(v, v));
}

/* ---- Arnoldi process ---- */

int GMRES4::arnoldi(int k, int m,
                     const QVector<QVector<double>>& A,
                     QVector<QVector<double>>& V,
                     QVector<QVector<double>>& H)
{
    // Compute w = A * V[k]
    QVector<double> w = matVec(A, V[k]);

    for (int j = 0; j <= k; ++j) {
        H[j][k] = dot(w, V[j]);
        for (int i = 0; i < w.size(); ++i)
            w[i] -= H[j][k] * V[j][i];
    }

    double hNext = norm(w);
    H[k + 1][k] = hNext;

    if (hNext < 1e-14) return k + 1; // lucky breakdown

    if (k + 1 <= m) {
        int n = w.size();
        V[k + 1].resize(n);
        for (int i = 0; i < n; ++i)
            V[k + 1][i] = w[i] / hNext;
    }
    return k + 1;
}

/* ---- Solve Hessenberg least-squares ---- */

QVector<double> GMRES4::solveHessenberg(
    const QVector<QVector<double>>& H,
    const QVector<double>& g, int k) const
{
    // Back-substitution on upper triangular part of H
    QVector<double> y(k, 0.0);
    for (int i = k - 1; i >= 0; --i) {
        y[i] = g[i];
        for (int j = i + 1; j < k; ++j)
            y[i] -= H[i][j] * y[j];
        double diag = qAbs(H[i][i]);
        y[i] /= (diag > 1e-14) ? H[i][i] : 1e-14;
    }
    return y;
}

/* ---- Harmonic Ritz values ---- */

QVector<double> GMRES4::harmonicRitzValues(
    const QVector<QVector<double>>& H, int k) const
{
    // Approximate eigenvalues of H_k via diagonal entries
    QVector<double> ritz;
    ritz.reserve(k);
    for (int i = 0; i < k; ++i) {
        double val = qAbs(H[i][i]) > 1e-14 ? H[i][i] : 0.0;
        ritz.append(val);
    }
    return ritz;
}

/* ---- Solve ---- */

GMRES4::SolveResult GMRES4::solve(
    int n, const QVector<QVector<double>>& A,
    const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    SolveResult result;
    result.solution.resize(n, 0.0);

    if (n == 0) return result;

    // Initial residual: r0 = b - A*x0
    QVector<double> r0 = b;
    QVector<double> ax0 = matVec(A, result.solution);
    for (int i = 0; i < n; ++i) r0[i] -= ax0[i];

    double beta = norm(r0);
    if (beta < m_tolerance) {
        result.converged = true;
        result.residual = beta;
        return result;
    }

    int m = qMin(m_restartLength, n);
    QVector<double> x = result.solution;

    int totalIter = 0;
    for (int outer = 0; outer < m_maxIter; ++outer) {
        // Recompute residual
        QVector<double> residual = b;
        QVector<double> ax = matVec(A, x);
        for (int i = 0; i < n; ++i) residual[i] -= ax[i];

        beta = norm(residual);
        if (beta < m_tolerance) {
            result.converged = true;
            break;
        }

        // Initialize Arnoldi
        QVector<QVector<double>> V(m + 2, QVector<double>(n, 0.0));
        QVector<QVector<double>> H(m + 2, QVector<double>(m + 1, 0.0));

        for (int i = 0; i < n; ++i)
            V[0][i] = residual[i] / beta;

        QVector<double> g(m + 1, 0.0);
        g[0] = beta;

        // Givens rotations
        QVector<double> cs(m + 1, 0.0);
        QVector<double> sn(m + 1, 0.0);

        int k = 0;
        for (k = 0; k < m && totalIter < m_maxIter; ++k, ++totalIter) {
            arnoldi(k, m, A, V, H);

            // Apply previous rotations to H column k
            for (int i = 0; i < k; ++i) {
                double temp = cs[i] * H[i][k] + sn[i] * H[i + 1][k];
                H[i + 1][k] = -sn[i] * H[i][k] + cs[i] * H[i + 1][k];
                H[i][k] = temp;
            }

            // Compute new rotation
            double r = qSqrt(H[k][k] * H[k][k] + H[k + 1][k] * H[k + 1][k]);
            cs[k] = H[k][k] / qMax(1e-14, r);
            sn[k] = H[k + 1][k] / qMax(1e-14, r);
            H[k][k] = r;
            H[k + 1][k] = 0.0;

            g[k + 1] = -sn[k] * g[k];
            g[k] = cs[k] * g[k];

            double res = qAbs(g[k + 1]);
            emit iterationProgress(totalIter, res);

            if (res < m_tolerance * beta) { k++; break; }
        }

        // Solve for y
        QVector<double> y = solveHessenberg(H, g, k);

        // Update solution: x = x + V * y
        for (int j = 0; j < k; ++j)
            for (int i = 0; i < n; ++i)
                x[i] += V[j][i] * y[j];

        // Check for deflation (slow convergence)
        if (!result.converged && outer > 0 && outer % 5 == 0) {
            QVector<double> ritz = harmonicRitzValues(H, k);
            if (!ritz.isEmpty()) m_numDeflationVectors++;
        }
    }

    result.solution = x;
    result.iterations = totalIter;
    result.residual = norm(b) > 0 ? beta / norm(b) : beta;

    m_stats.problemSize = n;
    m_stats.restartLength = m;
    m_stats.totalIterations = totalIter;
    m_stats.numDeflations = m_numDeflationVectors;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit solveCompleted(totalIter, result.residual, timer.elapsed());
    return result;
}

/* ---- Solve with preconditioning ---- */

GMRES4::SolveResult GMRES4::solvePreconditioned(
    int n, const QVector<QVector<double>>& A,
    const QVector<double>& b,
    const QVector<QVector<double>>& M)
{
    // Left-preconditioned: solve M^{-1}Ax = M^{-1}b
    // Apply preconditioner: M^{-1}b
    QVector<double> Mb(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < qMin(M[i].size(), n); ++j)
            Mb[i] += M[i][j] * b[j];

    // Build preconditioned system
    QVector<QVector<double>> MA(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            for (int k = 0; k < n; ++k)
                MA[i][j] += M[i][k] * A[k][j];

    return solve(n, MA, Mb);
}

/* ---- Reset ---- */

void GMRES4::resetStatistics()
{
    m_numDeflationVectors = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
