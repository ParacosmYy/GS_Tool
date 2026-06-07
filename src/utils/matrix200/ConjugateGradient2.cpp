/**
 * @file ConjugateGradient2.cpp
 * @brief ConjugateGradient2 实现
 *
 * 实现预条件共轭梯度：IC(0)不完全Cholesky、FCG柔性变体、收敛监控。
 */

#include "utils/matrix200/ConjugateGradient2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

ConjugateGradient2::ConjugateGradient2(QObject *parent) : QObject(parent) {}
ConjugateGradient2::~ConjugateGradient2() = default;

/* ---- Configuration ---- */

void ConjugateGradient2::setMaxIterations(int iter) { m_maxIter = qMax(1, iter); }
void ConjugateGradient2::setTolerance(double tol) { m_tolerance = qMax(1e-15, tol); }
void ConjugateGradient2::setMode(Mode mode) { m_mode = mode; }

/* ---- Build IC(0) preconditioner ---- */

void ConjugateGradient2::buildPreconditioner(const QVector<QVector<double>>& A)
{
    int n = A.size();
    m_L = QVector<QVector<double>>(n, QVector<double>(n, 0.0));
    m_diag.resize(n, 0.0);

    // IC(0): incomplete Cholesky with same sparsity as A
    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int k = 0; k < i; ++k) {
            if (qAbs(A[i][k]) < 1e-15) continue; // maintain sparsity
            sum += m_L[i][k] * m_L[i][k];
        }
        double d = A[i][i] - sum;
        if (d <= 0.0) d = 1e-10; // regularize
        m_L[i][i] = qSqrt(d);
        m_diag[i] = m_L[i][i];

        for (int j = i + 1; j < n; ++j) {
            if (qAbs(A[j][i]) < 1e-15) continue; // maintain sparsity
            double s = 0.0;
            for (int k = 0; k < i; ++k)
                s += m_L[j][k] * m_L[i][k];
            m_L[j][i] = (A[j][i] - s) / m_L[i][i];
        }
    }
}

/* ---- Forward solve L*y = b ---- */

QVector<double> ConjugateGradient2::forwardSolve(const QVector<double>& b) const
{
    int n = b.size();
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double s = b[i];
        for (int j = 0; j < i; ++j)
            s -= m_L[i][j] * y[j];
        y[i] = s / qMax(m_diag[i], 1e-15);
    }
    return y;
}

/* ---- Backward solve L^T*x = y ---- */

QVector<double> ConjugateGradient2::backwardSolve(const QVector<double>& y) const
{
    int n = y.size();
    QVector<double> x(n, 0.0);
    for (int i = n - 1; i >= 0; --i) {
        double s = y[i];
        for (int j = i + 1; j < n; ++j)
            s -= m_L[j][i] * x[j];
        x[i] = s / qMax(m_diag[i], 1e-15);
    }
    return x;
}

/* ---- Apply preconditioner ---- */

QVector<double> ConjugateGradient2::applyPreconditioner(const QVector<double>& r) const
{
    auto y = forwardSolve(r);
    return backwardSolve(y);
}

/* ---- SpMV ---- */

QVector<double> ConjugateGradient2::spmv(const QVector<QVector<double>>& A,
                                          const QVector<double>& x)
{
    int n = A.size();
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < qMin(A[i].size(), n); ++j)
            y[i] += A[i][j] * x[j];
    return y;
}

/* ---- Dot product ---- */

double ConjugateGradient2::dot(const QVector<double>& a, const QVector<double>& b)
{
    double s = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

/* ---- Compute residual ---- */

double ConjugateGradient2::computeResidual(const QVector<QVector<double>>& A,
                                            const QVector<double>& x,
                                            const QVector<double>& b)
{
    auto Ax = spmv(A, x);
    double norm = 0.0;
    for (int i = 0; i < b.size(); ++i) {
        double r = b[i] - (i < Ax.size() ? Ax[i] : 0.0);
        norm += r * r;
    }
    return qSqrt(norm);
}

/* ---- Solve ---- */

QVector<double> ConjugateGradient2::solve(const QVector<QVector<double>>& A,
                                           const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    int n = b.size();
    if (n == 0) return {};

    // Build preconditioner
    buildPreconditioner(A);

    m_residuals.clear();
    QVector<double> x(n, 0.0);

    // r = b - A*x
    auto Ax = spmv(A, x);
    QVector<double> r(n);
    for (int i = 0; i < n; ++i) r[i] = b[i] - (i < Ax.size() ? Ax[i] : 0.0);

    if (m_mode == StandardPCG) {
        // Standard PCG
        auto z = applyPreconditioner(r);
        QVector<double> p = z;
        double rzOld = dot(r, z);

        for (int it = 0; it < m_maxIter; ++it) {
            auto Ap = spmv(A, p);
            double pAp = dot(p, Ap);
            if (qAbs(pAp) < 1e-30) break;
            double alpha = rzOld / pAp;

            for (int i = 0; i < n; ++i) {
                x[i] += alpha * p[i];
                r[i] -= alpha * Ap[i];
            }

            double rNorm = qSqrt(dot(r, r));
            m_residuals.append(rNorm);
            if (rNorm < m_tolerance) break;

            z = applyPreconditioner(r);
            double rzNew = dot(r, z);
            double beta = rzNew / qMax(rzOld, 1e-30);
            for (int i = 0; i < n; ++i)
                p[i] = z[i] + beta * p[i];
            rzOld = rzNew;
        }
    } else {
        // Flexible CG (FCG)
        QVector<double> p = applyPreconditioner(r);
        double rzOld = dot(r, p);

        for (int it = 0; it < m_maxIter; ++it) {
            auto Ap = spmv(A, p);
            double pAp = dot(p, Ap);
            if (qAbs(pAp) < 1e-30) break;
            double alpha = rzOld / pAp;

            QVector<double> zOld = p; // store old preconditioned direction
            for (int i = 0; i < n; ++i) {
                x[i] += alpha * p[i];
                r[i] -= alpha * Ap[i];
            }

            double rNorm = qSqrt(dot(r, r));
            m_residuals.append(rNorm);
            if (rNorm < m_tolerance) break;

            auto z = applyPreconditioner(r);
            // Flexible update: use stored old direction
            double rzNew = dot(r, z);
            double beta = rzNew / qMax(rzOld, 1e-30);
            for (int i = 0; i < n; ++i)
                p[i] = z[i] + beta * zOld[i];
            rzOld = rzNew;
        }
    }

    double res = computeResidual(A, x, b);
    m_stats.totalSolves++;
    m_stats.matrixSize = n;
    m_stats.iterations = m_residuals.size();
    m_stats.residual = res;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(m_stats.iterations, res, timer.elapsed());
    return x;
}

/* ---- Solve sparse ---- */

QVector<double> ConjugateGradient2::solveSparse(const QVector<int>& rows,
                                                 const QVector<int>& cols,
                                                 const QVector<double>& vals,
                                                 int n,
                                                 const QVector<double>& b)
{
    // Convert to dense for internal solve
    QVector<QVector<double>> A(n, QVector<double>(n, 0.0));
    for (int i = 0; i < rows.size(); ++i) {
        int r = rows[i], c = cols[i];
        if (r >= 0 && r < n && c >= 0 && c < n)
            A[r][c] = vals[i];
    }
    return solve(A, b);
}

/* ---- Convergence history ---- */

QVector<double> ConjugateGradient2::convergenceHistory() const { return m_residuals; }

/* ---- Reset ---- */

void ConjugateGradient2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_L.clear();
    m_diag.clear();
    m_residuals.clear();
}
