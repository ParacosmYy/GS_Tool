/**
 * @file GMRES10.cpp
 * @brief GMRES10 实现
 *
 * 实现GMRES：嵌套Krylov子空间回收与压缩重启实现多右端项线性系统求解。
 */

#include "utils/matrix302/GMRES10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

GMRES10::GMRES10(QObject *parent)
    : QObject(parent) {}

GMRES10::~GMRES10() = default;

/* ---- Configuration ---- */

void GMRES10::setConfig(const SolverConfig& config)
{
    m_config = config;
    m_config.maxIterations = qBound(1, config.maxIterations, 100000);
    m_config.restartLength = qBound(2, config.restartLength, 500);
    m_config.tolerance = qBound(1e-15, config.tolerance, 1.0);
    m_config.numDeflationVectors = qBound(0, config.numDeflationVectors, 50);
}

/* ---- Matrix-vector multiply ---- */

QVector<double> GMRES10::matVec(const QVector<QVector<double>>& A,
                                  const QVector<double>& x) const
{
    int n = A.size();
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i) {
        int rowLen = qMin(A[i].size(), x.size());
        for (int j = 0; j < rowLen; ++j)
            y[i] += A[i][j] * x[j];
    }
    return y;
}

/* ---- Dot product ---- */

double GMRES10::dot(const QVector<double>& a, const QVector<double>& b) const
{
    double s = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i)
        s += a[i] * b[i];
    return s;
}

/* ---- Vector norm ---- */

double GMRES10::norm(const QVector<double>& v) const
{
    return qSqrt(dot(v, v));
}

/* ---- Arnoldi iteration ---- */

void GMRES10::arnoldi(const QVector<QVector<double>>& A,
                        const QVector<double>& q,
                        QVector<QVector<double>>& Q,
                        QVector<double>& h, int k) const
{
    // w = A * q
    auto w = matVec(A, q);
    int n = w.size();

    // Modified Gram-Schmidt
    h.resize(k + 2);
    for (int i = 0; i <= k; ++i) {
        h[i] = dot(w, Q[i]);
        for (int j = 0; j < n; ++j)
            w[j] -= h[i] * Q[i][j];
    }
    h[k + 1] = norm(w);

    // Normalize
    if (h[k + 1] > 1e-15) {
        for (int j = 0; j < n; ++j)
            w[j] /= h[k + 1];
    }
    Q[k + 1] = w;
}

/* ---- Solve upper Hessenberg via Givens rotations ---- */

QVector<double> GMRES10::solveHessenberg(const QVector<QVector<double>>& H,
                                            const QVector<double>& g,
                                            int m) const
{
    // Back-substitution on (m+1) x m system
    QVector<double> y(m, 0.0);
    for (int i = m - 1; i >= 0; --i) {
        y[i] = g[i];
        for (int j = i + 1; j < m; ++j)
            y[i] -= H[i][j] * y[j];
        if (qAbs(H[i][i]) > 1e-15)
            y[i] /= H[i][i];
    }
    return y;
}

/* ---- Apply deflation: project out recycled vectors ---- */

QVector<double> GMRES10::applyDeflation(const QVector<double>& v) const
{
    auto result = v;
    for (auto& dv : m_deflationVectors) {
        double d = dot(result, dv);
        int n = qMin(result.size(), dv.size());
        for (int i = 0; i < n; ++i)
            result[i] -= d * dv[i];
    }
    return result;
}

/* ---- Update deflation vectors from converged solution ---- */

void GMRES10::updateDeflation(const QVector<QVector<double>>& Q,
                                int m, const QVector<double>& y)
{
    // Add the solution direction to deflation subspace
    if (m_deflationVectors.size() >= m_config.numDeflationVectors)
        m_deflationVectors.removeFirst();

    int n = Q[0].size();
    QVector<double> newVec(n, 0.0);
    for (int i = 0; i < qMin(m, y.size()); ++i)
        for (int j = 0; j < n; ++j)
            newVec[j] += y[i] * Q[i][j];

    // Orthonormalize against existing
    for (auto& dv : m_deflationVectors) {
        double d = dot(newVec, dv);
        for (int j = 0; j < n; ++j)
            newVec[j] -= d * dv[j];
    }
    double nrm = norm(newVec);
    if (nrm > 1e-10) {
        for (int j = 0; j < n; ++j)
            newVec[j] /= nrm;
        m_deflationVectors.append(newVec);
    }
}

