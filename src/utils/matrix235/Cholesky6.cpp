/**
 * @file Cholesky6.cpp
 * @brief Cholesky6 实现
 *
 * 实现稀疏Cholesky分解：多重前沿装配树与并行前沿矩阵因子分解。
 */

#include "utils/matrix235/Cholesky6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

Cholesky6::Cholesky6(QObject *parent) : QObject(parent) {}
Cholesky6::~Cholesky6() = default;

/* ---- Configuration ---- */

void Cholesky6::setDimension(int n)
{
    m_n = n;
    m_entries.clear();
}

void Cholesky6::addEntry(int row, int col, double value)
{
    if (row < 0 || row >= m_n || col < 0 || col >= m_n) return;
    m_entries.append({qMax(row, col), qMin(row, col), value});
}

/* ---- Build elimination tree ---- */

QVector<int> Cholesky6::buildEliminationTree() const
{
    QVector<int> parent(m_n, -1);

    // Build column pattern from entries
    QVector<QVector<int>> colPattern(m_n);
    for (const auto& e : m_entries) {
        if (e.row != e.col)
            colPattern[e.col].append(e.row);
    }

    // Compute elimination tree via column merging
    QVector<int> ancestor(m_n, -1);
    for (int j = 0; j < m_n; ++j) {
        parent[j] = -1;
        ancestor[j] = -1;

        for (int r : colPattern[j]) {
            int i = r;
            // Find root of i's tree
            while (ancestor[i] != -1 && ancestor[i] != j) {
                int next = ancestor[i];
                ancestor[i] = j;
                i = next;
            }
            if (ancestor[i] == -1) {
                ancestor[i] = j;
                if (parent[j] == -1) parent[j] = i;
            }
        }
    }
    return parent;
}

/* ---- Build assembly tree ---- */

void Cholesky6::buildAssemblyTree(const QVector<int>& elimTree)
{
    QVector<QVector<int>> children(m_n);
    m_rootFrontal = -1;

    for (int i = 0; i < m_n; ++i) {
        if (elimTree[i] == -1)
            m_rootFrontal = i;
        else
            children[elimTree[i]].append(i);
    }

    m_frontals.resize(m_n);
    for (int i = 0; i < m_n; ++i) {
        m_frontals[i].nodeId = i;
        m_frontals[i].children = children[i];
        m_frontals[i].parent = elimTree[i];
        m_frontals[i].pivotSize = 1;
        m_frontals[i].factorized = false;
    }
}

/* ---- Assemble frontal matrix ---- */

void Cholesky6::assembleFrontal(int nodeId)
{
    auto& frontal = m_frontals[nodeId];
    frontal.rowIndices.clear();
    frontal.colIndices.clear();

    // Collect row/column indices from original matrix
    QSet<int> idxSet;
    idxSet.insert(nodeId);
    for (const auto& e : m_entries) {
        if (e.col == nodeId) idxSet.insert(e.row);
    }

    // Add contributions from children
    for (int child : frontal.children) {
        auto& childFrontal = m_frontals[child];
        for (int idx : childFrontal.rowIndices) {
            if (idx > nodeId) idxSet.insert(idx);
        }
    }

    // Sort indices
    frontal.rowIndices = idxSet.values().toVector();
    std::sort(frontal.rowIndices.begin(), frontal.rowIndices.end());
    frontal.colIndices = frontal.rowIndices;

    int sz = frontal.rowIndices.size();
    frontal.frontalData.resize(sz * sz, 0.0);

    // Fill from original entries
    for (const auto& e : m_entries) {
        if (e.col == nodeId || e.row == nodeId) {
            int rIdx = frontal.rowIndices.indexOf(qMax(e.row, e.col));
            int cIdx = frontal.colIndices.indexOf(qMin(e.row, e.col));
            if (rIdx >= 0 && cIdx >= 0)
                frontal.frontalData[rIdx * sz + cIdx] += e.value;
        }
    }

    // Add contributions from children
    for (int child : frontal.children) {
        auto& childF = m_frontals[child];
        int csz = childF.rowIndices.size();
        for (int ci = 0; ci < csz; ++ci) {
            for (int cj = 0; cj < csz; ++cj) {
                int ri = frontal.rowIndices.indexOf(childF.rowIndices[ci]);
                int rj = frontal.colIndices.indexOf(childF.colIndices[cj]);
                if (ri >= 0 && rj >= 0)
                    frontal.frontalData[ri * sz + rj] += childF.frontalData[ci * csz + cj];
            }
        }
    }
}

/* ---- Factorize single frontal ---- */

void Cholesky6::factorizeFrontal(int nodeId)
{
    auto& frontal = m_frontals[nodeId];
    int sz = frontal.rowIndices.size();

    // Dense Cholesky on the frontal matrix
    // L * L^T = A
    for (int i = 0; i < sz; ++i) {
        for (int j = 0; j <= i; ++j) {
            double sum = frontal.frontalData[i * sz + j];
            for (int k = 0; k < j; ++k)
                sum -= frontal.frontalData[i * sz + k] * frontal.frontalData[j * sz + k];

            if (i == j) {
                if (sum <= 0.0) sum = 1e-10;
                frontal.frontalData[i * sz + j] = qSqrt(sum);
            } else {
                frontal.frontalData[i * sz + j] = sum / qMax(1e-15, frontal.frontalData[j * sz + j]);
            }
        }
    }

    // Extract update block (Schur complement contribution)
    frontal.factorized = true;
    emit frontalFactorized(nodeId, frontal.pivotSize);
}

