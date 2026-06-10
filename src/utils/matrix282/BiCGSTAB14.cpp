/**
 * @file BiCGSTAB14.cpp
 * @brief BiCGSTAB14 实现
 *
 * 实现双共轭梯度稳定法：GPBi-CGS稳定变体与复合残差步的病态系统保证收敛求解器。
 */

#include "utils/matrix282/BiCGSTAB14.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

BiCGSTAB14::BiCGSTAB14(QObject *parent)
    : QObject(parent) {}

BiCGSTAB14::~BiCGSTAB14() = default;

/* ---- Configuration ---- */

void BiCGSTAB14::setTolerance(double tol) { m_tolerance = qBound(1e-15, tol, 1.0); }
void BiCGSTAB14::setMaxIterations(int iter) { m_maxIter = qBound(1, iter, 100000); }
void BiCGSTAB14::setCompositeStepFrequency(int freq) { m_compStepFreq = qBound(0, freq, 100); }

/* ---- Dot product ---- */

double BiCGSTAB14::dot(const QVector<double>& a, const QVector<double>& b) const
{
    double s = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

/* ---- Vector norm ---- */

double BiCGSTAB14::norm(const QVector<double>& v) const
{
    return qSqrt(dot(v, v));
}

/* ---- Sparse matrix-vector multiply ---- */

void BiCGSTAB14::spmv(int n, const QVector<int>& rowIdx, const QVector<int>& colIdx,
                       const QVector<double>& values, const QVector<double>& x,
                       QVector<double>& y) const
{
    y.fill(0.0, n);
    int nnz = values.size();
    for (int k = 0; k < nnz; ++k) {
        if (rowIdx[k] < n && colIdx[k] < x.size())
            y[rowIdx[k]] += values[k] * x[colIdx[k]];
    }
}

/* ---- Dense matrix-vector multiply ---- */

void BiCGSTAB14::denseMV(const QVector<QVector<double>>& A, const QVector<double>& x,
                           QVector<double>& y) const
{
    int n = A.size();
    y.fill(0.0, n);
    for (int i = 0; i < n; ++i) {
        int m = qMin(A[i].size(), x.size());
        for (int j = 0; j < m; ++j)
            y[i] += A[i][j] * x[j];
    }
}

/* ---- Solve with sparse matrix (BiCGSTAB + GPBi-CGS composite step) ---- */

BiCGSTAB14::SolveResult BiCGSTAB14::solve(
    int n, const QVector<int>& rowIdx, const QVector<int>& colIdx,
    const QVector<double>& values, const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    SolveResult result;
    if (n <= 0 || b.size() < n) return result;

    // Initial guess x = 0
    QVector<double> x(n, 0.0);
    QVector<double> r(n, 0.0);

    // r = b - A*x = b (since x=0)
    for (int i = 0; i < n; ++i) r[i] = b[i];

    double bNorm = norm(b);
    if (bNorm < 1e-30) {
        result.x = x;
        result.converged = true;
        return result;
    }

    result.initialResidual = norm(r);

    // Choose r_hat = r (shadow residual)
    QVector<double> rHat = r;
    double rho1 = 1.0, alpha = 1.0, omega = 1.0;

    QVector<double> v(n, 0.0), p(n, 0.0);

    for (int iter = 0; iter < m_maxIter; ++iter) {
        double rho = dot(rHat, r);
        if (qAbs(rho) < 1e-30) break;

        double beta = (rho / rho1) * (alpha / omega);

        // p = r + beta * (p - omega * v)
        for (int i = 0; i < n; ++i)
            p[i] = r[i] + beta * (p[i] - omega * v[i]);

        // v = A * p
        spmv(n, rowIdx, colIdx, values, p, v);

        alpha = rho / dot(rHat, v);
        if (qAbs(dot(rHat, v)) < 1e-30) break;

        // s = r - alpha * v
        QVector<double> s(n);
        for (int i = 0; i < n; ++i) s[i] = r[i] - alpha * v[i];

        // t = A * s
        QVector<double> t(n, 0.0);
        spmv(n, rowIdx, colIdx, values, s, t);

        omega = dot(t, s) / dot(t, t);
        if (qAbs(dot(t, t)) < 1e-30) omega = 1.0;

        // Update x and r
        for (int i = 0; i < n; ++i) {
            x[i] += alpha * p[i] + omega * s[i];
            r[i] = s[i] - omega * t[i];
        }

        rho1 = rho;
        double rNorm = norm(r);
        result.iterations = iter + 1;

        // Composite residual step (GPBi-CGS stabilization)
        if (m_compStepFreq > 0 && (iter + 1) % m_compStepFreq == 0) {
            // Recompute residual from scratch to avoid accumulation errors
            QVector<double> Ax(n, 0.0);
            spmv(n, rowIdx, colIdx, values, x, Ax);
            for (int i = 0; i < n; ++i) r[i] = b[i] - Ax[i];
            rNorm = norm(r);
        }

        emit iterationUpdate(iter + 1, rNorm);

        if (rNorm / bNorm < m_tolerance) {
            result.converged = true;
            break;
        }
    }

    result.x = x;
    result.finalResidual = norm(r);
    result.relativeResidual = result.finalResidual / bNorm;

    double elapsed = timer.elapsed();
    m_stats.matrixSize = n;
    m_stats.totalIterations += result.iterations;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit solveDone(result.iterations, result.finalResidual, result.converged, elapsed);

    return result;
}

/* ---- Solve with dense matrix ---- */

BiCGSTAB14::SolveResult BiCGSTAB14::solveDense(
    const QVector<QVector<double>>& A, const QVector<double>& b)
{
    int n = A.size();
    if (n == 0) return {};

    // Convert dense to COO sparse format
    QVector<int> rowIdx, colIdx;
    QVector<double> values;

    for (int i = 0; i < n; ++i) {
        int m = A[i].size();
        for (int j = 0; j < m; ++j) {
            if (qAbs(A[i][j]) > 1e-30) {
                rowIdx.append(i);
                colIdx.append(j);
                values.append(A[i][j]);
            }
        }
    }

    return solve(n, rowIdx, colIdx, values, b);
}

/* ---- Reset ---- */

void BiCGSTAB14::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
