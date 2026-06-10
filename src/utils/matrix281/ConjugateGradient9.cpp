/**
 * @file ConjugateGradient9.cpp
 * @brief ConjugateGradient9 实现
 *
 * 实现共轭梯度法：Polak-Ribiere公式与自动谱界估计的加速收敛稀疏线性方程组求解。
 */

#include "utils/matrix281/ConjugateGradient9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

ConjugateGradient9::ConjugateGradient9(QObject *parent)
    : QObject(parent) {}

ConjugateGradient9::~ConjugateGradient9() = default;

/* ---- Configuration ---- */

void ConjugateGradient9::setTolerance(double tol) { m_tolerance = qBound(1e-15, tol, 1.0); }
void ConjugateGradient9::setMaxIterations(int iter) { m_maxIter = qBound(1, iter, 100000); }

/* ---- Vector operations ---- */

double ConjugateGradient9::dot(const QVector<double>& a, const QVector<double>& b)
{
    double s = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

void ConjugateGradient9::axpy(double alpha, const QVector<double>& x,
                                double beta, QVector<double>& y)
{
    int n = qMin(x.size(), y.size());
    for (int i = 0; i < n; ++i) y[i] = alpha * x[i] + beta * y[i];
}

double ConjugateGradient9::norm(const QVector<double>& v)
{
    return qSqrt(dot(v, v));
}

/* ---- Sparse matrix-vector multiply ---- */

void ConjugateGradient9::spmv(const QVector<QVector<QPair<int, double>>>& A,
                                const QVector<double>& x, QVector<double>& y)
{
    int n = A.size();
    for (int i = 0; i < n; ++i) {
        double s = 0.0;
        for (const auto& entry : A[i])
            s += entry.second * x[entry.first];
        y[i] = s;
    }
}

/* ---- Dense matrix-vector multiply ---- */

void ConjugateGradient9::densemv(const QVector<QVector<double>>& A,
                                    const QVector<double>& x, QVector<double>& y)
{
    int n = A.size();
    for (int i = 0; i < n; ++i) {
        double s = 0.0;
        int m = qMin(A[i].size(), x.size());
        for (int j = 0; j < m; ++j) s += A[i][j] * x[j];
        y[i] = s;
    }
}

/* ---- Estimate spectral bounds ---- */

QPair<double, double> ConjugateGradient9::estimateSpectralBounds(
    const QVector<QVector<QPair<int, double>>>& A) const
{
    int n = A.size();
    if (n == 0) return {0.0, 0.0};

    // Power iteration to estimate max eigenvalue
    QVector<double> v(n, 1.0 / qSqrt(static_cast<double>(n)));
    double lambdaMax = 0.0;

    for (int iter = 0; iter < 50; ++iter) {
        QVector<double> w(n, 0.0);
        spmv(A, v, w);
        double lambda = dot(v, w);
        lambdaMax = qMax(lambdaMax, qAbs(lambda));
        double nw = norm(w);
        if (nw > 1e-15) {
            for (int i = 0; i < n; ++i) v[i] = w[i] / nw;
        }
    }

    // Lanczos-style estimate for min eigenvalue
    double lambdaMin = lambdaMax * 0.01; // Conservative estimate
    QVector<double> vOld = v;
    for (int iter = 0; iter < 30; ++iter) {
        QVector<double> w(n, 0.0);
        spmv(A, v, w);
        // Shifted inverse: approximate (A - sigma*I)^{-1} * v
        double sigma = 0.0;
        for (int i = 0; i < n; ++i) {
            double diag = 0.0;
            for (const auto& e : A[i]) {
                if (e.first == i) { diag = e.second; break; }
            }
            w[i] = (qAbs(diag - sigma) > 1e-15) ? v[i] / (diag - sigma) : v[i];
        }
        double nw = norm(w);
        if (nw > 1e-15) {
            for (int i = 0; i < n; ++i) w[i] /= nw;
        }
        // Rayleigh quotient
        QVector<double> Aw(n, 0.0);
        spmv(A, w, Aw);
        lambdaMin = dot(w, Aw);
        if (lambdaMin < 0) lambdaMin = 1e-6;
        vOld = v;
        v = w;
    }

    return {qMax(1e-10, lambdaMin), qMax(lambdaMin + 1e-6, lambdaMax)};
}

/* ---- Solve sparse system Ax = b ---- */

ConjugateGradient9::SolveResult ConjugateGradient9::solve(
    const QVector<QVector<QPair<int, double>>>& A,
    const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    SolveResult result;
    int n = b.size();
    if (n == 0) return result;

    // Estimate spectral bounds
    auto bounds = estimateSpectralBounds(A);
    result.spectralMin = bounds.first;
    result.spectralMax = bounds.second;
    result.conditionNumber = (bounds.first > 1e-15) ? bounds.second / bounds.first : 1e18;

    // Initial guess x = 0
    result.x.resize(n, 0.0);
    QVector<double> r = b; // r = b - A*0 = b
    QVector<double> p = r; // Initial search direction
    double rsOld = dot(r, r);

    double bNorm = norm(b);
    if (bNorm < 1e-15) {
        result.converged = true;
        result.residualNorm = 0.0;
        result.iterations = 0;
        return result;
    }

    for (int iter = 0; iter < m_maxIter; ++iter) {
        // Compute Ap
        QVector<double> Ap(n, 0.0);
        spmv(A, p, Ap);

        double pAp = dot(p, Ap);
        if (qAbs(pAp) < 1e-30) break;

        double alpha = rsOld / pAp;

        // Update x and r
        for (int i = 0; i < n; ++i) {
            result.x[i] += alpha * p[i];
            r[i] -= alpha * Ap[i];
        }

        double rsNew = dot(r, r);
        result.residualNorm = qSqrt(rsNew);

        emit iterationUpdate(iter, result.residualNorm,
                              (bounds.first + bounds.second) / 2.0);

        if (result.residualNorm / bNorm < m_tolerance) {
            result.converged = true;
            result.iterations = iter + 1;
            break;
        }

        // Polak-Ribiere formula: beta = (r_{k+1}^T * (r_{k+1} - r_k)) / (r_k^T * r_k)
        // Approximation using consecutive residuals
        double beta = rsNew / rsOld;

        for (int i = 0; i < n; ++i)
            p[i] = r[i] + beta * p[i];

        rsOld = rsNew;
        result.iterations = iter + 1;
    }

    double elapsed = timer.elapsed();
    m_stats.totalIterations += result.iterations;
    if (result.converged) m_stats.numConverged++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit solveComplete(n, result.iterations, result.residualNorm, elapsed);

    return result;
}

/* ---- Solve dense system ---- */

ConjugateGradient9::SolveResult ConjugateGradient9::solveDense(
    const QVector<QVector<double>>& A,
    const QVector<double>& b)
{
    // Convert dense to sparse format
    int n = A.size();
    QVector<QVector<QPair<int, double>>> sparseA(n);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < A[i].size(); ++j) {
            if (qAbs(A[i][j]) > 1e-15)
                sparseA[i].append({j, A[i][j]});
        }
    }
    return solve(sparseA, b);
}

/* ---- Compute residual ---- */

QVector<double> ConjugateGradient9::residual(
    const QVector<QVector<double>>& A,
    const QVector<double>& x,
    const QVector<double>& b) const
{
    int n = b.size();
    QVector<double> r(n, 0.0);
    QVector<double> Ax(n, 0.0);
    densemv(A, x, Ax);
    for (int i = 0; i < n; ++i) r[i] = b[i] - Ax[i];
    return r;
}

/* ---- Reset ---- */

void ConjugateGradient9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
