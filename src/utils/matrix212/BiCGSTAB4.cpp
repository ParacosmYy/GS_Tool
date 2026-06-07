/**
 * @file BiCGSTAB4.cpp
 * @brief BiCGSTAB4 实现
 *
 * 实现BiCGSTAB：GPBi-CG变体、SAINV预条件器、稀疏矩阵求解。
 */

#include "utils/matrix212/BiCGSTAB4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

BiCGSTAB4::BiCGSTAB4(QObject *parent) : QObject(parent) {}
BiCGSTAB4::~BiCGSTAB4() = default;

/* ---- Configuration ---- */

void BiCGSTAB4::setMaxIterations(int maxIter) { m_maxIter = qMax(1, maxIter); }
void BiCGSTAB4::setTolerance(double tol) { m_tol = qMax(1e-15, tol); }
void BiCGSTAB4::setUseGPBiCG(bool enable) { m_useGPBiCG = enable; }

/* ---- Dot product ---- */

double BiCGSTAB4::dot(const QVector<double>& a, const QVector<double>& b)
{
    double s = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

/* ---- Sparse matrix-vector product ---- */

QVector<double> BiCGSTAB4::spmv(const QVector<QVector<double>>& A,
                                   const QVector<double>& x)
{
    int n = A.size();
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i) {
        int cols = qMin(A[i].size(), x.size());
        for (int j = 0; j < cols; ++j)
            y[i] += A[i][j] * x[j];
    }
    return y;
}

/* ---- Forward triangular solve ---- */

QVector<double> BiCGSTAB4::forwardSolve(
    const QVector<QVector<QPair<int, double>>>& L,
    const QVector<double>& b) const
{
    int n = L.size();
    QVector<double> x(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double sum = b[i];
        for (const auto& [col, val] : L[i]) {
            if (col < i) sum -= val * x[col];
            else if (col == i) { /* diagonal handled below */ }
        }
        // Find diagonal
        double diag = 1.0;
        for (const auto& [col, val] : L[i])
            if (col == i) { diag = val; break; }
        x[i] = sum / qMax(qAbs(diag), 1e-15);
    }
    return x;
}

/* ---- Backward triangular solve ---- */

QVector<double> BiCGSTAB4::backwardSolve(
    const QVector<QVector<QPair<int, double>>>& U,
    const QVector<double>& b) const
{
    int n = U.size();
    QVector<double> x(n, 0.0);
    for (int i = n - 1; i >= 0; --i) {
        double sum = b[i];
        for (const auto& [col, val] : U[i]) {
            if (col > i) sum -= val * x[col];
        }
        double diag = 1.0;
        for (const auto& [col, val] : U[i])
            if (col == i) { diag = val; break; }
        x[i] = sum / qMax(qAbs(diag), 1e-15);
    }
    return x;
}

/* ---- Build SAINV preconditioner ---- */

void BiCGSTAB4::buildSAINV(const QVector<double>& values,
                              const QVector<int>& colIdx,
                              const QVector<int>& rowPtr, int n)
{
    m_precondN = n;
    m_sainvL.resize(n);
    m_sainvU.resize(n);

    // Simplified SAINV: build sparse approximate inverse via incomplete solve
    // Use dropped inverse of A^T * A diagonal
    for (int i = 0; i < n; ++i) {
        // Extract row i of A
        double diag = 1.0;
        int rowStart = rowPtr[i];
        int rowEnd = (i + 1 < n) ? rowPtr[i + 1] : values.size();
        for (int k = rowStart; k < rowEnd; ++k) {
            if (colIdx[k] == i) { diag = values[k]; break; }
        }

        // L factor: just keep lower entries with inverse scaling
        for (int k = rowStart; k < rowEnd; ++k) {
            int j = colIdx[k];
            if (j <= i) {
                m_sainvL[i].append({j, values[k] / qMax(qAbs(diag), 1e-15)});
            }
        }
        // Ensure diagonal
        bool hasDiag = false;
        for (auto& [col, val] : m_sainvL[i])
            if (col == i) { val = 1.0; hasDiag = true; }
        if (!hasDiag) m_sainvL[i].append({i, 1.0});

        // U factor: keep upper entries
        for (int k = rowStart; k < rowEnd; ++k) {
            int j = colIdx[k];
            if (j >= i) {
                m_sainvU[i].append({j, values[k] / qMax(qAbs(diag), 1e-15)});
            }
        }
        hasDiag = false;
        for (auto& [col, val] : m_sainvU[i])
            if (col == i) { val = 1.0; hasDiag = true; }
        if (!hasDiag) m_sainvU[i].append({i, 1.0});
    }
}

/* ---- Apply preconditioner ---- */

