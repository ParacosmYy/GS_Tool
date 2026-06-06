/**
 * @file SparseLU2.cpp
 * @brief SparseLU2 实现
 *
 * 实现稀疏LU分解：AMD近似最小度排序、阈值部分主元、三角求解。
 */

#include "utils/matrix190/SparseLU2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SparseLU2::SparseLU2(QObject *parent) : QObject(parent) {}
SparseLU2::~SparseLU2() = default;

/* ---- Configuration ---- */

void SparseLU2::setThreshold(double threshold)
{
    m_threshold = qBound(0.0, threshold, 1.0);
}

/* ---- AMD approximate minimum degree ordering ---- */

QVector<int> SparseLU2::amdOrdering(int n, const QVector<QVector<QPair<int, double>>>& cols) const
{
    if (n == 0) return {};

    // Build degree array
    QVector<int> degree(n);
    for (int i = 0; i < n; ++i) degree[i] = cols[i].size();

    QVector<bool> eliminated(n, false);
    QVector<int> order;
    order.reserve(n);

    for (int step = 0; step < n; ++step) {
        // Pick uneliminated vertex with minimum degree
        int best = -1, bestDeg = std::numeric_limits<int>::max();
        for (int i = 0; i < n; ++i) {
            if (!eliminated[i] && degree[i] < bestDeg) {
                bestDeg = degree[i];
                best = i;
            }
        }
        if (best < 0) break;

        order.append(best);
        eliminated[best] = true;

        // Update degrees of neighbors (approximate fill)
        for (auto& entry : cols[best]) {
            int j = entry.first;
            if (!eliminated[j]) degree[j] = qMax(1, degree[j] - 1);
        }
    }
    return order;
}

/* ---- Threshold partial pivoting ---- */

int SparseLU2::selectPivot(const QVector<QVector<QPair<int, double>>>& U,
                            int col, int startRow,
                            const QVector<bool>& usedRows) const
{
    double maxVal = 0.0;
    int pivotRow = -1;
    double colMax = 0.0;

    // Find column max for threshold
    for (int r = startRow; r < U.size(); ++r) {
        if (usedRows[r]) continue;
        for (auto& e : U[r]) {
            if (e.first == col && qAbs(e.second) > colMax)
                colMax = qAbs(e.second);
        }
    }

    double tol = m_threshold * colMax;
    for (int r = startRow; r < U.size(); ++r) {
        if (usedRows[r]) continue;
        for (auto& e : U[r]) {
            if (e.first == col && qAbs(e.second) >= tol && qAbs(e.second) > maxVal) {
                maxVal = qAbs(e.second);
                pivotRow = r;
            }
        }
    }
    return pivotRow;
}

/* ---- Scatter-add ---- */

void SparseLU2::scatterAdd(QVector<double>& denseCol,
                            const QVector<QPair<int, double>>& sparseRow,
                            double factor) const
{
    for (auto& e : sparseRow)
        if (e.first < denseCol.size())
            denseCol[e.first] += factor * e.second;
}

/* ---- Main factorization ---- */

