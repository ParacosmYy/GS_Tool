/**
 * @file SparseSOR.cpp
 * @brief 稀疏SOR迭代求解器实现 — 逐次超松弛/CSR存储/动态omega
 */
#include "utils/matrix26/SparseSOR.h"
#include <QElapsedTimer>
#include <cmath>

SparseSOR::SparseSOR(QObject* parent)
    : QObject(parent), m_n(0), m_omega(1.5), m_dynamicOmega(true)
    , m_tolerance(1e-10), m_maxIter(10000)
{
}

void SparseSOR::buildFromCOO(const QVector<int>& rows, const QVector<int>& cols,
                             const QVector<double>& vals, int n)
{
    m_n = n;
    int nnz = qMin(rows.size(), qMin(cols.size(), vals.size()));
    QVector<int> rowCounts(n + 1, 0);
    for (int i = 0; i < nnz; ++i) {
        if (rows[i] >= 0 && rows[i] < n) rowCounts[rows[i] + 1]++;
    }
    m_rowPtr.resize(n + 1);
    m_rowPtr[0] = 0;
    for (int i = 1; i <= n; ++i)
        m_rowPtr[i] = m_rowPtr[i - 1] + rowCounts[i];
    m_colIdx.resize(nnz); m_values.resize(nnz);
    QVector<int> pos(n, 0);
    for (int i = 0; i < nnz; ++i) {
        int r = rows[i];
        if (r < 0 || r >= n) continue;
        int dest = m_rowPtr[r] + pos[r];
        m_colIdx[dest] = cols[i]; m_values[dest] = vals[i]; pos[r]++;
    }
    for (int i = 0; i < n; ++i) {
        int start = m_rowPtr[i], end = m_rowPtr[i + 1];
        for (int j = start; j < end - 1; ++j) {
            for (int k = start; k < end - 1 - (j - start); ++k) {
                if (m_colIdx[k] > m_colIdx[k + 1]) {
                    std::swap(m_colIdx[k], m_colIdx[k + 1]);
                    std::swap(m_values[k], m_values[k + 1]);
                }
            }
        }
    }
    m_diag.resize(n);
    for (int i = 0; i < n; ++i) {
        m_diag[i] = 0.0;
        for (int j = m_rowPtr[i]; j < m_rowPtr[i + 1]; ++j) {
            if (m_colIdx[j] == i) { m_diag[i] = m_values[j]; break; }
        }
    }
}

void SparseSOR::setOmega(double omega) { m_omega = qBound(0.01, omega, 1.99); }
void SparseSOR::setDynamicOmega(bool enable) { m_dynamicOmega = enable; }
void SparseSOR::setTolerance(double tol) { m_tolerance = qMax(1e-15, tol); }
void SparseSOR::setMaxIterations(int maxIter) { m_maxIter = qMax(1, maxIter); }

SparseSOR::SolveResult SparseSOR::solve(const QVector<double>& rhs)
{
    if (m_n <= 0) return SolveResult{};
    QVector<double> x0(m_n, 0.0);
    return solveWithGuess(rhs, x0);
}

SparseSOR::SolveResult SparseSOR::solveWithGuess(const QVector<double>& rhs,
                                                  const QVector<double>& initialGuess)
{
    SolveResult result;
    if (m_n <= 0 || rhs.size() < m_n) return result;
    QElapsedTimer timer; timer.start();
    result.solution = initialGuess;
    if (result.solution.size() < m_n) result.solution.resize(m_n);
    double prevResidual = computeResidualNorm(result.solution, rhs);
    double omega = m_omega;
    for (int iter = 0; iter < m_maxIter; ++iter) {
        for (int i = 0; i < m_n; ++i) {
            double sum = 0.0;
            for (int j = m_rowPtr[i]; j < m_rowPtr[i + 1]; ++j) {
                if (m_colIdx[j] != i) sum += m_values[j] * result.solution[m_colIdx[j]];
            }
            if (std::abs(m_diag[i]) > 1e-15)
                result.solution[i] = (1.0 - omega) * result.solution[i]
                    + omega * (rhs[i] - sum) / m_diag[i];
        }
        double curResidual = computeResidualNorm(result.solution, rhs);
        if (iter % 50 == 0) emit iterationProgress(iter, curResidual);
        if (m_dynamicOmega && iter > 0) omega = adjustOmega(iter, prevResidual, curResidual);
        if (curResidual < m_tolerance) {
            result.converged = true; result.iterations = iter + 1;
            result.finalResidual = curResidual; result.omegaUsed = omega; break;
        }
        prevResidual = curResidual;
        result.iterations = iter + 1;
        result.finalResidual = curResidual; result.omegaUsed = omega;
    }
    m_stats.totalSolves++; m_stats.totalIterations += result.iterations;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
    m_stats.avgIterations = static_cast<double>(m_stats.totalIterations) / m_stats.totalSolves;
    if (result.finalResidual < m_stats.bestResidual) m_stats.bestResidual = result.finalResidual;
    emit solveComplete(result.converged, result.iterations);
    return result;
}

QVector<double> SparseSOR::residual(const QVector<double>& x, const QVector<double>& rhs) const
{
    QVector<double> r(m_n, 0.0);
    for (int i = 0; i < m_n; ++i) {
        r[i] = rhs[i];
        for (int j = m_rowPtr[i]; j < m_rowPtr[i + 1]; ++j)
            r[i] -= m_values[j] * x[m_colIdx[j]];
    }
    return r;
}

QVector<double> SparseSOR::spmv(const QVector<double>& x) const
{
    QVector<double> y(m_n, 0.0);
    for (int i = 0; i < m_n; ++i)
        for (int j = m_rowPtr[i]; j < m_rowPtr[i + 1]; ++j)
            y[i] += m_values[j] * x[m_colIdx[j]];
    return y;
}

double SparseSOR::computeResidualNorm(const QVector<double>& x,
                                      const QVector<double>& rhs) const
{
    double norm = 0.0;
    for (int i = 0; i < m_n; ++i) {
        double ri = rhs[i];
        for (int j = m_rowPtr[i]; j < m_rowPtr[i + 1]; ++j)
            ri -= m_values[j] * x[m_colIdx[j]];
        norm += ri * ri;
    }
    return std::sqrt(norm);
}

double SparseSOR::adjustOmega(int /*iteration*/, double oldResidual, double newResidual)
{
    double ratio = (oldResidual > 1e-30) ? newResidual / oldResidual : 1.0;
    double omega = m_omega;
    if (ratio < 0.5) omega = qMin(1.99, omega * 1.05);
    else if (ratio > 0.95) omega = qMax(0.5, omega * 0.95);
    return omega;
}

void SparseSOR::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
