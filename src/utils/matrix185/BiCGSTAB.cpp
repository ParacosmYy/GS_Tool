/**
 * @file BiCGSTAB.cpp
 * @brief BiCGSTAB 实现
 *
 * 实现BiCGSTAB稳定化双共轭梯度法：ILU(0)预处理、CSR稀疏存储、收敛监控。
 */

#include "utils/matrix185/BiCGSTAB.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

BiCGSTAB::BiCGSTAB(QObject *parent) : QObject(parent) {}
BiCGSTAB::~BiCGSTAB() = default;

/* ---- Configuration ---- */

void BiCGSTAB::setMaxIterations(int iter) { m_maxIter = qMax(10, iter); }
void BiCGSTAB::setTolerance(double tol) { m_tol = qMax(1e-15, tol); }
void BiCGSTAB::setILUEnabled(bool enabled) { m_iluEnabled = enabled; }

/* ---- Set matrix (COO -> CSR) ---- */

void BiCGSTAB::setMatrix(int n, const QVector<SparseEntry>& entries)
{
    m_n = n;
    // Sort entries by row then column
    auto sorted = entries;
    std::sort(sorted.begin(), sorted.end(),
              [](const SparseEntry& a, const SparseEntry& b) {
                  return (a.row < b.row) || (a.row == b.row && a.col < b.col);
              });

    // Build CSR
    m_rowPtr.resize(n + 1, 0);
    m_values.clear();
    m_colIdx.clear();

    for (const auto& e : sorted) {
        if (qFuzzyIsNull(e.value)) continue;
        m_values.append(e.value);
        m_colIdx.append(e.col);
        m_rowPtr[e.row + 1]++;
    }

    // Prefix sum for row pointers
    for (int i = 0; i < n; ++i)
        m_rowPtr[i + 1] += m_rowPtr[i];

    // Precompute ILU if enabled
    if (m_iluEnabled) computeILU0();
}

/* ---- ILU(0) ---- */

void BiCGSTAB::computeILU0()
{
    m_luValues = m_values;
    m_luColIdx = m_colIdx;
    m_luRowPtr = m_rowPtr;

    for (int i = 1; i < m_n; ++i) {
        for (int kk = m_luRowPtr[i]; kk < m_luRowPtr[i + 1]; ++kk) {
            int k = m_luColIdx[kk];
            if (k >= i) continue;

            // Find diagonal of row k
            double dkk = 0.0;
            for (int jj = m_luRowPtr[k]; jj < m_luRowPtr[k + 1]; ++jj) {
                if (m_luColIdx[jj] == k) { dkk = m_luValues[jj]; break; }
            }
            if (qFuzzyIsNull(dkk)) continue;

            m_luValues[kk] /= dkk;

            // Update remaining entries in row i
            for (int jj = kk + 1; jj < m_luRowPtr[i + 1]; ++jj) {
                int j = m_luColIdx[jj];
                // Find a[k,j]
                double akj = 0.0;
                for (int ll = m_luRowPtr[k]; ll < m_luRowPtr[k + 1]; ++ll) {
                    if (m_luColIdx[ll] == j) { akj = m_luValues[ll]; break; }
                }
                if (!qFuzzyIsNull(akj))
                    m_luValues[jj] -= m_luValues[kk] * akj;
            }
        }
    }
}

/* ---- ILU solve ---- */

QVector<double> BiCGSTAB::iluSolve(const QVector<double>& r) const
{
    int n = r.size();
    QVector<double> y(n, 0.0);
    QVector<double> x(n, 0.0);

    // Forward solve Ly = r (L has unit diagonal)
    for (int i = 0; i < n; ++i) {
        double sum = r[i];
        for (int jj = m_luRowPtr[i]; jj < m_luRowPtr[i + 1]; ++jj) {
            if (m_luColIdx[jj] < i)
                sum -= m_luValues[jj] * y[m_luColIdx[jj]];
        }
        y[i] = sum;
    }

    // Backward solve Ux = y
    for (int i = n - 1; i >= 0; --i) {
        double sum = y[i];
        double diag = 1.0;
        for (int jj = m_luRowPtr[i]; jj < m_luRowPtr[i + 1]; ++jj) {
            if (m_luColIdx[jj] > i)
                sum -= m_luValues[jj] * x[m_luColIdx[jj]];
            else if (m_luColIdx[jj] == i)
                diag = m_luValues[jj];
        }
        x[i] = sum / diag;
    }
    return x;
}