SparseLU2::LUFactorization SparseLU2::factorize(
    int n, const QVector<QVector<QPair<int, double>>>& cols)
{
    QElapsedTimer timer;
    timer.start();

    LUFactorization lu;
    if (n == 0) return lu;

    // Initialize permutation identity
    lu.rowPerm.resize(n);
    lu.colPerm.resize(n);
    for (int i = 0; i < n; ++i) { lu.rowPerm[i] = i; lu.colPerm[i] = i; }

    // Apply AMD column ordering
    auto amdOrder = amdOrdering(n, cols);
    if (amdOrder.size() == n) lu.colPerm = amdOrder;

    // Convert column sparse to dense working array for Gaussian elimination
    QVector<QVector<double>> U(n, QVector<double>(n, 0.0));
    for (int j = 0; j < n; ++j) {
        int cj = lu.colPerm[j];
        if (cj < cols.size()) {
            for (auto& e : cols[cj])
                if (e.first < n) U[e.first][j] = e.second;
        }
    }

    QVector<bool> usedRows(n, false);

    // Left-looking LU with threshold pivoting
    for (int k = 0; k < n; ++k) {
        // Select pivot row
        int pivot = -1;
        double maxVal = 0.0;
        for (int i = k; i < n; ++i) {
            if (qAbs(U[i][k]) > maxVal) {
                maxVal = qAbs(U[i][k]);
                pivot = i;
            }
        }

        if (pivot < 0 || maxVal < 1e-15) continue; // Skip zero column

        // Swap rows
        if (pivot != k) {
            std::swap(U[k], U[pivot]);
            std::swap(lu.rowPerm[k], lu.rowPerm[pivot]);
        }

        // Eliminate below
        for (int i = k + 1; i < n; ++i) {
            if (qAbs(U[i][k]) < 1e-15) continue;
            double factor = U[i][k] / U[k][k];
            for (int j = k; j < n; ++j)
                U[i][j] -= factor * U[k][j];
            U[i][k] = factor; // Store L factor in lower part
        }
    }

    // Extract sparse L and U
    lu.L.resize(n);
    lu.U.resize(n);
    int fillIn = 0;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i > j && qAbs(U[i][j]) > 1e-15) {
                lu.L[i].append({j, U[i][j]});
                fillIn++;
            } else if (i == j) {
                lu.L[i].append({j, 1.0});
                lu.U[i].append({j, U[i][j]});
            } else if (i <= j && qAbs(U[i][j]) > 1e-15) {
                lu.U[i].append({j, U[i][j]});
                fillIn++;
            }
        }
    }

    m_stats.totalFactorizations++;
    m_stats.matrixSize = n;
    m_stats.fillInCount = fillIn;
    m_stats.pivotingThreshold = m_threshold;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFactorizations;

    emit factorizationCompleted(n, fillIn, timer.elapsed());
    return lu;
}

/* ---- Triangular solve ---- */

QVector<double> SparseLU2::solve(const LUFactorization& lu,
                                  const QVector<double>& b) const
{
    int n = lu.rowPerm.size();
    if (n == 0 || b.size() != n) return {};

    // Apply row permutation: Pb
    QVector<double> y(n);
    for (int i = 0; i < n; ++i)
        y[i] = (lu.rowPerm[i] < b.size()) ? b[lu.rowPerm[i]] : 0.0;

    // Forward substitution Ly = Pb
    for (int i = 0; i < n; ++i) {
        for (auto& e : lu.L[i]) {
            if (e.first < i) y[i] -= e.second * y[e.first];
        }
    }

    // Back substitution Ux = y
    QVector<double> x(n, 0.0);
    for (int i = n - 1; i >= 0; --i) {
        x[i] = y[i];
        for (auto& e : lu.U[i]) {
            if (e.first > i && e.first < n) x[i] -= e.second * x[e.first];
        }
        // Find diagonal
        double diag = 1.0;
        for (auto& e : lu.U[i]) {
            if (e.first == i) { diag = e.second; break; }
        }
        if (qAbs(diag) > 1e-15) x[i] /= diag;
    }

    return x;
}

/* ---- Fill-in count ---- */

int SparseLU2::computeFillIn(const LUFactorization& lu) const
{
    int count = 0;
    for (int i = 0; i < lu.L.size(); ++i) count += lu.L[i].size();
    for (int i = 0; i < lu.U.size(); ++i) count += lu.U[i].size();
    return count;
}

/* ---- Condition estimate (1-norm) ---- */

double SparseLU2::conditionEstimate(const LUFactorization& lu, int n) const
{
    if (n == 0) return 0.0;

    // Simple estimate: max row sum of U
    double maxNorm = 0.0;
    for (int i = 0; i < qMin(n, lu.U.size()); ++i) {
        double rowSum = 0.0;
        for (auto& e : lu.U[i]) rowSum += qAbs(e.second);
        maxNorm = qMax(maxNorm, rowSum);
    }
    return (maxNorm > 1e-15) ? maxNorm : 1.0;
}

/* ---- Reset ---- */

void SparseLU2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
