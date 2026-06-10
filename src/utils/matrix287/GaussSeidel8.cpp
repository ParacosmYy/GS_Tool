/**
 * @file GaussSeidel8.cpp
 * @brief GaussSeidel8 实现
 *
 * 实现高斯-赛德尔迭代：块SOR与双色排序的缓存友好并行稀疏迭代求解。
 */

#include "utils/matrix287/GaussSeidel8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

GaussSeidel8::GaussSeidel8(QObject *parent)
    : QObject(parent) {}

GaussSeidel8::~GaussSeidel8() = default;

/* ---- Configuration ---- */

void GaussSeidel8::setTolerance(double tol) { m_tol = qMax(1e-15, tol); }
void GaussSeidel8::setMaxIterations(int maxIter) { m_maxIter = qBound(10, maxIter, 100000); }
void GaussSeidel8::setOmega(double omega) { m_omega = qBound(0.1, omega, 1.95); }
void GaussSeidel8::setBlockSize(int blockSize) { m_blockSize = qBound(1, blockSize, 64); }

/* ---- Set sparse matrix (CSR) ---- */

void GaussSeidel8::setMatrix(int n, const QVector<int>& rowPtr,
                              const QVector<SparseEntry>& entries)
{
    m_n = n;
    m_rowPtr = rowPtr;
    m_entries = entries;
    buildColorOrdering();
}

/* ---- Build two-color ordering ---- */

void GaussSeidel8::buildColorOrdering()
{
    m_colorGroup[0].clear();
    m_colorGroup[1].clear();

    // Simple red-black coloring: even rows = color 0, odd rows = color 1
    // Works well for 5-point stencil matrices
    for (int i = 0; i < m_n; ++i) {
        int color = i % 2;
        m_colorGroup[color].append(i);
    }
}

QVector<int> GaussSeidel8::twoColorOrdering() const
{
    QVector<int> order;
    order.reserve(m_n);
    for (int c = 0; c < 2; ++c)
        for (int v : m_colorGroup[c])
            order.append(v);
    return order;
}

/* ---- Matrix-vector product ---- */

QVector<double> GaussSeidel8::matVec(const QVector<double>& x) const
{
    QVector<double> y(m_n, 0.0);
    for (int i = 0; i < m_n; ++i) {
        double sum = 0.0;
        for (int k = m_rowPtr[i]; k < m_rowPtr[i + 1]; ++k)
            sum += m_entries[k].value * x[m_entries[k].col];
        y[i] = sum;
    }
    return y;
}

/* ---- L2 norm ---- */

double GaussSeidel8::normL2(const QVector<double>& v)
{
    double sum = 0.0;
    for (double x : v) sum += x * x;
    return qSqrt(sum);
}

/* ---- Residual computation ---- */

QVector<double> GaussSeidel8::residual(const QVector<double>& x,
                                        const QVector<double>& b) const
{
    QVector<double> ax = matVec(x);
    QVector<double> r(m_n);
    for (int i = 0; i < m_n; ++i)
        r[i] = b[i] - ax[i];
    return r;
}

/* ---- Solve a small block via direct elimination ---- */

void GaussSeidel8::solveBlock(QVector<double>& x, const QVector<double>& b,
                               int startRow, int endRow) const
{
    int blockSize = endRow - startRow;
    if (blockSize <= 0) return;

    // For block size 1: standard Gauss-Seidel update
    if (blockSize == 1) {
        int i = startRow;
        double diag = 1.0;
        double sum = b[i];
        for (int k = m_rowPtr[i]; k < m_rowPtr[i + 1]; ++k) {
            if (m_entries[k].col == i)
                diag = m_entries[k].value;
            else
                sum -= m_entries[k].value * x[m_entries[k].col];
        }
        if (qAbs(diag) > 1e-15) {
            double gs = sum / diag;
            x[i] = x[i] + m_omega * (gs - x[i]);  // SOR relaxation
        }
        return;
    }

    // Multi-row block: sequential GS sweep within block
    for (int i = startRow; i < endRow; ++i) {
        double diag = 1.0;
        double sum = b[i];
        for (int k = m_rowPtr[i]; k < m_rowPtr[i + 1]; ++k) {
            if (m_entries[k].col == i)
                diag = m_entries[k].value;
            else
                sum -= m_entries[k].value * x[m_entries[k].col];
        }
        if (qAbs(diag) > 1e-15) {
            double gs = sum / diag;
            x[i] = x[i] + m_omega * (gs - x[i]);
        }
    }
}

/* ---- Main solver ---- */

GaussSeidel8::SolveResult GaussSeidel8::solve(const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    SolveResult result;
    if (m_n == 0 || b.size() != m_n) return result;

    QVector<double> x(m_n, 0.0);  // Initial guess = 0
    bool converged = false;
    int iter = 0;

    for (iter = 0; iter < m_maxIter; ++iter) {
        // Two-color (red-black) sweep for parallel-friendly access
        for (int c = 0; c < 2; ++c) {
            // Process all rows of this color (independent → parallelizable)
            for (int idx = 0; idx < m_colorGroup[c].size(); ++idx) {
                int i = m_colorGroup[c][idx];
                int blockEnd = qMin(i + m_blockSize, m_n);

                // Adjust block to stay within same color
                int actualEnd = i + 1;
                for (int j = i + 1; j < blockEnd; ++j) {
                    if (j % 2 != i % 2) break;
                    actualEnd = j + 1;
                }
                solveBlock(x, b, i, actualEnd);
            }
        }

        // Check convergence every 10 iterations
        if (iter % 10 == 0 || iter == m_maxIter - 1) {
            QVector<double> r = residual(x, b);
            double resNorm = normL2(r);
            emit iterationDone(iter, resNorm, timer.elapsed());

            if (resNorm < m_tol) {
                converged = true;
                break;
            }
        }
    }

    result.solution = x;
    result.residualNorm = normL2(residual(x, b));
    result.iterations = iter;
    result.converged = converged;

    double elapsed = timer.elapsed();
    m_stats.matrixSize = m_n;
    m_stats.totalIterations += iter;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit solveDone(m_n, iter, converged, elapsed);

    return result;
}

/* ---- Reset ---- */

void GaussSeidel8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