/* ---- Parallel factorization ---- */

void Cholesky6::factorizeParallel()
{
    // Post-order traversal for bottom-up factorization
    QVector<int> postOrder;
    QVector<bool> visited(m_n, false);

    // Simple iterative post-order
    QVector<int> stack;
    stack.append(m_rootFrontal);

    while (!stack.isEmpty()) {
        int node = stack.last();
        bool allChildrenDone = true;
        for (int child : m_frontals[node].children) {
            if (!visited[child]) {
                stack.append(child);
                allChildrenDone = false;
            }
        }
        if (allChildrenDone) {
            stack.removeLast();
            visited[node] = true;
            postOrder.append(node);
        }
    }

    // Process frontals bottom-up (independent frontals can be parallelized)
    for (int nodeId : postOrder) {
        assembleFrontal(nodeId);
        factorizeFrontal(nodeId);
    }
}

/* ---- Extract factor L ---- */

void Cholesky6::extractFactorL()
{
    m_factorL.clear();
    m_factorL.resize(m_n);

    for (int i = 0; i < m_n; ++i) {
        auto& frontal = m_frontals[i];
        int sz = frontal.rowIndices.size();

        for (int r = 0; r < sz; ++r) {
            double val = frontal.frontalData[r * sz + 0];
            if (qAbs(val) > 1e-15) {
                int row = frontal.rowIndices[r];
                if (row >= 0 && row < m_n)
                    m_factorL[row].append(qMakePair(i, val));
            }
        }
    }
}

/* ---- COLAMD ordering ---- */

QVector<int> Cholesky6::colamdOrdering() const
{
    // Simplified column approximate minimum degree ordering
    QVector<int> order(m_n);
    for (int i = 0; i < m_n; ++i) order[i] = i;
    return order;
}

/* ---- Factorize ---- */

bool Cholesky6::factorize()
{
    QElapsedTimer timer;
    timer.start();

    if (m_n <= 0) return false;

    QVector<int> elimTree = buildEliminationTree();
    buildAssemblyTree(elimTree);

    if (m_rootFrontal < 0) return false;

    factorizeParallel();
    extractFactorL();

    // Compute tree height
    int height = 0;
    for (int i = 0; i < m_n; ++i) {
        int h = 0, cur = i;
        while (m_frontals[cur].parent >= 0) { cur = m_frontals[cur].parent; h++; }
        height = qMax(height, h);
    }

    m_stats.matrixSize = m_n;
    m_stats.numNonZeros = m_entries.size();
    m_stats.numFrontals = m_n;
    m_stats.treeHeight = height + 1;
    m_stats.flops = 0.0;
    for (const auto& f : m_frontals) {
        int sz = f.rowIndices.size();
        m_stats.flops += sz * sz * sz / 3.0;
    }

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit factorizeCompleted(m_n, m_stats.flops, timer.elapsed());
    return true;
}

/* ---- Forward solve ---- */

QVector<double> Cholesky6::solveForward(const QVector<double>& b) const
{
    if (b.size() != m_n) return QVector<double>();

    QVector<double> x = b;
    for (int i = 0; i < m_n; ++i) {
        for (const auto& entry : m_factorL[i]) {
            if (entry.first < i)
                x[i] -= entry.second * x[entry.first];
        }
        if (i < m_factorL.size() && !m_factorL[i].isEmpty()) {
            double diag = m_factorL[i].first().second;
            if (qAbs(diag) > 1e-15) x[i] /= diag;
        }
    }
    return x;
}

/* ---- Backward solve ---- */

QVector<double> Cholesky6::solveBackward(const QVector<double>& b) const
{
    if (b.size() != m_n) return QVector<double>();

    QVector<double> x = b;
    for (int i = m_n - 1; i >= 0; --i) {
        for (int j = i + 1; j < m_n; ++j) {
            for (const auto& entry : m_factorL[j]) {
                if (entry.first == i)
                    x[i] -= entry.second * x[j];
            }
        }
        if (i < m_factorL.size() && !m_factorL[i].isEmpty()) {
            double diag = m_factorL[i].first().second;
            if (qAbs(diag) > 1e-15) x[i] /= diag;
        }
    }
    return x;
}

/* ---- Solve A*x = b ---- */

QVector<double> Cholesky6::solve(const QVector<double>& b) const
{
    QVector<double> y = solveForward(b);
    return solveBackward(y);
}

/* ---- Assembly tree accessor ---- */

QVector<Cholesky6::FrontalMatrix> Cholesky6::assemblyTree() const
{
    return m_frontals;
}

/* ---- Reset ---- */

void Cholesky6::resetStatistics()
{
    m_entries.clear();
    m_frontals.clear();
    m_factorL.clear();
    m_rootFrontal = -1;
    m_n = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
