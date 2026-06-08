/**
 * @file GMRES3.cpp
 * @brief GMRES3 实现
 *
 * 实现GMRES迭代求解器：重启Arnoldi、Givens旋转、泄放重启。
 */

#include "utils/matrix218/GMRES3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <functional>

/* ---- Construction / Destruction ---- */

GMRES3::GMRES3(QObject *parent) : QObject(parent) {}
GMRES3::~GMRES3() = default;

/* ---- Configuration ---- */

void GMRES3::setParameters(int krylovDim, double tolerance, int maxRestarts)
{
    m_krylovDim = qMax(2, krylovDim);
    m_tol = qMax(1e-15, tolerance);
    m_maxRestarts = qMax(1, maxRestarts);
}

/* ---- Dot product ---- */

double GMRES3::dot(const QVector<double>& a, const QVector<double>& b)
{
    double s = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

/* ---- Vector norm ---- */

double GMRES3::norm(const QVector<double>& v)
{
    return qSqrt(dot(v, v));
}

/* ---- Back-solve upper triangular ---- */

QVector<double> GMRES3::backSolve(const QVector<QVector<double>>& R,
                                   const QVector<double>& rhs, int size)
{
    QVector<double> x(size, 0.0);
    for (int i = size - 1; i >= 0; --i) {
        x[i] = rhs[i];
        for (int j = i + 1; j < size; ++j)
            x[i] -= R[i][j] * x[j];
        if (qAbs(R[i][i]) > 1e-15)
            x[i] /= R[i][i];
    }
    return x;
}

/* ---- Arnoldi step ---- */

void GMRES3::arnoldiStep(QVector<QVector<double>>& V,
                          QVector<QVector<double>>& H,
                          int step, int n,
                          const std::function<QVector<double>(const QVector<double>&)>& matvec) const
{
    // w = A * V[step]
    QVector<double> w = matvec(V[step]);

    // Modified Gram-Schmidt orthogonalization
    for (int i = 0; i <= step; ++i) {
        H[i][step] = dot(w, V[i]);
        for (int j = 0; j < n; ++j)
            w[j] -= H[i][step] * V[i][j];
    }
    double wNorm = norm(w);
    H[step + 1][step] = wNorm;

    if (wNorm > 1e-15) {
        V[step + 1].resize(n);
        for (int j = 0; j < n; ++j)
            V[step + 1][j] = w[j] / wNorm;
    }
}

/* ---- Apply Givens rotations ---- */

void GMRES3::applyGivens(QVector<QVector<double>>& H,
                          QVector<double>& cosRot,
                          QVector<double>& sinRot,
                          int step) const
{
    for (int i = 0; i < step; ++i) {
        double temp = cosRot[i] * H[i][step] + sinRot[i] * H[i + 1][step];
        H[i + 1][step] = -sinRot[i] * H[i][step] + cosRot[i] * H[i + 1][step];
        H[i][step] = temp;
    }

    // Compute new rotation
    double r = qSqrt(H[step][step] * H[step][step] +
                     H[step + 1][step] * H[step + 1][step]);
    if (r < 1e-15) return;

    cosRot[step] = H[step][step] / r;
    sinRot[step] = H[step + 1][step] / r;

    H[step][step] = r;
    H[step + 1][step] = 0.0;
}

/* ---- Compute residual ---- */

double GMRES3::computeResidual(const QVector<QVector<double>>& H,
                                const QVector<double>& g, int step) const
{
    // Residual = |beta * e1 - H*y| after rotations
    double res = 0.0;
    for (int i = step; i <= step + 1 && i < g.size(); ++i)
        res += g[i] * g[i];
    return qSqrt(res);
}

/* ---- Solve sparse-friendly ---- */

QVector<double> GMRES3::solveSparse(
    const QVector<double>& b, int n,
    std::function<QVector<double>(const QVector<double>&)> matvec) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> x(n, 0.0);
    double beta = norm(b);
    if (beta < 1e-15) return x;

    int restarts = 0;
    int totalIter = 0;

    while (restarts < m_maxRestarts) {
        // Compute initial residual r = b - A*x
        QVector<double> r = matvec(x);
        for (int i = 0; i < n; ++i) r[i] = b[i] - r[i];

        double resNorm = norm(r);
        if (resNorm < m_tol * beta) break;

        // Initialize Arnoldi basis V[0] = r / ||r||
        QVector<QVector<double>> V(m_krylovDim + 1);
        V[0].resize(n);
        for (int i = 0; i < n; ++i) V[0][i] = r[i] / resNorm;

        // Hessenberg matrix
        QVector<QVector<double>> H(m_krylovDim + 1,
                                    QVector<double>(m_krylovDim, 0.0));
        QVector<double> g(m_krylovDim + 1, 0.0);
        g[0] = resNorm;
        QVector<double> cosRot(m_krylovDim, 0.0);
        QVector<double> sinRot(m_krylovDim, 0.0);

        int j = 0;
        for (; j < m_krylovDim; ++j) {
            arnoldiStep(V, H, j, n, matvec);
            applyGivens(H, cosRot, sinRot, j);

            // Update g vector
            g[j + 1] = -sinRot[j] * g[j];
            g[j] = cosRot[j] * g[j];

            totalIter++;
            double res = qAbs(g[j + 1]);
            if (res < m_tol * beta) {
                j++;
                break;
            }
        }

        // Back-solve for y
        QVector<double> y = backSolve(H, g, j);

        // Update x = x + V * y
        for (int k = 0; k < j; ++k)
            for (int i = 0; i < n; ++i)
                x[i] += V[k][i] * y[k];

        restarts++;
    }

    GMRES3* self = const_cast<GMRES3*>(this);
    self->m_stats.problemSize = n;
    self->m_stats.krylovDim = m_krylovDim;
    self->m_stats.iterations = totalIter;
    self->m_stats.restarts = restarts;
    self->m_stats.residualNorm = norm(matvec(x));
    self->m_stats.totalSolves++;
    self->m_timeSum += timer.elapsed();
    self->m_stats.avgProcessingTimeMs = self->m_timeSum / self->m_stats.totalSolves;
    emit self->solveCompleted(totalIter, m_stats.residualNorm, timer.elapsed());

    return x;
}

/* ---- Solve dense ---- */

QVector<double> GMRES3::solve(const QVector<QVector<double>>& A,
                               const QVector<double>& b)
{
    int n = b.size();
    auto matvec = [&A, n](const QVector<double>& x) -> QVector<double> {
        QVector<double> y(n, 0.0);
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                y[i] += A[i][j] * x[j];
        return y;
    };

    return solveSparse(b, n, matvec);
}

/* ---- Reset ---- */

void GMRES3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_deflatedVectors.clear();
}
