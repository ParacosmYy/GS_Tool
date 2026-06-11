/**
 * @file BiCGSTAB16.cpp
 * @brief BiCGSTAB16 实现
 *
 * 实现BiCGSTAB求解器：多项式预处理与前瞻残差监控防止不定系统停滞。
 */

#include "utils/matrix296/BiCGSTAB16.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

BiCGSTAB16::BiCGSTAB16(QObject *parent)
    : QObject(parent) {}

BiCGSTAB16::~BiCGSTAB16() = default;

/* ---- Configuration ---- */

void BiCGSTAB16::setMaxIterations(int maxIter) { m_maxIter = qBound(10, maxIter, 100000); }
void BiCGSTAB16::setTolerance(double tol) { m_tol = qBound(1e-15, tol, 1.0); }
void BiCGSTAB16::setPreconditionDegree(int deg) { m_precondDegree = qBound(1, deg, 10); }

/* ---- Dot product ---- */

double BiCGSTAB16::dot(const QVector<double>& a, const QVector<double>& b) const
{
    double s = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i)
        s += a[i] * b[i];
    return s;
}

/* ---- Vector norm ---- */

double BiCGSTAB16::norm(const QVector<double>& v) const
{
    return qSqrt(dot(v, v));
}

/* ---- Sparse matrix-vector product ---- */

void BiCGSTAB16::spmv(int n, const QVector<double>& val,
                        const QVector<int>& col, const QVector<int>& row,
                        const QVector<double>& x, QVector<double>& y) const
{
    for (int i = 0; i < n; ++i) {
        double s = 0.0;
        for (int j = row[i]; j < row[i + 1]; ++j)
            s += val[j] * x[col[j]];
        y[i] = s;
    }
}

/* ---- Dense matrix-vector product ---- */

void BiCGSTAB16::denseMV(const QVector<QVector<double>>& A,
                            const QVector<double>& x, QVector<double>& y) const
{
    int n = A.size();
    for (int i = 0; i < n; ++i) {
        double s = 0.0;
        for (int j = 0; j < n; ++j)
            s += A[i][j] * x[j];
        y[i] = s;
    }
}

/* ---- Polynomial preconditioner: M^{-1} ≈ sum_{k=0}^{deg} (I - A)^k ---- */

void BiCGSTAB16::polynomialPrecondition(
    int n, const QVector<double>& val,
    const QVector<int>& col, const QVector<int>& row,
    const QVector<double>& r, QVector<double>& z) const
{
    // z = M^{-1} * r using Neumann polynomial: z = r + (I-A)*r + (I-A)^2*r + ...
    z = r; // k=0 term
    QVector<double> tmp(n, 0.0);
    QVector<double> prev = r;

    for (int k = 1; k <= m_precondDegree; ++k) {
        spmv(n, val, col, row, prev, tmp);
        for (int i = 0; i < n; ++i)
            tmp[i] = prev[i] - tmp[i]; // (I - A) * prev
        for (int i = 0; i < n; ++i)
            z[i] += tmp[i];
        prev = tmp;
    }
}

/* ---- Solve sparse system ---- */