/* ---- Main solve ---- */

GMRES10::SolveResult GMRES10::solve(const QVector<QVector<double>>& A,
                                      const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    SolveResult result;
    int n = b.size();
    if (n == 0 || A.size() != n) return result;

    // Initial guess x0 = 0
    QVector<double> x(n, 0.0);
    auto r0 = matVec(A, x);
    for (int i = 0; i < n; ++i)
        r0[i] = b[i] - r0[i];

    // Apply deflation
    r0 = applyDeflation(r0);

    double beta = norm(r0);
    result.initialResidualNorm = beta;
    if (beta < m_config.tolerance) {
        result.solution = x;
        result.residualNorm = beta;
        result.converged = true;
        result.iterations = 0;
        return result;
    }

    int totalIter = 0;
    bool converged = false;
    int m = m_config.restartLength;

    while (totalIter < m_config.maxIterations && !converged) {
        // Build Krylov basis
        QVector<QVector<double>> Q(m + 1);
        Q[0].resize(n);
        for (int i = 0; i < n; ++i)
            Q[0][i] = r0[i] / beta;

        QVector<QVector<double>> H(m + 1, QVector<double>(m, 0.0));
        QVector<double> g(m + 1, 0.0);
        g[0] = beta;

        // Givens rotation cos/sin
        QVector<double> cs(m), sn(m);

        int k = 0;
        for (; k < m && totalIter < m_config.maxIterations; ++k, ++totalIter) {
            QVector<double> h;
            arnoldi(A, Q[k], Q, h, k);

            // Copy h into H column k
            for (int i = 0; i <= k + 1 && i < h.size(); ++i)
                H[i][k] = h[i];

            // Apply previous Givens rotations
            for (int i = 0; i < k; ++i) {
                double temp = cs[i] * H[i][k] + sn[i] * H[i + 1][k];
                H[i + 1][k] = -sn[i] * H[i][k] + cs[i] * H[i + 1][k];
                H[i][k] = temp;
            }

            // New Givens rotation
            double r = qSqrt(H[k][k] * H[k][k] + H[k + 1][k] * H[k + 1][k]);
            if (r > 1e-15) {
                cs[k] = H[k][k] / r;
                sn[k] = H[k + 1][k] / r;
            } else {
                cs[k] = 1.0;
                sn[k] = 0.0;
            }

            H[k][k] = cs[k] * H[k][k] + sn[k] * H[k + 1][k];
            H[k + 1][k] = 0.0;

            g[k + 1] = -sn[k] * g[k];
            g[k] = cs[k] * g[k];

            double residual = qAbs(g[k + 1]);
            if (residual / beta < m_config.tolerance) {
                converged = true;
                k++;
                break;
            }
        }

        // Solve least-squares
        auto y = solveHessenberg(H, g, k);

        // Update solution: x = x + Q * y
        for (int i = 0; i < k; ++i)
            for (int j = 0; j < n; ++j)
                x[j] += y[i] * Q[i][j];

        // Update deflation
        updateDeflation(Q, k, y);

        if (!converged) {
            // Restart: compute new residual
            auto ax = matVec(A, x);
            for (int i = 0; i < n; ++i)
                r0[i] = b[i] - ax[i];
            r0 = applyDeflation(r0);
            beta = norm(r0);
        }

        result.residualNorm = qAbs(g[k]);
    }

    result.solution = x;
    result.iterations = totalIter;
    result.converged = converged;

    m_stats.totalSolves++;
    m_stats.problemSize = n;
    m_iterSum += totalIter;
    m_stats.avgIterations = static_cast<double>(m_iterSum) / m_stats.totalSolves;

    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveDone(n, totalIter, result.residualNorm, elapsed);
    return result;
}

/* ---- Solve multiple RHS using recycled subspace ---- */

QVector<GMRES10::SolveResult> GMRES10::solveMultiple(
    const QVector<QVector<double>>& A,
    const QVector<QVector<double>>& B)
{
    QVector<SolveResult> results;
    results.reserve(B.size());
    for (auto& b : B)
        results.append(solve(A, b));
    return results;
}

/* ---- Reset ---- */

void GMRES10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_iterSum = 0;
    m_deflationVectors.clear();
}
