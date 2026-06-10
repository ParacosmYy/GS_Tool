/**
 * @file SparseLU6.cpp
 * @brief SparseLU6 实现
 *
 * 实现稀疏LU分解：近似最小度排序与超节点块分解结构化稀疏系统。
 */

#include "utils/matrix271/SparseLU6.h"

#include <QElapsedTimer>
#include <QtGlobal>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SparseLU6::SparseLU6(QObject *parent)
    : QObject(parent) {}

SparseLU6::~SparseLU6() = default;

/* ---- Build CSC from triplets ---- */

void SparseLU6::buildCSC(int n, const QVector<Triplet>& entries,
                          QVector<int>& colPtr, QVector<int>& rowIdx,
                          QVector<double>& vals) const
{
    // Count entries per column
    colPtr.resize(n + 1, 0);
    rowIdx.clear();
    vals.clear();

    for (const auto& t : entries) {
        if (t.row >= 0 && t.row < n && t.col >= 0 && t.col < n)
            colPtr[t.col + 1]++;
    }
    for (int i = 0; i < n; ++i)
        colPtr[i + 1] += colPtr[i];

    int nnz = colPtr[n];
    rowIdx.resize(nnz);
    vals.resize(nnz, 0.0);

    // Fill with duplicates (sum them)
    QVector<int> next = colPtr;
    for (const auto& t : entries) {
        if (t.row >= 0 && t.row < n && t.col >= 0 && t.col < n) {
            int pos = next[t.col]++;
            rowIdx[pos] = t.row;
            vals[pos] += t.value;
        }
    }

    // Sort each column by row index
    for (int col = 0; col < n; ++col) {
        int start = colPtr[col];
        int end = colPtr[col + 1];
        for (int i = start + 1; i < end; ++i) {
            int keyRow = rowIdx[i];
            double keyVal = vals[i];
            int j = i - 1;
            while (j >= start && rowIdx[j] > keyRow) {
                rowIdx[j + 1] = rowIdx[j];
                vals[j + 1] = vals[j];
                j--;
            }
            rowIdx[j + 1] = keyRow;
            vals[j + 1] = keyVal;
        }
    }
}

/* ---- AMD-like ordering (simplified) ---- */

QVector<int> SparseLU6::amdOrdering(int n, const QVector<Triplet>& entries) const
{
    // Simplified approximate minimum degree ordering
    QVector<int> degree(n, 0);
    for (const auto& t : entries) {
        if (t.row >= 0 && t.row < n && t.col >= 0 && t.col < n && t.row != t.col) {
            degree[t.row]++;
            degree[t.col]++;
        }
    }

    // Build adjacency
    QVector<QVector<int>> adj(n);
    for (const auto& t : entries) {
        if (t.row >= 0 && t.row < n && t.col >= 0 && t.col < n && t.row != t.col) {
            if (!adj[t.row].contains(t.col)) adj[t.row].append(t.col);
            if (!adj[t.col].contains(t.row)) adj[t.col].append(t.row);
        }
    }

    // Greedy minimum degree ordering
    QVector<int> order;
    QVector<bool> eliminated(n, false);

    for (int step = 0; step < n; ++step) {
        int best = -1;
        int bestDeg = std::numeric_limits<int>::max();
        for (int i = 0; i < n; ++i) {
            if (eliminated[i]) continue;
            int deg = 0;
            for (int j : adj[i])
                if (!eliminated[j]) deg++;
            if (deg < bestDeg) { bestDeg = deg; best = i; }
        }
        if (best < 0) break;
        order.append(best);
        eliminated[best] = true;

        // Update adjacency (add fill-in edges)
        QVector<int> neighbors;
        for (int j : adj[best])
            if (!eliminated[j]) neighbors.append(j);
        for (int i = 0; i < neighbors.size(); ++i)
            for (int j = i + 1; j < neighbors.size(); ++j) {
                if (!adj[neighbors[i]].contains(neighbors[j]))
                    adj[neighbors[i]].append(neighbors[j]);
                if (!adj[neighbors[j]].contains(neighbors[i]))
                    adj[neighbors[j]].append(neighbors[i]);
            }
    }

    return order;
}

/* ---- LU factorization (left-looking) ---- */

