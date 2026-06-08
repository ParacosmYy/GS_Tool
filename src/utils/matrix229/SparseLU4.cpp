/**
 * @file SparseLU4.cpp
 * @brief SparseLU4 实现
 *
 * 实现稀疏LU分解：AMF排序、层次调度ILU阈值丢弃、三角求解。
 */

#include "utils/matrix229/SparseLU4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SparseLU4::SparseLU4(QObject *parent) : QObject(parent) {}
SparseLU4::~SparseLU4() = default;

/* ---- Configuration ---- */

void SparseLU4::setParameters(double dropTolerance, int maxFill)
{
    m_dropTolerance = qMax(0.0, dropTolerance);
    m_maxFill = maxFill;
}

/* ---- COO to CSC ---- */

void SparseLU4::cooToCSC(const QVector<SparseEntry>& entries, int n,
                           QVector<int>& colPtr, QVector<int>& rowIdx,
                           QVector<double>& values) const
{
    // Sort by column, then by row
    QVector<SparseEntry> sorted = entries;
    std::sort(sorted.begin(), sorted.end(),
              [](const SparseEntry& a, const SparseEntry& b) {
                  return (a.col == b.col) ? (a.row < b.row) : (a.col < b.col);
              });

    colPtr.resize(n + 1, 0);
    rowIdx.clear();
    values.clear();

    for (const auto& e : sorted) {
        rowIdx.append(e.row);
        values.append(e.value);
        colPtr[e.col + 1]++;
    }
    for (int i = 0; i < n; ++i)
        colPtr[i + 1] += colPtr[i];
}

/* ---- Permute matrix ---- */

QVector<SparseLU4::SparseEntry> SparseLU4::permute(
    const QVector<SparseEntry>& entries, const QVector<int>& perm) const
{
    QVector<int> invPerm(perm.size());
    for (int i = 0; i < perm.size(); ++i) invPerm[perm[i]] = i;

    QVector<SparseEntry> result;
    result.reserve(entries.size());
    for (const auto& e : entries) {
        SparseEntry pe;
        pe.row = invPerm[e.row];
        pe.col = invPerm[e.col];
        pe.value = e.value;
        result.append(pe);
    }
    return result;
}

/* ---- AMF ordering ---- */

QVector<int> SparseLU4::amfOrdering(const QVector<SparseEntry>& entries, int n) const
{
    // Approximate Minimum Fill: greedy reordering to minimize fill-in
    QVector<int> perm(n);
    for (int i = 0; i < n; ++i) perm[i] = i;

    // Compute degree (approximate fill estimate) for each column
    QVector<int> degree(n, 0);
    QVector<QSet<int>> adj(n);
    for (const auto& e : entries) {
        if (e.row != e.col) {
            adj[e.col].insert(e.row);
            adj[e.row].insert(e.col);
        }
    }
    for (int i = 0; i < n; ++i) degree[i] = adj[i].size();

    // Greedy selection: pick column with smallest degree
    QVector<bool> eliminated(n, false);
    for (int step = 0; step < n; ++step) {
        int best = -1;
        int bestDeg = std::numeric_limits<int>::max();
        for (int i = 0; i < n; ++i) {
            if (!eliminated[i] && degree[i] < bestDeg) {
                bestDeg = degree[i];
                best = i;
            }
        }
        if (best < 0) break;

        perm[step] = best;
        eliminated[best] = true;

        // Update degrees: clique fill for neighbors
        QVector<int> neighbors;
        for (int v : adj[best]) {
            if (!eliminated[v]) neighbors.append(v);
        }
        for (int i = 0; i < neighbors.size(); ++i) {
            for (int j = i + 1; j < neighbors.size(); ++j) {
                int a = neighbors[i], b = neighbors[j];
                if (!adj[a].contains(b)) {
                    adj[a].insert(b);
                    adj[b].insert(a);
                    degree[a]++;
                    degree[b]++;
                }
            }
            degree[neighbors[i]] = adj[neighbors[i]].size();
        }
    }
    return perm;
}

/* ---- Symbolic factorization ---- */

QVector<QVector<int>> SparseLU4::symbolicFactor(int n,
    const QVector<int>& colPtr, const QVector<int>& rowIdx) const
{
    QVector<QVector<int>> fillPattern(n);
    for (int j = 0; j < n; ++j) {
        QSet<int> pattern;
        for (int p = colPtr[j]; p < colPtr[j + 1]; ++p) {
            if (rowIdx[p] > j) pattern.insert(rowIdx[p]);
        }
        for (int k = 0; k < j; ++k) {
            if (fillPattern[k].contains(j)) {
                for (int v : fillPattern[k]) {
                    if (v > j) pattern.insert(v);
                }
            }
        }
        fillPattern[j] = pattern.values().toVector();
        std::sort(fillPattern[j].begin(), fillPattern[j].end());
    }
    return fillPattern;
}

/* ---- Factorize ---- */