QVector<double> BiCGSTAB4::applyPreconditioner(const QVector<double>& v) const
{
    if (m_sainvL.isEmpty() || m_sainvU.isEmpty()) return v;
    auto y = forwardSolve(m_sainvL, v);
    return backwardSolve(m_sainvU, y);
}

/* ---- Solve dense ---- */

QVector<double> BiCGSTAB4::solve(const QVector<QVector<double>>& A,
                                    const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();
    int n = b.size();
    m_convergence.clear();
    m_stats.matrixSize = n;

    // Build simple SAINV from dense
    QVector<double> values;
    QVector<int> colIdx, rowPtr;
    rowPtr.append(0);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (qAbs(A[i][j]) > 1e-15) {
                values.append(A[i][j]);
                colIdx.append(j);
            }
        }
        rowPtr.append(values.size());
    }
    buildSAINV(values, colIdx, rowPtr, n);
    return solveSparse(values, colIdx, rowPtr, b, n);
}

/* ---- Solve sparse ---- */

QVector<double> BiCGSTAB4::solveSparse(const QVector<double>& values,
                                          const QVector<int>& colIdx,
                                          const QVector<int>& rowPtr,
                                          const QVector<double>& b, int n)
{
    QElapsedTimer timer;
    timer.start();
    m_convergence.clear();

    // Build sparse A*x product
    auto spmvCSR = [&](const QVector<double>& x) -> QVector<double> {
        QVector<double> y(n, 0.0);
        for (int i = 0; i < n; ++i) {
            int start = rowPtr[i];
            int end = (i + 1 < rowPtr.size()) ? rowPtr[i + 1] : values.size();
            for (int k = start; k < end; ++k)
                y[i] += values[k] * x[colIdx[k]];
        }
        return y;
    };

    // Initial guess x = 0
    QVector<double> x(n, 0.0);
    auto r = spmvCSR(b); // Actually r = b - A*0 = b
    r = b; // r = b (since x=0)

    double rNorm = qSqrt(dot(r, r));
    double bNorm = qSqrt(dot(b, b));
    if (bNorm < 1e-15) { m_stats.finalResidual = 0.0; return x; }

    auto r0hat = r;
    double rho0 = dot(r0hat, r);
    auto p = r;
    auto v = QVector<double>(n, 0.0);
    auto s = QVector<double>(n, 0.0);
    auto t = QVector<double>(n, 0.0);

    double omega = 1.0;
    double alpha = 0.0;
    double rho1 = 0.0;

    int iter = 0;
    for (iter = 0; iter < m_maxIter; ++iter) {
        rho1 = dot(r0hat, r);
        if (qAbs(rho1) < 1e-30) break;

        double beta = (rho1 / qMax(rho0, 1e-30)) * (alpha / qMax(omega, 1e-30));
        // p = r + beta * (p - omega * v)
        for (int i = 0; i < n; ++i)
            p[i] = r[i] + beta * (p[i] - omega * v[i]);

        // Apply preconditioner
        auto phat = applyPreconditioner(p);
        v = spmvCSR(phat);

        double r0v = dot(r0hat, v);
        if (qAbs(r0v) < 1e-30) break;
        alpha = rho1 / r0v;

        // s = r - alpha * v
        for (int i = 0; i < n; ++i)
            s[i] = r[i] - alpha * v[i];

        auto shat = applyPreconditioner(s);
        t = spmvCSR(shat);

        omega = dot(t, s) / qMax(dot(t, t), 1e-30);

        // GPBi-CG variant: extra smoothing step
        if (m_useGPBiCG && iter > 0) {
            double gamma = dot(t, s);
            double eta = gamma / qMax(dot(t, t), 1e-30);
            for (int i = 0; i < n; ++i)
                s[i] = s[i] - eta * t[i];
            for (int i = 0; i < n; ++i)
                x[i] = x[i] + alpha * phat[i] + eta * shat[i];
        } else {
            for (int i = 0; i < n; ++i)
                x[i] = x[i] + alpha * phat[i] + omega * shat[i];
        }

        for (int i = 0; i < n; ++i)
            r[i] = s[i] - omega * t[i];

        rho0 = rho1;
        double residual = qSqrt(dot(r, r)) / bNorm;
        m_convergence.append(residual);
        if (residual < m_tol) break;
    }

    m_stats.iterations = iter;
    m_stats.finalResidual = m_convergence.isEmpty() ? 0.0 : m_convergence.last();
    m_stats.totalSolves++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
    emit solveCompleted(iter, m_stats.finalResidual, timer.elapsed());
    return x;
}

/* ---- Convergence history ---- */

QVector<double> BiCGSTAB4::convergenceHistory() const { return m_convergence; }

/* ---- Reset ---- */

void BiCGSTAB4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_sainvL.clear();
    m_sainvU.clear();
    m_convergence.clear();
    m_precondN = 0;
}
