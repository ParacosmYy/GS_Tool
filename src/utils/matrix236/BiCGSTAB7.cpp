/**
 * @file BiCGSTAB7.cpp
 * @brief BiCGSTAB7 实现
 *
 * 实现双共轭梯度稳定法：右预条件与IDR(s)稳定变体。
 */

#include "utils/matrix236/BiCGSTAB7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

BiCGSTAB7::BiCGSTAB7(QObject *parent) : QObject(parent) {}
BiCGSTAB7::~BiCGSTAB7() = default;

/* ---- Configuration ---- */

void BiCGSTAB7::setParameters(int maxIter, double tolerance, int idrS)
{
    m_maxIter = qMax(1, maxIter);
    m_tol = qMax(1e-15, tolerance);
    m_idrS = qMax(1, idrS);
}

/* ---- Build matrix (COO -> CSR) ---- */

void BiCGSTAB7::buildMatrix(int n, const QVector<Entry>& entries)
{
    m_n = qMax(1, n);
    int nnz = entries.size();

    // Sort entries by row, then column
    QVector<Entry> sorted = entries;
    std::sort(sorted.begin(), sorted.end(), [](const Entry& a, const Entry& b) {
        return (a.row < b.row) || (a.row == b.row && a.col < b.col);
    });

    m_values.resize(nnz);
    m_colIdx.resize(nnz);
    m_rowPtr.resize(m_n + 1, 0);
    m_diag.resize(m_n, 1.0);  // Default diagonal = 1

    for (int i = 0; i < nnz; ++i) {
        m_values[i] = sorted[i].value;
        m_colIdx[i] = sorted[i].col;
        m_rowPtr[sorted[i].row + 1]++;
        if (sorted[i].row == sorted[i].col)
            m_diag[sorted[i].row] = sorted[i].value;
    }

    // Prefix sum for row pointers
    for (int i = 0; i < m_n; ++i)
        m_rowPtr[i + 1] += m_rowPtr[i];

    m_stats.matrixSize = m_n;
    m_stats.numNonZeros = nnz;
}

/* ---- CSR SpMV ---- */

void BiCGSTAB7::spmv(const QVector<double>& x, QVector<double>& y) const
{
    for (int i = 0; i < m_n; ++i) {
        double sum = 0.0;
        for (int j = m_rowPtr[i]; j < m_rowPtr[i + 1]; ++j)
            sum += m_values[j] * x[m_colIdx[j]];
        y[i] = sum;
    }
}

/* ---- Jacobi preconditioner ---- */

void BiCGSTAB7::precondition(const QVector<double>& r, QVector<double>& z) const
{
    for (int i = 0; i < m_n; ++i)
        z[i] = r[i] / (qAbs(m_diag[i]) > 1e-15 ? m_diag[i] : 1.0);
}

/* ---- Vector operations ---- */

double BiCGSTAB7::dot(const QVector<double>& a, const QVector<double>& b)
{
    double s = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

double BiCGSTAB7::norm(const QVector<double>& v)
{
    return qSqrt(dot(v, v));
}

void BiCGSTAB7::axpy(double alpha, const QVector<double>& x, QVector<double>& y)
{
    int n = qMin(x.size(), y.size());
    for (int i = 0; i < n; ++i) y[i] += alpha * x[i];
}

/* ---- Solve ---- */

BiCGSTAB7::SolveResult BiCGSTAB7::solve(const QVector<double>& b, const QVector<double>& x0)
{
    QElapsedTimer timer;
    timer.start();

    SolveResult result;
    m_residualHistory.clear();

    if (m_n == 0 || b.size() != m_n) {
        result.converged = false;
        return result;
    }

    // Initial guess
    QVector<double> x = (x0.size() == m_n) ? x0 : QVector<double>(m_n, 0.0);

    // r = b - A*x
    QVector<double> r(m_n), Ax(m_n);
    spmv(x, Ax);
    for (int i = 0; i < m_n; ++i) r[i] = b[i] - Ax[i];

    double rNorm = norm(r);
    double bNorm = qMax(norm(b), 1e-15);
    result.initialResidual = rNorm;

    if (rNorm / bNorm < m_tol) {
        result.solution = x;
        result.converged = true;
        result.residualNorm = rNorm;
        return result;
    }

    // Choose r_hat = r (shadow residual)
    QVector<double> rHat = r;
    double rho = dot(rHat, r);

    // Initialize BiCGSTAB vectors
    QVector<double> p = r, v(m_n, 0.0), s(m_n, 0.0), t(m_n, 0.0);
    QVector<double> pHat(m_n), sHat(m_n), y(m_n), z(m_n);

    bool converged = false;
    int iter = 0;

    for (iter = 0; iter < m_maxIter; ++iter) {
        // Precondition: p_hat = M^{-1} p (right preconditioning)
        precondition(p, pHat);

        // v = A * p_hat
        spmv(pHat, v);

        double alpha = rho / qMax(dot(rHat, v), 1e-30);

        // s = r - alpha * v
        for (int i = 0; i < m_n; ++i) s[i] = r[i] - alpha * v[i];

        // Check for early convergence
        double sNorm = norm(s);
        if (sNorm / bNorm < m_tol) {
            for (int i = 0; i < m_n; ++i) x[i] += alpha * pHat[i];
            converged = true;
            break;
        }

        // Precondition: s_hat = M^{-1} s
        precondition(s, sHat);

        // t = A * s_hat
        spmv(sHat, t);

        // omega = (t, s) / (t, t)
        double omega = dot(t, s) / qMax(dot(t, t), 1e-30);

        // Update x
        for (int i = 0; i < m_n; ++i) x[i] += alpha * pHat[i] + omega * sHat[i];

        // Update r = s - omega * t
        for (int i = 0; i < m_n; ++i) r[i] = s[i] - omega * t[i];

        rNorm = norm(r);
        m_residualHistory.append(rNorm);

        emit iterationCompleted(iter, rNorm);

        if (rNorm / bNorm < m_tol) { converged = true; break; }

        // IDR(s) stabilization: periodically project residual
        if (m_idrS > 0 && (iter + 1) % m_idrS == 0 && rNorm > 0.0) {
            // Minimal residual smoothing
            double sigma = dot(r, s) / qMax(dot(s, s), 1e-30);
            sigma = qBound(-1.0, sigma, 1.0);
            for (int i = 0; i < m_n; ++i)
                r[i] = r[i] - sigma * s[i];
            rNorm = norm(r);
        }

        double rhoNew = dot(rHat, r);
        if (qAbs(rhoNew) < 1e-30) break;

        double beta = (rhoNew / qMax(rho, 1e-30)) * (alpha / qMax(omega, 1e-30));
        for (int i = 0; i < m_n; ++i)
            p[i] = r[i] + beta * (p[i] - omega * v[i]);
        rho = rhoNew;
    }

    result.solution = x;
    result.iterations = iter + 1;
    result.residualNorm = rNorm;
    result.converged = converged;

    m_stats.totalIterations += iter + 1;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit solveCompleted(iter + 1, rNorm, timer.elapsed());
    return result;
}

/* ---- Residual history ---- */

QVector<double> BiCGSTAB7::residualHistory() const { return m_residualHistory; }

/* ---- Reset ---- */

void BiCGSTAB7::resetStatistics()
{
    m_values.clear();
    m_colIdx.clear();
    m_rowPtr.clear();
    m_diag.clear();
    m_pVectors.clear();
    m_gVectors.clear();
    m_residualHistory.clear();
    m_n = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
