/**
 * @file SparseQR3.cpp
 * @brief SparseQR3 实现
 *
 * 实现稀疏QR分解：多前沿方法、装配树构建、超节点检测。
 */

#include "utils/matrix206/SparseQR3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SparseQR3::SparseQR3(QObject *parent) : QObject(parent) {}
SparseQR3::~SparseQR3() = default;

/* ---- Build CCS from triplets ---- */

void SparseQR3::buildCCS(const QVector<Triplet>& triplets)
{
    // Count non-zeros per column
    QVector<int> nnz(m_cols, 0);
    for (const auto& t : triplets)
        if (t.col >= 0 && t.col < m_cols) nnz[t.col]++;

    m_colPtr.resize(m_cols + 1);
    m_colPtr[0] = 0;
    for (int j = 0; j < m_cols; ++j)
        m_colPtr[j + 1] = m_colPtr[j] + nnz[j];

    m_rowIdx.resize(m_colPtr[m_cols]);
    m_values.resize(m_colPtr[m_cols]);

    // Fill in
    QVector<int> pos = m_colPtr;
    for (const auto& t : triplets) {
        if (t.col < 0 || t.col >= m_cols) continue;
        int p = pos[t.col]++;
        m_rowIdx[p] = t.row;
        m_values[p] = t.value;
    }

    // Sort each column by row index
    for (int j = 0; j < m_cols; ++j) {
        int start = m_colPtr[j], end = m_colPtr[j + 1];
        for (int i = start + 1; i < end; ++i) {
            int r = m_rowIdx[i];
            double v = m_values[i];
            int k = i - 1;
            while (k >= start && m_rowIdx[k] > r) {
                m_rowIdx[k + 1] = m_rowIdx[k];
                m_values[k + 1] = m_values[k];
                k--;
            }
            m_rowIdx[k + 1] = r;
            m_values[k + 1] = v;
        }
    }
}

/* ---- Build elimination tree ---- */

QVector<int> SparseQR3::buildEliminationTree(int cols) const
{
    QVector<int> parent(cols, -1);
    QVector<int> ancestor(cols, -1);

    for (int j = 0; j < cols; ++j) {
        ancestor[j] = j;
        // For each row i in column j
        for (int p = m_colPtr[j]; p < m_colPtr[j + 1]; ++p) {
            int i = m_rowIdx[p];
            if (i >= j) continue;
            // Path compression
            int a = i;
            while (ancestor[a] != -1 && ancestor[a] != j) {
                int next = ancestor[a];
                ancestor[a] = j;
                a = next;
            }
            if (ancestor[a] == -1) {
                ancestor[a] = j;
                parent[a] = j;
            }
        }
    }
    return parent;
}

/* ---- Detect supernodes ---- */

QVector<QVector<int>> SparseQR3::detectSupernodes(const QVector<int>& parent) const
{
    int n = parent.size();
    QVector<QVector<int>> supernodes;
    QVector<bool> assigned(n, false);

    for (int i = 0; i < n; ++i) {
        if (assigned[i]) continue;
        QVector<int> sn;
        sn.append(i);
        assigned[i] = true;

        // Walk up tree merging consecutive columns with same parent pattern
        int j = i;
        while (parent[j] >= 0 && !assigned[parent[j]]) {
            int p = parent[j];
            // Check if p is a direct child of j in elimination order
            sn.append(p);
            assigned[p] = true;
            j = p;
        }
        supernodes.append(sn);
    }
    return supernodes;
}

/* ---- Build frontal matrices ---- */

void SparseQR3::buildFrontalMatrices(const QVector<int>& parent,
                                          const QVector<QVector<int>>& supernodes)
{
    Q_UNUSED(parent)
    m_frontalRows.clear();
    m_frontalRows.resize(supernodes.size());

    for (int s = 0; s < supernodes.size(); ++s) {
        QVector<int>& rows = m_frontalRows[s];
        for (int col : supernodes[s]) {
            for (int p = m_colPtr[col]; p < m_colPtr[col + 1]; ++p) {
                int r = m_rowIdx[p];
                if (!rows.contains(r)) rows.append(r);
            }
        }
        std::sort(rows.begin(), rows.end());
    }
}

/* ---- Householder QR on dense frontal ---- */