SparseLU4::LUResult SparseLU4::factorize(const QVector<SparseEntry>& entries, int n)
{
    QElapsedTimer timer;
    timer.start();

    LUResult result;
    m_stats.matrixSize = n;
    m_stats.nnzOriginal = entries.size();

    // AMF ordering
    QVector<int> perm = amfOrdering(entries, n);
    result.permutation = perm;

    // Permute matrix
    QVector<SparseEntry> permEntries = permute(entries, perm);

    // Convert to CSC
    QVector<int> colPtr, rowIdx;
    QVector<double> values;
    cooToCSC(permEntries, n, colPtr, rowIdx, values);

    // Build dense column arrays for factorization
    QVector<QVector<double>> L(n, QVector<double>(n, 0.0));
    QVector<QVector<double>> U(n, QVector<double>(n, 0.0));

    // Initialize U with upper triangle, L with identity
    for (int i = 0; i < n; ++i) L[i][i] = 1.0;
    for (const auto& e : permEntries) {
        if (e.row <= e.col) U[e.row][e.col] = e.value;
        else L[e.row][e.col] = e.value;
    }

    // Gaussian elimination with threshold dropping
    int fillIn = 0;
    int drops = 0;
    double maxVal = 0.0;
    for (const auto& e : permEntries)
        maxVal = qMax(maxVal, qAbs(e.value));
    double threshold = m_dropTolerance * maxVal;

    for (int k = 0; k < n; ++k) {
        if (qAbs(U[k][k]) < 1e-15) continue;
        for (int i = k + 1; i < n; ++i) {
            double factor = L[i][k] / U[k][k];
            if (qAbs(factor) < threshold) { drops++; continue; }

            L[i][k] = factor;
            for (int j = k; j < n; ++j) {
                U[i][j] -= factor * U[k][j];
                if (qAbs(U[i][j]) < threshold && i != j) {
                    U[i][j] = 0.0;
                    drops++;
                }
            }
            if (i > k) fillIn++;
        }
    }

    // Extract sparse L and U
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j <= i; ++j) {
            if (L[i][j] != 0.0) {
                SparseEntry e{i, j, L[i][j]};
                result.L.append(e);
            }
        }
    }
    for (int i = 0; i < n; ++i) {
        for (int j = i; j < n; ++j) {
            if (U[i][j] != 0.0) {
                SparseEntry e{i, j, U[i][j]};
                result.U.append(e);
            }
        }
    }

    result.fillInCount = fillIn;
    result.dropCount = drops;
    m_stats.fillInCount = fillIn;
    m_stats.droplets = drops;
    m_stats.nnzL = result.L.size();
    m_stats.nnzU = result.U.size();

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit factorizationCompleted(n, fillIn, timer.elapsed());
    return result;
}

/* ---- Forward solve (lower triangular) ---- */

QVector<double> SparseLU4::forwardSolve(const QVector<SparseEntry>& L,
                                           int n, const QVector<double>& b) const
{
    QVector<double> x(n, 0.0);
    // Build row-indexed structure for fast access
    QVector<QVector<QPair<int, double>>> rows(n);
    for (const auto& e : L)
        if (e.row >= 0 && e.row < n) rows[e.row].append(qMakePair(e.col, e.value));

    for (int i = 0; i < n; ++i) {
        double sum = b[i];
        for (const auto& p : rows[i]) {
            if (p.first < i) sum -= p.second * x[p.first];
        }
        // L has unit diagonal
        x[i] = sum;
    }
    return x;
}

/* ---- Backward solve (upper triangular) ---- */

QVector<double> SparseLU4::backwardSolve(const QVector<SparseEntry>& U,
                                            int n, const QVector<double>& y) const
{
    QVector<double> x(n, 0.0);
    QVector<QVector<QPair<int, double>>> rows(n);
    for (const auto& e : U)
        if (e.row >= 0 && e.row < n) rows[e.row].append(qMakePair(e.col, e.value));

    for (int i = n - 1; i >= 0; --i) {
        double sum = y[i];
        double diag = 1.0;
        for (const auto& p : rows[i]) {
            if (p.first == i) diag = p.second;
            else if (p.first > i) sum -= p.second * x[p.first];
        }
        x[i] = (qAbs(diag) > 1e-15) ? sum / diag : 0.0;
    }
    return x;
}

/* ---- Solve Ax=b ---- */

QVector<double> SparseLU4::solve(const LUResult& lu,
                                   const QVector<double>& b) const
{
    int n = m_stats.matrixSize;
    if (n == 0 || b.size() < n) return QVector<double>();

    // Apply permutation to b
    QVector<double> pb(n);
    for (int i = 0; i < n; ++i)
        pb[lu.permutation[i]] = b[i];

    QVector<double> y = forwardSolve(lu.L, n, pb);
    QVector<double> x = backwardSolve(lu.U, n, y);

    // Apply inverse permutation
    QVector<double> result(n);
    for (int i = 0; i < n; ++i)
        result[lu.permutation[i]] = x[i];

    return result;
}

/* ---- ILU threshold ---- */

SparseLU4::LUResult SparseLU4::iluThreshold(const QVector<SparseEntry>& entries, int n)
{
    return factorize(entries, n);
}

/* ---- Reset ---- */

void SparseLU4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
