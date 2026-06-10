/**
 * @file GaussSeidel7.cpp
 * @brief GaussSeidel7 实现
 *
 * 实现高斯-赛德尔迭代：逐次超松弛与红黑排序加速稀疏系统求解。
 */

#include "utils/matrix273/GaussSeidel7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

GaussSeidel7::GaussSeidel7(QObject *parent)
    : QObject(parent) {}

GaussSeidel7::~GaussSeidel7() = default;

/* ---- Configuration ---- */

void GaussSeidel7::setOmega(double omega)
{
    m_omega = qBound(0.1, omega, 2.0);
}

void GaussSeidel7::setMaxIterations(int maxIter)
{
    m_maxIter = qMax(1, maxIter);
}

void GaussSeidel7::setTolerance(double tol)
{
    m_tol = qBound(1e-15, tol, 1.0);
}

void GaussSeidel7::setRedBlackOrdering(bool enabled)
{
    m_redBlack = enabled;
}

/* ---- Build checkerboard coloring ---- */

QVector<int> GaussSeidel7::buildColoring(int n) const
{
    // Simple 1D red-black: even indices = red, odd = black
    QVector<int> color(n);
    for (int i = 0; i < n; ++i)
        color[i] = i % 2;
    return color;
}

/* ---- Residual norm computation ---- */

double GaussSeidel7::residualNorm(int n, const QVector<double>& x,
                                   const QVector<double>& b,
                                   const QVector<int>& rowPtr,
                                   const QVector<int>& colIdx,
                                   const QVector<double>& values) const
{
    double sum = 0.0;
    for (int i = 0; i < n; ++i) {
        double ax = 0.0;
        for (int k = rowPtr[i]; k < rowPtr[i + 1]; ++k)
            ax += values[k] * x[colIdx[k]];
        double r = ax - b[i];
        sum += r * r;
    }
    return qSqrt(sum);
}

/* ---- Sparse CSR solve with SOR ---- */

QVector<double> GaussSeidel7::solve(int n, const QVector<double>& b,
                                     const QVector<int>& rowPtr,
                                     const QVector<int>& colIdx,
                                     const QVector<double>& values)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> x(n, 0.0);
    if (n == 0 || rowPtr.size() < n + 1) return x;

    // Initial guess: zero vector
    QVector<int> coloring = buildColoring(n);

    int iter = 0;
    double res = std::numeric_limits<double>::max();

    for (iter = 0; iter < m_maxIter; ++iter) {
        double prevRes = res;

        if (m_redBlack) {
            // Red sweep (color == 0)
            for (int i = 0; i < n; ++i) {
                if (coloring[i] != 0) continue;
                double sigma = 0.0;
                double diag = 1.0;
                for (int k = rowPtr[i]; k < rowPtr[i + 1]; ++k) {
                    int j = colIdx[k];
                    if (j == i) diag = values[k];
                    else sigma += values[k] * x[j];
                }
                if (qAbs(diag) < 1e-15) continue;
                x[i] = (1.0 - m_omega) * x[i] + m_omega * (b[i] - sigma) / diag;
            }
            // Black sweep (color == 1)
            for (int i = 0; i < n; ++i) {
                if (coloring[i] != 1) continue;
                double sigma = 0.0;
                double diag = 1.0;
                for (int k = rowPtr[i]; k < rowPtr[i + 1]; ++k) {
                    int j = colIdx[k];
                    if (j == i) diag = values[k];
                    else sigma += values[k] * x[j];
                }
                if (qAbs(diag) < 1e-15) continue;
                x[i] = (1.0 - m_omega) * x[i] + m_omega * (b[i] - sigma) / diag;
            }
        } else {
            // Standard lexicographic Gauss-Seidel SOR
            for (int i = 0; i < n; ++i) {
                double sigma = 0.0;
                double diag = 1.0;
                for (int k = rowPtr[i]; k < rowPtr[i + 1]; ++k) {
                    int j = colIdx[k];
                    if (j == i) diag = values[k];
                    else sigma += values[k] * x[j];
                }
                if (qAbs(diag) < 1e-15) continue;
                x[i] = (1.0 - m_omega) * x[i] + m_omega * (b[i] - sigma) / diag;
            }
        }

        res = residualNorm(n, x, b, rowPtr, colIdx, values);
        if (res < m_tol) break;
        if (qAbs(prevRes - res) < m_tol * 1e-3) break;
    }

    double elapsed = timer.elapsed();
    m_stats.matrixSize = n;
    m_stats.iterationsUsed = iter + 1;
    m_stats.finalResidual = res;
    m_stats.omega = m_omega;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit solveCompleted(iter + 1, res, elapsed);

    return x;
}

/* ---- Dense system solve ---- */

QVector<double> GaussSeidel7::solveDense(const QVector<QVector<double>>& A,
                                          const QVector<double>& b)
{
    int n = b.size();
    if (n == 0 || A.size() < n) return {};

    // Convert dense to CSR
    QVector<int> rowPtr(n + 1, 0);
    QVector<int> colIdx;
    QVector<double> values;

    for (int i = 0; i < n; ++i) {
        rowPtr[i] = values.size();
        for (int j = 0; j < n; ++j) {
            if (qAbs(A[i][j]) > 1e-15) {
                colIdx.append(j);
                values.append(A[i][j]);
            }
        }
    }
    rowPtr[n] = values.size();

    return solve(n, b, rowPtr, colIdx, values);
}

/* ---- Reset ---- */

void GaussSeidel7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