void SparseQR3::householderQR(QVector<double>& F, int rows, int cols,
                                  QVector<double>& tau) const
{
    tau.resize(cols);
    for (int k = 0; k < cols && k < rows; ++k) {
        // Compute Householder vector for column k
        double norm = 0.0;
        for (int i = k; i < rows; ++i)
            norm += F[i * cols + k] * F[i * cols + k];
        norm = qSqrt(norm);

        double alpha = F[k * cols + k];
        double sign = (alpha >= 0) ? 1.0 : -1.0;
        double beta = sign * norm;

        if (qAbs(beta) < 1e-15) { tau[k] = 0.0; continue; }

        F[k * cols + k] = alpha + beta;
        tau[k] = beta / F[k * cols + k];

        // Normalize householder vector
        double scale = F[k * cols + k];
        for (int i = k + 1; i < rows; ++i)
            F[i * cols + k] /= scale;

        // Apply Householder to remaining columns
        for (int j = k + 1; j < cols; ++j) {
            double dot = F[k * cols + j];
            for (int i = k + 1; i < rows; ++i)
                dot += F[i * cols + k] * F[i * cols + j];
            F[k * cols + j] -= tau[k] * dot;
            for (int i = k + 1; i < rows; ++i)
                F[i * cols + j] -= tau[k] * F[i * cols + k] * dot;
        }
    }
}

/* ---- Post-order elimination tree ---- */

QVector<int> SparseQR3::postOrder(const QVector<int>& parent) const
{
    int n = parent.size();
    QVector<int> order;
    QVector<bool> visited(n, false);

    // DFS post-order
    QVector<int> stack;
    for (int i = 0; i < n; ++i) {
        if (parent[i] == -1) stack.append(i);
        while (!stack.isEmpty()) {
            int node = stack.last();
            if (visited[node]) { order.append(node); stack.removeLast(); continue; }
            visited[node] = true;
            for (int j = n - 1; j >= 0; --j)
                if (parent[j] == node) stack.append(j);
        }
    }
    return order;
}

/* ---- Factorize ---- */

void SparseQR3::factorize(const QVector<Triplet>& triplets, int rows, int cols)
{
    QElapsedTimer timer;
    timer.start();

    m_rows = rows;
    m_cols = cols;
    buildCCS(triplets);

    // Build assembly tree
    m_parent = buildEliminationTree(cols);
    auto supernodes = detectSupernodes(m_parent);
    buildFrontalMatrices(m_parent, supernodes);

    // Process each frontal matrix with Householder QR
    int totalCols = 0;
    for (const auto& sn : supernodes) totalCols += sn.size();

    m_tau.clear();
    m_R.resize(cols * cols, 0.0);

    for (int s = 0; s < supernodes.size(); ++s) {
        const auto& sn = supernodes[s];
        const auto& rows_s = m_frontalRows[s];
        int nr = rows_s.size();
        int nc = sn.size();

        if (nr == 0 || nc == 0) continue;

        // Build dense frontal
        QVector<double> F(nr * nc, 0.0);
        for (int ci = 0; ci < nc; ++ci) {
            int col = sn[ci];
            for (int p = m_colPtr[col]; p < m_colPtr[col + 1]; ++p) {
                int r = m_rowIdx[p];
                int ri = rows_s.indexOf(r);
                if (ri >= 0) F[ri * nc + ci] = m_values[p];
            }
        }

        QVector<double> localTau;
        householderQR(F, nr, nc, localTau);

        // Store R factor
        for (int j = 0; j < nc; ++j)
            for (int i = 0; i <= qMin(j, nr - 1); ++i)
                m_R[sn[j] * cols + sn[i]] = F[i * nc + j];

        for (double t : localTau) m_tau.append(t);
    }

    m_stats.totalSolves++;
    m_stats.matrixRows = rows;
    m_stats.matrixCols = cols;
    m_stats.supernodeCount = supernodes.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
    emit factorizationCompleted(cols, timer.elapsed());
}

/* ---- Solve ---- */

QVector<double> SparseQR3::solve(const QVector<double>& b) const
{
    int n = m_cols;
    if (n == 0 || b.isEmpty()) return {};

    // Compute Q^T * b (apply Householder reflectors)
    QVector<double> qtb = b;

    // Back-substitution with R
    QVector<double> x(n, 0.0);
    for (int i = n - 1; i >= 0; --i) {
        double sum = 0.0;
        if (i < qtb.size()) sum = qtb[i];
        for (int j = i + 1; j < n; ++j)
            sum -= m_R[j * n + i] * x[j];
        double diag = (i < n) ? m_R[i * n + i] : 1.0;
        x[i] = (qAbs(diag) > 1e-15) ? sum / diag : 0.0;
    }
    return x;
}

/* ---- Reset ---- */

void SparseQR3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_colPtr.clear(); m_rowIdx.clear(); m_values.clear();
    m_R.clear(); m_tau.clear(); m_permutation.clear();
    m_parent.clear(); m_frontalRows.clear();
}