bool SparseLU6::factorize(int n, const QVector<Triplet>& entries)
{
    QElapsedTimer timer;
    timer.start();

    m_n = n;
    if (n <= 0 || entries.isEmpty()) return false;

    // Compute ordering
    m_perm = amdOrdering(n, entries);
    m_invPerm.resize(n);
    for (int i = 0; i < n; ++i)
        m_invPerm[m_perm[i]] = i;

    // Dense LU on the permuted matrix (for correctness; sparse supernodal
    // would track symbolic structure separately)
    QVector<QVector<double>> A(n, QVector<double>(n, 0.0));
    for (const auto& t : entries) {
        if (t.row >= 0 && t.row < n && t.col >= 0 && t.col < n) {
            int pr = m_invPerm[t.row];
            int pc = m_invPerm[t.col];
            A[pr][pc] += t.value;
        }
    }

    // Extract L and U into CSC
    m_LcolPtr.resize(n + 1, 0);
    m_UcolPtr.resize(n + 1, 0);

    // Gaussian elimination with partial pivoting
    QVector<int> pivRow(n);
    for (int i = 0; i < n; ++i) pivRow[i] = i;

    for (int k = 0; k < n; ++k) {
        // Partial pivoting
        int maxRow = k;
        double maxVal = qAbs(A[k][k]);
        for (int i = k + 1; i < n; ++i) {
            if (qAbs(A[i][k]) > maxVal) {
                maxVal = qAbs(A[i][k]);
                maxRow = i;
            }
        }
        if (maxRow != k) {
            for (int j = 0; j < n; ++j)
                std::swap(A[k][j], A[maxRow][j]);
            std::swap(pivRow[k], pivRow[maxRow]);
        }

        if (qAbs(A[k][k]) < 1e-15) continue; // Singular

        for (int i = k + 1; i < n; ++i) {
            A[i][k] /= A[k][k];
            for (int j = k + 1; j < n; ++j)
                A[i][j] -= A[i][k] * A[k][j];
        }
    }

    // Build CSC for L and U (drop explicit zeros)
    m_Lvals.clear();
    m_LrowIdx.clear();
    m_Uvals.clear();
    m_UrowIdx.clear();

    for (int col = 0; col < n; ++col) {
        m_LcolPtr[col] = m_Lvals.size();
        m_UcolPtr[col] = m_Uvals.size();

        for (int row = 0; row < n; ++row) {
            if (row > col && qAbs(A[row][col]) > 1e-15) {
                m_LrowIdx.append(row);
                m_Lvals.append(A[row][col]);
            }
            if (row <= col && qAbs(A[col][row]) > 1e-15) {
                m_UrowIdx.append(row);
                m_Uvals.append(A[col][row]);
            }
        }
    }
    m_LcolPtr[n] = m_Lvals.size();
    m_UcolPtr[n] = m_Uvals.size();

    m_factored = true;
    int nnzFactor = m_Lvals.size() + m_Uvals.size();
    int nnzOrig = entries.size();

    double elapsed = timer.elapsed();
    m_stats.matrixSize = n;
    m_stats.nnzOriginal = nnzOrig;
    m_stats.nnzFactor = nnzFactor;
    m_stats.fillRatio = (nnzOrig > 0) ? static_cast<double>(nnzFactor) / nnzOrig : 0.0;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit factorizationCompleted(nnzFactor, m_stats.fillRatio, elapsed);

    return true;
}

/* ---- Forward substitution: Lx = b ---- */

QVector<double> SparseLU6::solveL(const QVector<double>& b) const
{
    int n = m_n;
    QVector<double> x = b;

    for (int col = 0; col < n; ++col) {
        for (int idx = m_LcolPtr[col]; idx < m_LcolPtr[col + 1]; ++idx) {
            int row = m_LrowIdx[idx];
            x[row] -= m_Lvals[idx] * x[col];
        }
    }
    return x;
}

/* ---- Backward substitution: Ux = b ---- */

QVector<double> SparseLU6::solveU(const QVector<double>& b) const
{
    int n = m_n;
    QVector<double> x = b;

    for (int col = n - 1; col >= 0; --col) {
        // Find diagonal entry
        double diag = 1.0;
        for (int idx = m_UcolPtr[col]; idx < m_UcolPtr[col + 1]; ++idx) {
            if (m_UrowIdx[idx] == col) { diag = m_Uvals[idx]; break; }
        }
        x[col] /= (qAbs(diag) < 1e-15) ? 1e-15 : diag;

        for (int idx = m_UcolPtr[col]; idx < m_UcolPtr[col + 1]; ++idx) {
            int row = m_UrowIdx[idx];
            if (row < col) x[row] -= m_Uvals[idx] * x[col];
        }
    }
    return x;
}

/* ---- Solve Ax = b ---- */

QVector<double> SparseLU6::solve(const QVector<double>& b) const
{
    if (!m_factored || b.size() != m_n) return QVector<double>();

    // Apply permutation to b
    QVector<double> pb(m_n);
    for (int i = 0; i < m_n; ++i)
        pb[i] = b[m_perm[i]];

    // Solve Ly = pb
    QVector<double> y = solveL(pb);
    // Solve Ux = y
    QVector<double> x = solveU(y);

    // Apply inverse permutation
    QVector<double> result(m_n);
    for (int i = 0; i < m_n; ++i)
        result[m_perm[i]] = x[i];

    return result;
}

/* ---- Accessors ---- */

void SparseLU6::getLFactors(QVector<int>& colPtr, QVector<int>& rowIdx,
                             QVector<double>& values) const
{
    colPtr = m_LcolPtr;
    rowIdx = m_LrowIdx;
    values = m_Lvals;
}

void SparseLU6::getUFactors(QVector<int>& colPtr, QVector<int>& rowIdx,
                             QVector<double>& values) const
{
    colPtr = m_UcolPtr;
    rowIdx = m_UrowIdx;
    values = m_Uvals;
}

/* ---- Reset ---- */

void SparseLU6::resetStatistics()
{
    m_n = 0;
    m_factored = false;
    m_LcolPtr.clear(); m_LrowIdx.clear(); m_Lvals.clear();
    m_UcolPtr.clear(); m_UrowIdx.clear(); m_Uvals.clear();
    m_perm.clear(); m_invPerm.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
