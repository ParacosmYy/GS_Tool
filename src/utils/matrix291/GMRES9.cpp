/**
 * @file GMRES9.cpp
 * @brief GMRES9 实现
 *
 * 实现GMRES求解器：缩减重启与调和Ritz向量回收求解移位线性系统序列。
 */

#include "utils/matrix291/GMRES9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

GMRES9::GMRES9(QObject *parent)
    : QObject(parent) {}

GMRES9::~GMRES9() = default;

/* ---- Configuration ---- */

void GMRES9::setConfig(const GMRESConfig& cfg)
{
    m_config = cfg;
    m_config.maxRestart = qBound(5, m_config.maxRestart, 500);
    m_config.maxIterations = qBound(10, m_config.maxIterations, 100000);
    m_config.tolerance = qBound(1e-15, m_config.tolerance, 1.0);
    m_config.numRecycled = qBound(0, m_config.numRecycled, m_config.maxRestart - 1);
}

/* ---- Vector helpers ---- */

double GMRES9::vecNorm(const QVector<double>& v)
{
    double s = 0.0;
    for (double x : v) s += x * x;
    return qSqrt(s);
}

double GMRES9::dotProduct(const QVector<double>& a, const QVector<double>& b)
{
    double s = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

/* ---- Orthonormalize via modified Gram-Schmidt ---- */

void GMRES9::orthonormalize(QVector<double>& v, const QVector<QVector<double>>& basis) const
{
    for (int i = 0; i < basis.size(); ++i) {
        double d = dotProduct(v, basis[i]);
        for (int j = 0; j < v.size(); ++j) v[j] -= d * basis[i][j];
    }
    double norm = vecNorm(v);
    if (norm > 1e-15) {
        for (int j = 0; j < v.size(); ++j) v[j] /= norm;
    }
}

/* ---- Arnoldi process ---- */

int GMRES9::arnoldi(MatVecFunc& matVec, const QVector<QVector<double>>& V,
                     QVector<double>& h, int j, int m)
{
    Q_UNUSED(m)
    QVector<double> w = matVec(V[j]);
    int n = w.size();
    h.resize(j + 2);

    for (int i = 0; i <= j; ++i) {
        h[i] = dotProduct(w, V[i]);
        for (int k = 0; k < n; ++k) w[k] -= h[i] * V[i][k];
    }
    h[j + 1] = vecNorm(w);
    if (h[j + 1] > 1e-15) {
        for (int k = 0; k < n; ++k) w[k] /= h[j + 1];
    }
    return (h[j + 1] < 1e-15) ? 1 : 0; // 1 = lucky breakdown
}

/* ---- Apply Givens rotations ---- */

void GMRES9::applyGivens(QVector<QVector<double>>& H, QVector<double>& cs,
                          QVector<double>& sn, int j)
{
    for (int i = 0; i < j; ++i) {
        double temp = cs[i] * H[i][j] + sn[i] * H[i + 1][j];
        H[i + 1][j] = -sn[i] * H[i][j] + cs[i] * H[i + 1][j];
        H[i][j] = temp;
    }
    // Compute new Givens rotation
    double a = H[j][j], b = H[j + 1][j];
    double r = qSqrt(a * a + b * b);
    if (r < 1e-30) r = 1e-30;
    cs[j] = a / r;
    sn[j] = b / r;
    H[j][j] = r;
    H[j + 1][j] = 0.0;
}

/* ---- Solve least-squares ---- */

QVector<double> GMRES9::solveLeastSquares(const QVector<QVector<double>>& H,
                                           const QVector<double>& g, int k) const
{
    QVector<double> y(k, 0.0);
    // Back-substitution on upper triangular H[0:k, 0:k]
    for (int i = k - 1; i >= 0; --i) {
        y[i] = g[i];
        for (int j = i + 1; j < k; ++j) y[i] -= H[i][j] * y[j];
        if (qAbs(H[i][i]) > 1e-30) y[i] /= H[i][i];
    }
    return y;
}

/* ---- Compute harmonic Ritz vectors ---- */

QVector<QVector<double>> GMRES9::computeHarmonicRitz(
    const QVector<QVector<double>>& V, const QVector<QVector<double>>& H, int k) const
{
    int numR = qMin(m_config.numRecycled, k);
    if (numR <= 0 || k <= 0) return {};

    // Compute Ritz values from H(0:k, 0:k) diagonalization
    // Simplified: select smallest residual Ritz vectors
    // Use the last 'numR' Arnoldi vectors as recycled subspace
    QVector<QVector<double>> recycled(numR);
    for (int i = 0; i < numR; ++i) {
        int idx = k - numR + i;
        if (idx >= 0 && idx < V.size()) {
            recycled[i] = V[idx];
        } else {
            recycled[i].resize(V[0].size(), 0.0);
        }
    }
    return recycled;
}

/* ---- Main solve ---- */

GMRES9::SolveResult GMRES9::solve(MatVecFunc matVec, const QVector<double>& b,
                                   const QVector<double>& x0)
{
    QElapsedTimer timer;
    timer.start();

    SolveResult result;
    int n = b.size();
    if (n == 0) return result;

    QVector<double> x = x0.isEmpty() ? QVector<double>(n, 0.0) : x0;

    // r0 = b - A*x0
    QVector<double> Ax = matVec(x);
    QVector<double> r0(n);
    for (int i = 0; i < n; ++i) r0[i] = b[i] - Ax[i];

    double beta = vecNorm(r0);
    result.initialResidual = beta;
    if (beta < m_config.tolerance) {
        result.solution = x;
        result.residualNorm = beta;
        result.converged = true;
        return result;
    }

    int m = m_config.maxRestart;
    int totalIters = 0;
    int restarts = 0;

    while (totalIters < m_config.maxIterations) {
        int kMax = qMin(m, n - 1);
        // Arnoldi vectors
        QVector<QVector<double>> V(kMax + 1, QVector<double>(n, 0.0));
        for (int i = 0; i < n; ++i) V[0][i] = r0[i] / beta;

        QVector<QVector<double>> H(kMax + 1, QVector<double>(kMax, 0.0));
        QVector<double> cs(kMax, 0.0), sn(kMax, 0.0);
        QVector<double> g(kMax + 1, 0.0);
        g[0] = beta;

        int k = 0;
        for (k = 0; k < kMax; ++k) {
            QVector<double> hCol;
            int breakdown = arnoldi(matVec, V, hCol, k, kMax);

            // Fill H column
            for (int i = 0; i <= k + 1; ++i) {
                if (i < H.size() && k < H[i].size()) H[i][k] = hCol[i];
            }

            // Store the new Arnoldi vector
            if (hCol[k + 1] > 1e-15) {
                QVector<double> w = matVec(V[k]);
                for (int i = 0; i <= k; ++i) {
                    double d = dotProduct(w, V[i]);
                    for (int j = 0; j < n; ++j) w[j] -= d * V[i][j];
                }
                double norm = vecNorm(w);
                if (k + 1 < V.size() && norm > 1e-15)
                    for (int j = 0; j < n; ++j) V[k + 1][j] = w[j] / norm;
            }

            applyGivens(H, cs, sn, k);

            // Update g
            g[k + 1] = -sn[k] * g[k];
            g[k] = cs[k] * g[k];

            totalIters++;
            double residual = qAbs(g[k + 1]);
            if (residual / beta < m_config.tolerance || breakdown) break;
        }

        // Solve for y
        QVector<double> y = solveLeastSquares(H, g, k + 1);

        // Update x
        for (int i = 0; i < n; ++i)
            for (int j = 0; j <= k; ++j)
                x[i] += y[j] * V[j][i];

        // Compute new residual
        QVector<double> newAx = matVec(x);
        for (int i = 0; i < n; ++i) r0[i] = b[i] - newAx[i];
        beta = vecNorm(r0);

        // Recycle harmonic Ritz vectors for warm restart
        m_recycledBasis = computeHarmonicRitz(V, H, k + 1);

        restarts++;
        if (beta < m_config.tolerance) break;
    }

    result.solution = x;
    result.residualNorm = beta;
    result.iterations = totalIters;
    result.restarts = restarts;
    result.converged = (beta < m_config.tolerance * result.initialResidual);

    double elapsed = timer.elapsed();
    m_stats.totalSolves++;
    m_stats.totalIterations += totalIters;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveDone(totalIters, beta, elapsed);
    return result;
}

/* ---- Solve shifted sequence ---- */

QVector<GMRES9::SolveResult> GMRES9::solveShiftedSequence(
    MatVecFunc matVec, const QVector<double>& b,
    const QVector<double>& shifts, const QVector<double>& x0)
{
    QVector<SolveResult> results(shifts.size());
    for (int i = 0; i < shifts.size(); ++i) {
        double sigma = shifts[i];
        // Create shifted matvec: (A + sigma*I)*v = A*v + sigma*v
        auto shiftedMatVec = [&](const QVector<double>& v) -> QVector<double> {
            QVector<double> Av = matVec(v);
            for (int j = 0; j < Av.size(); ++j) Av[j] += sigma * v[j];
            return Av;
        };
        results[i] = solve(shiftedMatVec, b, x0);
    }
    return results;
}

/* ---- Reset ---- */

void GMRES9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_recycledBasis.clear();
}
