/**
 * @file IterativeRefinement2.cpp
 * @brief IterativeRefinement2 实现
 *
 * 实现迭代精化：混合精度残差计算、GMRES内校正扫描。
 */

#include "utils/matrix216/IterativeRefinement2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <limits>
#include <algorithm>

/* ---- Construction / Destruction ---- */

IterativeRefinement2::IterativeRefinement2(QObject *parent) : QObject(parent) {}
IterativeRefinement2::~IterativeRefinement2() = default;

/* ---- Configuration ---- */

void IterativeRefinement2::setMaxIterations(int maxIter) { m_maxIter = qMax(1, maxIter); }
void IterativeRefinement2::setTolerance(double tol) { m_tol = qMax(1e-15, tol); }
void IterativeRefinement2::setGMRESIterations(int iters) { m_gmresIter = qMax(1, iters); }

/* ---- Vector utilities ---- */

QVector<double> IterativeRefinement2::matVec(const QVector<QVector<double>>& A,
                                              const QVector<double>& x)
{
    int n = A.size();
    QVector<double> result(n, 0.0);
    for (int i = 0; i < n; ++i) {
        int m = qMin(A[i].size(), x.size());
        for (int j = 0; j < m; ++j)
            result[i] += A[i][j] * x[j];
    }
    return result;
}

double IterativeRefinement2::vecNorm(const QVector<double>& v)
{
    double sum = 0.0;
    for (double x : v) sum += x * x;
    return qSqrt(sum);
}

/* ---- Compute residual ---- */

QVector<double> IterativeRefinement2::computeResidual(
    const QVector<QVector<double>>& A,
    const QVector<double>& x,
    const QVector<double>& b)
{
    int n = b.size();
    QVector<double> r(n, 0.0);
    auto ax = matVec(A, x);
    for (int i = 0; i < n; ++i)
        r[i] = b[i] - ax[i];
    return r;
}

/* ---- Back substitution ---- */

QVector<double> IterativeRefinement2::backSolve(const QVector<QVector<double>>& U,
                                                  const QVector<double>& b)
{
    int n = b.size();
    QVector<double> x(n, 0.0);
    for (int i = n - 1; i >= 0; --i) {
        double sum = b[i];
        for (int j = i + 1; j < n; ++j)
            sum -= U[i][j] * x[j];
        x[i] = (qAbs(U[i][i]) > 1e-15) ? sum / U[i][i] : 0.0;
    }
    return x;
}

/* ---- Arnoldi process ---- */

void IterativeRefinement2::arnoldi(const QVector<QVector<double>>& A,
                                    QVector<QVector<double>>& Q,
                                    QVector<QVector<double>>& H,
                                    int step) const
{
    int n = Q[0].size();
    // q_{step+1} = A * q_step
    auto v = matVec(A, Q[step]);

    // Modified Gram-Schmidt
    for (int i = 0; i <= step; ++i) {
        double dot = 0.0;
        for (int j = 0; j < n; ++j)
            dot += v[j] * Q[i][j];
        H[i][step] = dot;
        for (int j = 0; j < n; ++j)
            v[j] -= dot * Q[i][j];
    }

    double norm = vecNorm(v);
    H[step + 1][step] = norm;
    if (norm > 1e-15) {
        for (int j = 0; j < n; ++j)
            Q[step + 1][j] = v[j] / norm;
    }
}

/* ---- GMRES inner solve ---- */

QVector<double> IterativeRefinement2::gmresSolve(const QVector<QVector<double>>& A,
                                                    const QVector<double>& rhs,
                                                    int maxIter) const
{
    int n = rhs.size();
    if (n == 0) return {};

    int m = qMin(maxIter, n);

    // Krylov basis
    QVector<QVector<double>> Q(m + 1, QVector<double>(n, 0.0));
    QVector<QVector<double>> H(m + 1, QVector<double>(m, 0.0));

    // Initial residual
    double beta = vecNorm(rhs);
    if (beta < 1e-15) return QVector<double>(n, 0.0);

    for (int j = 0; j < n; ++j)
        Q[0][j] = rhs[j] / beta;

    // Givens rotations storage
    QVector<double> cs(m, 0.0);
    QVector<double> sn(m, 0.0);
    QVector<double> g(m + 1, 0.0);
    g[0] = beta;

    for (int k = 0; k < m; ++k) {
        arnoldi(A, Q, H, k);

        // Apply previous Givens rotations
        for (int i = 0; i < k; ++i) {
            double temp = cs[i] * H[i][k] + sn[i] * H[i + 1][k];
            H[i + 1][k] = -sn[i] * H[i][k] + cs[i] * H[i + 1][k];
            H[i][k] = temp;
        }

        // New Givens rotation
        double r = qSqrt(H[k][k] * H[k][k] + H[k + 1][k] * H[k + 1][k]);
        if (r < 1e-15) break;
        cs[k] = H[k][k] / r;
        sn[k] = H[k + 1][k] / r;
        H[k][k] = r;
        H[k + 1][k] = 0.0;

        g[k + 1] = -sn[k] * g[k];
        g[k] = cs[k] * g[k];

        if (qAbs(g[k + 1]) < m_tol) { m = k + 1; break; }
    }

    // Back-solve upper triangular H * y = g
    QVector<double> y(m, 0.0);
    for (int i = m - 1; i >= 0; --i) {
        double sum = g[i];
        for (int j = i + 1; j < m; ++j)
            sum -= H[i][j] * y[j];
        y[i] = (qAbs(H[i][i]) > 1e-15) ? sum / H[i][i] : 0.0;
    }

    // Compute solution: x = Q_m * y
    QVector<double> x(n, 0.0);
    for (int j = 0; j < m; ++j)
        for (int i = 0; i < n; ++i)
            x[i] += Q[j][i] * y[j];

    return x;
}

/* ---- Solve ---- */

IterativeRefinement2::Result IterativeRefinement2::solve(
    const QVector<QVector<double>>& A,
    const QVector<double>& b,
    const QVector<double>& x0)
{
    QElapsedTimer timer;
    timer.start();

    Result result;
    int n = b.size();
    if (n == 0) { result.converged = false; return result; }

    QVector<double> x = x0;
    if (x.size() != n) x.fill(0.0, n);

    double initialRes = vecNorm(computeResidual(A, x, b));
    m_stats.initialResidual = initialRes;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        // High-precision residual
        auto r = computeResidual(A, x, b);
        double resNorm = vecNorm(r);

        emit refinementStep(iter, resNorm, timer.elapsed());

        if (resNorm < m_tol) {
            result.converged = true;
            break;
        }

        // GMRES inner correction: solve A * delta = r
        auto delta = gmresSolve(A, r, m_gmresIter);

        // Update solution: x = x + delta
        for (int i = 0; i < n; ++i)
            x[i] += delta[i];

        result.iterations = iter + 1;
    }

    result.solution = x;
    result.residualNorm = vecNorm(computeResidual(A, x, b));
    if (!result.converged && result.residualNorm < m_tol)
        result.converged = true;

    m_stats.matrixSize = n;
    m_stats.refinementSteps = result.iterations;
    m_stats.finalResidual = result.residualNorm;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit solveCompleted(result.iterations, result.residualNorm, timer.elapsed());

    return result;
}

/* ---- Reset ---- */

void IterativeRefinement2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
