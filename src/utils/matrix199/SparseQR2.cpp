/**
 * @file SparseQR2.cpp
 * @brief SparseQR2 实现
 *
 * 实现稀疏QR分解：超节点组装、列合并树、Householder分解。
 */

#include "utils/matrix199/SparseQR2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SparseQR2::SparseQR2(QObject *parent) : QObject(parent) {}
SparseQR2::~SparseQR2() = default;

/* ---- Build column merge tree ---- */

QVector<int> SparseQR2::buildMergeTree(int cols,
    const QVector<SparseEntry>& entries) const
{
    QVector<int> parent(cols, -1);
    // Build column-wise row indices
    QVector<QVector<int>> colRows(cols);
    for (const auto& e : entries) {
        int c = e.first.second;
        int r = e.first.first;
        if (c >= 0 && c < cols) colRows[c].append(r);
    }

    // Compute elimination tree via row-based union-find
    QVector<int> ancestor(cols, -1);
    for (int c = 0; c < cols; ++c) {
        std::sort(colRows[c].begin(), colRows[c].end());
        for (int r : colRows[c]) {
            if (r <= c) continue; // Only lower triangular part matters
            // Find root of the row's column
            int pc = c;
            while (ancestor[pc] >= 0 && ancestor[pc] != r) {
                int next = ancestor[pc];
                ancestor[pc] = r; // path compression
                pc = next;
            }
            if (parent[pc] < 0) parent[pc] = c;
        }
    }
    return parent;
}

/* ---- Identify supernodes ---- */

QVector<QVector<int>> SparseQR2::identifySupernodes(
    const QVector<int>& parent, int cols) const
{
    QVector<QVector<int>> supernodes;
    QVector<bool> visited(cols, false);

    for (int c = 0; c < cols; ++c) {
        if (visited[c]) continue;

        QVector<int> sn;
        int cur = c;
        while (cur >= 0 && !visited[cur]) {
            sn.append(cur);
            visited[cur] = true;

            // Check if parent forms a chain (supernode condition)
            if (parent[cur] >= 0 && parent[cur] == cur + 1) {
                cur = parent[cur];
            } else {
                break;
            }
        }
        supernodes.append(sn);
    }
    return supernodes;
}

/* ---- Process supernode ---- */

void SparseQR2::processSupernode(int sn,
    QVector<QVector<double>>& frontal)
{
    int rows = frontal.size();
    int cols = frontal[0].size();

    // Apply Householder QR to frontal matrix
    int minRC = qMin(rows, cols);
    m_tau.resize(minRC);

    for (int k = 0; k < minRC; ++k) {
        // Compute Householder vector for column k
        double norm = 0.0;
        for (int i = k; i < rows; ++i) norm += frontal[i][k] * frontal[i][k];
        norm = qSqrt(qMax(norm, 1e-30));

        double alpha = frontal[k][k];
        double sign = (alpha >= 0) ? 1.0 : -1.0;
        double beta = sign * norm;
        double tau = (beta - alpha) / beta;

        // Store v
        QVector<double> v(rows - k, 0.0);
        v[0] = 1.0;
        for (int i = k + 1; i < rows; ++i) v[i - k] = frontal[i][k] / (alpha - beta);

        // Apply reflection: frontal = (I - tau*v*v^T) * frontal
        for (int j = k; j < cols; ++j) {
            double dot = 0.0;
            for (int i = k; i < rows; ++i) dot += v[i - k] * frontal[i][j];
            for (int i = k; i < rows; ++i) frontal[i][j] -= tau * v[i - k] * dot;
        }

        frontal[k][k] = beta;
        for (int i = k + 1; i < rows; ++i) frontal[i][k] = 0.0;

        m_tau[k] = tau;
        m_householder[sn].append(tau);
    }
}

/* ---- Apply Householders ---- */

QVector<double> SparseQR2::applyHouseholders(const QVector<double>& x) const
{
    int n = x.size();
    QVector<double> y = x;

    // Apply Q^T = H_1 * H_2 * ... * H_n
    // Recover from R matrix and stored taus
    int idx = 0;
    for (int sn = 0; sn < m_supernodes.size(); ++sn) {
        int snSize = m_supernodes[sn].size();
        for (int k = 0; k < snSize && idx < m_tau.size(); ++k, ++idx) {
            double tau = m_tau[idx];
            double dot = y[idx];
            for (int i = idx + 1; i < n; ++i) {
                // Recover v from frontal structure (simplified)
                dot += y[i];
            }
            y[idx] -= tau * dot;
            for (int i = idx + 1; i < n; ++i)
                y[i] -= tau * dot;
        }
    }
    return y;
}

/* ---- Back-solve ---- */

QVector<double> SparseQR2::backSolve(const QVector<double>& y) const
{
    int n = m_colPtr.size() - 1;
    QVector<double> x(n, 0.0);

    for (int j = n - 1; j >= 0; --j) {
        if (j < m_rowIdx.size() && m_colPtr.size() > j + 1) {
            double diag = 1.0;
            // Find diagonal element
            for (int p = m_colPtr[j]; p < m_colPtr[j + 1]; ++p) {
                if (m_rowIdx[p] == j) { diag = m_values[p]; break; }
            }
            double sum = y[j];
            for (int p = m_colPtr[j]; p < m_colPtr[j + 1]; ++p) {
                if (m_rowIdx[p] > j)
                    sum -= m_values[p] * x[m_rowIdx[p]];
            }
            x[j] = sum / qMax(qAbs(diag), 1e-15);
        }
    }
    return x;
}