/* ---- Sparse matrix-vector multiply ---- */

QVector<double> BiCGSTAB::spmv(const QVector<double>& x) const
{
    QVector<double> y(m_n, 0.0);
    for (int i = 0; i < m_n; ++i) {
        double sum = 0.0;
        for (int jj = m_rowPtr[i]; jj < m_rowPtr[i + 1]; ++jj)
            sum += m_values[jj] * x[m_colIdx[jj]];
        y[i] = sum;
    }
    return y;
}

/* ---- Vector utilities ---- */

double BiCGSTAB::dot(const QVector<double>& a, const QVector<double>& b) const
{
    double s = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

QVector<double> BiCGSTAB::vecAdd(const QVector<double>& a, const QVector<double>& b) const
{
    int n = a.size();
    QVector<double> r(n);
    for (int i = 0; i < n; ++i) r[i] = a[i] + b[i];
    return r;
}

QVector<double> BiCGSTAB::vecSub(const QVector<double>& a, const QVector<double>& b) const
{
    int n = a.size();
    QVector<double> r(n);
    for (int i = 0; i < n; ++i) r[i] = a[i] - b[i];
    return r;
}

QVector<double> BiCGSTAB::vecScale(const QVector<double>& v, double s) const
{
    int n = v.size();
    QVector<double> r(n);
    for (int i = 0; i < n; ++i) r[i] = v[i] * s;
    return r;
}

double BiCGSTAB::vecNorm(const QVector<double>& v) const
{
    return qSqrt(dot(v, v));
}

/* ---- Residual ---- */

double BiCGSTAB::residual(const QVector<double>& x, const QVector<double>& b) const
{
    auto ax = spmv(x);
    auto r = vecSub(b, ax);
    return vecNorm(r);
}

/* ---- Solve ---- */

QVector<double> BiCGSTAB::solve(const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0 || b.size() != m_n) return {};

    QVector<double> x(m_n, 0.0);
    auto r0 = vecSub(b, spmv(x));
    auto r0hat = r0; // Shadow residual

    double rho = dot(r0, r0hat);
    double omega = 1.0;
    double alpha = 1.0;

    QVector<double> v(m_n, 0.0);
    QVector<double> p(m_n, 0.0);

    double bNorm = vecNorm(b);
    if (bNorm < 1e-30) bNorm = 1.0;

    bool converged = false;
    int iter = 0;

    for (iter = 0; iter < m_maxIter; ++iter) {
        double rhoNew = dot(r0, r0hat);
        if (qFuzzyIsNull(rhoNew)) break;

        double beta = (rhoNew / rho) * (alpha / omega);
        p = vecAdd(r0, vecScale(vecSub(p, vecScale(v, omega)), beta));

        // Apply preconditioner
        QVector<double> phat = m_iluEnabled ? iluSolve(p) : p;
        v = spmv(phat);
        alpha = rhoNew / dot(r0hat, v);

        QVector<double> s = vecSub(r0, vecScale(v, alpha));

        // Check early convergence
        if (vecNorm(s) / bNorm < m_tol) {
            x = vecAdd(x, vecScale(phat, alpha));
            converged = true;
            break;
        }

        QVector<double> shat = m_iluEnabled ? iluSolve(s) : s;
        QVector<double> t = spmv(shat);
        omega = dot(t, s) / dot(t, t);

        x = vecAdd(x, vecAdd(vecScale(phat, alpha), vecScale(shat, omega)));
        r0 = vecSub(s, vecScale(t, omega));

        rho = rhoNew;

        if (vecNorm(r0) / bNorm < m_tol) {
            converged = true;
            break;
        }
    }

    double finalRes = residual(x, b);

    m_stats.totalSolves++;
    m_stats.matrixSize = m_n;
    m_stats.numNonZeros = m_values.size();
    m_stats.numIterations = iter;
    m_stats.finalResidual = finalRes;
    m_stats.converged = converged;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(iter, finalRes, converged);
    return x;
}

/* ---- Reset ---- */

void BiCGSTAB::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