BiCGSTAB16::SolveResult BiCGSTAB16::solve(
    int n, const QVector<double>& values,
    const QVector<int>& colIdx, const QVector<int>& rowPtr,
    const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    SolveResult result;
    result.solution.resize(n, 0.0);

    double rhsNorm = norm(rhs);
    if (rhsNorm < 1e-30) { result.converged = true; return result; }

    // r0 = b - A*x0
    QVector<double> r(n), Ax(n);
    spmv(n, values, colIdx, rowPtr, result.solution, Ax);
    for (int i = 0; i < n; ++i)
        r[i] = rhs[i] - Ax[i];

    QVector<double> r0hat = r; // Shadow residual
    double rho = 1.0, alpha = 1.0, omega = 1.0;
    QVector<double> v(n, 0.0), p(n, 0.0);

    // Look-ahead stagnation detection
    double prevResidual = norm(r);
    int stagnationCount = 0;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        double rhoPrev = rho;
        rho = dot(r0hat, r);

        if (qAbs(rho) < 1e-30) {
            result.stagnated = true;
            break;
        }

        // Beta computation
        double beta = (rho / rhoPrev) * (alpha / omega);

        // p = r + beta * (p - omega * v)
        for (int i = 0; i < n; ++i)
            p[i] = r[i] + beta * (p[i] - omega * v[i]);

        // Apply polynomial preconditioner: y = M^{-1} * p
        QVector<double> y(n);
        polynomialPrecondition(n, values, colIdx, rowPtr, p, y);

        // v = A * y
        spmv(n, values, colIdx, rowPtr, y, v);

        alpha = rho / dot(r0hat, v);
        if (qAbs(alpha) > 1e30) {
            result.stagnated = true;
            break;
        }

        // s = r - alpha * v
        QVector<double> s(n);
        for (int i = 0; i < n; ++i)
            s[i] = r[i] - alpha * v[i];

        // Check for early convergence
        double sNorm = norm(s);
        if (sNorm / rhsNorm < m_tol) {
            for (int i = 0; i < n; ++i)
                result.solution[i] += alpha * y[i];
            result.converged = true;
            result.iterations = iter + 1;
            break;
        }

        // Apply preconditioner: z = M^{-1} * s
        QVector<double> z(n);
        polynomialPrecondition(n, values, colIdx, rowPtr, s, z);

        // t = A * z
        QVector<double> t(n);
        spmv(n, values, colIdx, rowPtr, z, t);

        // omega = (t, s) / (t, t)
        omega = dot(t, s) / dot(t, t);
        if (qAbs(omega) < 1e-30) {
            result.stagnated = true;
            break;
        }

        // Update solution: x += alpha*y + omega*z
        for (int i = 0; i < n; ++i)
            result.solution[i] += alpha * y[i] + omega * z[i];

        // Update residual: r = s - omega*t
        for (int i = 0; i < n; ++i)
            r[i] = s[i] - omega * t[i];

        // Look-ahead stagnation detection
        double curRes = norm(r);
        if (curRes / rhsNorm < m_tol) {
            result.converged = true;
            result.iterations = iter + 1;
            break;
        }

        if (qAbs(curRes - prevResidual) / qMax(curRes, 1e-30) < 1e-12)
            stagnationCount++;
        else
            stagnationCount = 0;

        if (stagnationCount > 10) {
            result.stagnated = true;
            result.iterations = iter + 1;
            break;
        }
        prevResidual = curRes;
        result.iterations = iter + 1;
    }

    // Final residual
    QVector<double> finalR(n);
    spmv(n, values, colIdx, rowPtr, result.solution, finalR);
    for (int i = 0; i < n; ++i)
        finalR[i] = rhs[i] - finalR[i];
    result.residualNorm = norm(finalR);

    double elapsed = timer.elapsed();
    m_stats.problemSize = n;
    m_stats.totalSolves++;
    m_iterSum += result.iterations;
    m_stats.avgIterations = m_iterSum / m_stats.totalSolves;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveDone(n, result.iterations, result.residualNorm, elapsed);
    return result;
}

/* ---- Solve dense system ---- */

BiCGSTAB16::SolveResult BiCGSTAB16::solveDense(
    const QVector<QVector<double>>& A, const QVector<double>& b)
{
    int n = A.size();
    // Convert dense to CSR
    QVector<double> values;
    QVector<int> colIdx, rowPtr(n + 1, 0);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (qAbs(A[i][j]) > 1e-30) {
                values.append(A[i][j]);
                colIdx.append(j);
            }
        }
        rowPtr[i + 1] = values.size();
    }
    return solve(n, values, colIdx, rowPtr, b);
}

/* ---- Reset ---- */

void BiCGSTAB16::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_iterSum = 0.0;
}