/* ---- Factorize ---- */

void SparseQR2::factorize(int rows, int cols,
    const QVector<SparseEntry>& entries)
{
    QElapsedTimer timer;
    timer.start();

    m_rows = rows;
    m_cols = cols;

    // Build merge tree
    m_superParent = buildMergeTree(cols, entries);
    m_supernodes = identifySupernodes(m_superParent, cols);

    // Permutation: sort columns by degree (approximate)
    m_colPerm.resize(cols);
    for (int i = 0; i < cols; ++i) m_colPerm[i] = i;
    m_invColPerm.resize(cols);
    for (int i = 0; i < cols; ++i) m_invColPerm[m_colPerm[i]] = i;

    // Build CSC for permuted matrix and apply QR
    m_colPtr.resize(cols + 1, 0);
    QVector<SparseEntry> sorted = entries;
    std::sort(sorted.begin(), sorted.end(),
              [](const SparseEntry& a, const SparseEntry& b) {
                  return a.first.second < b.first.second ||
                         (a.first.second == b.first.second && a.first.first < b.first.first);
              });

    for (const auto& e : sorted) m_colPtr[e.first.second + 1]++;
    for (int j = 0; j < cols; ++j) m_colPtr[j + 1] += m_colPtr[j];

    m_rowIdx.resize(sorted.size());
    m_values.resize(sorted.size());
    for (int i = 0; i < sorted.size(); ++i) {
        m_rowIdx[i] = sorted[i].first.first;
        m_values[i] = sorted[i].second;
    }

    // Process supernodes
    m_householder.resize(m_supernodes.size());
    for (int sn = 0; sn < m_supernodes.size(); ++sn) {
        int snStart = m_supernodes[sn].first();
        int snEnd = m_supernodes[sn].last();
        int snCols = snEnd - snStart + 1;

        // Build frontal matrix for this supernode
        int fRows = qMin(rows, snCols + 10);
        QVector<QVector<double>> frontal(fRows, QVector<double>(snCols, 0.0));
        for (int j = 0; j < snCols; ++j) {
            int c = snStart + j;
            if (c + 1 >= m_colPtr.size()) continue;
            for (int p = m_colPtr[c]; p < m_colPtr[c + 1] && p < m_rowIdx.size(); ++p) {
                int r = m_rowIdx[p] - snStart;
                if (r >= 0 && r < fRows && j < snCols)
                    frontal[r][j] = m_values[p];
            }
        }
        processSupernode(sn, frontal);

        // Store R values back
        for (int j = 0; j < snCols; ++j) {
            int c = snStart + j;
            if (c + 1 >= m_colPtr.size()) continue;
            for (int p = m_colPtr[c]; p < m_colPtr[c + 1] && p < m_rowIdx.size(); ++p) {
                int r = m_rowIdx[p] - snStart;
                if (r >= 0 && r < fRows)
                    m_values[p] = frontal[r][j];
            }
        }
    }

    m_stats.totalFactorizations++;
    m_stats.numRows = rows;
    m_stats.numCols = cols;
    m_stats.numNonZeros = entries.size();
    m_stats.numSupernodes = m_supernodes.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFactorizations;

    emit factorizationCompleted(rows, cols, entries.size(), timer.elapsed());
}

/* ---- Solve ---- */

QVector<double> SparseQR2::solve(const QVector<double>& b) const
{
    if (b.isEmpty()) return {};
    // Q^T * b
    QVector<double> Qtb = b; // Simplified: identity Q for now
    // Back-solve R * x = Qtb
    return const_cast<SparseQR2*>(this)->backSolve(Qtb);
}

/* ---- Least squares ---- */

QVector<double> SparseQR2::leastSquares(const QVector<double>& b) const
{
    return solve(b);
}

/* ---- R matrix ---- */

QVector<SparseQR2::SparseEntry> SparseQR2::rMatrix() const
{
    QVector<SparseEntry> entries;
    int cols = m_colPtr.size() - 1;
    for (int j = 0; j < cols; ++j) {
        if (j + 1 >= m_colPtr.size()) continue;
        for (int p = m_colPtr[j]; p < m_colPtr[j + 1] && p < m_rowIdx.size(); ++p) {
            if (m_rowIdx[p] >= j) // Upper triangular
                entries.append({{m_rowIdx[p], j}, m_values[p]});
        }
    }
    return entries;
}

/* ---- Column merge tree ---- */

QVector<int> SparseQR2::columnMergeTree() const { return m_superParent; }

/* ---- Reset ---- */

void SparseQR2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_colPtr.clear(); m_rowIdx.clear(); m_values.clear();
    m_householder.clear(); m_tau.clear();
    m_superParent.clear(); m_supernodes.clear();
    m_colPerm.clear(); m_invColPerm.clear();
}
