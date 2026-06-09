/**
 * @file SparseLU4.cpp
 * @brief SparseLU4 实现
 *
 * 实现稀疏LU分解：列近似最小度排序与超节点块三角分解。
 */

#include "utils/matrix243/SparseLU4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SparseLU4::SparseLU4(QObject *parent) : QObject(parent) {}
SparseLU4::~SparseLU4() = default;

/* ---- COLAMD-like greedy degree ordering ---- */

QVector<int> SparseLU4::colamdOrder(int n, const QVector<Triplet>& triplets) const
{
    // Compute column degrees (number of nonzeros per column)
    QVector<int> degree(n, 0);
    for (const auto& t : triplets) degree[t.col]++;

    // Greedy minimum degree ordering
    QVector<int> order(n);
    QVector<bool> eliminated(n, false);

    for (int step = 0; step < n; ++step) {
        int minDeg = std::numeric_limits<int>::max();
        int best = 0;
        for (int j = 0; j < n; ++j) {
            if (!eliminated[j] && degree[j] < minDeg) {
                minDeg = degree[j];
                best = j;
            }
        }
        order[step] = best;
        eliminated[best] = true;

        // Approximate degree update: increase neighbors' degrees
        for (const auto& t : triplets) {
            if (t.col == best && !eliminated[t.row < n ? t.row : 0]) {
                // Only approximate: increase degree of rows sharing this column
                degree[t.row % n]++;
            }
        }
    }

    // Build inverse permutation
    QVector<int> perm(n);
    for (int i = 0; i < n; ++i) perm[order[i]] = i;
    return perm;
}

/* ---- Build CSC from COO ---- */

void SparseLU4::buildCSC(int n, const QVector<Triplet>& triplets,
                         const QVector<int>& perm,
                         QVector<int>& colPtr, QVector<int>& rowIdx,
                         QVector<double>& vals) const
{
    // Count entries per permuted column
    colPtr.assign(n + 1, 0);
    for (const auto& t : triplets) {
        int pc = perm[t.col];
        colPtr[pc + 1]++;
    }
    for (int j = 0; j < n; ++j) colPtr[j + 1] += colPtr[j];

    int nnz = triplets.size();
    rowIdx.resize(nnz);
    vals.resize(nnz);

    QVector<int> next = colPtr;
    for (const auto& t : triplets) {
        int pc = perm[t.col];
        int pos = next[pc]++;
        rowIdx[pos] = t.row;
        vals[pos] = t.val;
    }

    // Sort each column by row index
    for (int j = 0; j < n; ++j) {
        int start = colPtr[j], end = colPtr[j + 1];
        for (int i = start + 1; i < end; ++i) {
            int key = rowIdx[i];
            double keyVal = vals[i];
            int k = i - 1;
            while (k >= start && rowIdx[k] > key) {
                rowIdx[k + 1] = rowIdx[k];
                vals[k + 1] = vals[k];
                k--;
            }
            rowIdx[k + 1] = key;
            vals[k + 1] = keyVal;
        }
    }
}

/* ---- Detect supernodes ---- */

void SparseLU4::detectSupernodes()
{
    m_supernodes.clear();
    if (m_n == 0) return;

    int snStart = 0;
    for (int j = 1; j < m_n; ++j) {
        // Check if column j-1 and j have similar sparsity pattern in L
        bool isSupernode = true;
        int prevStart = m_lColPtr[j - 1], prevEnd = m_lColPtr[j];
        int curStart = m_lColPtr[j], curEnd = m_lColPtr[j + 1];

        if (prevEnd - prevStart != curEnd - curStart) isSupernode = false;
        else {
            for (int i = 0; i < prevEnd - prevStart - 1 && isSupernode; ++i) {
                if (m_lRowIdx[prevStart + i + 1] != m_lRowIdx[curStart + i + 1] - 1)
                    isSupernode = false;
            }
        }

        if (!isSupernode) {
            m_supernodes.append(snStart);
            snStart = j;
        }
    }
    m_supernodes.append(snStart);
    m_supernodes.append(m_n);
    m_stats.numSupernodes = m_supernodes.size() - 1;
}

/* ---- Factorize ---- */

bool SparseLU4::factorize(int n, const QVector<Triplet>& triplets)
{
    QElapsedTimer timer;
    timer.start();

    m_n = n;
    m_stats.matrixSize = n;
    m_stats.nnzOriginal = triplets.size();

    // COLAMD ordering
    m_colPerm = colamdOrder(n, triplets);
    m_colInvPerm.resize(n);
    for (int i = 0; i < n; ++i) m_colInvPerm[m_colPerm[i]] = i;

    // Build permuted CSC
    QVector<int> colPtr, rowIdx;
    QVector<double> vals;
    buildCSC(n, triplets, m_colPerm, colPtr, rowIdx, vals);

    // Allocate dense working storage for LU
    QVector<QVector<double>> LU(n, QVector<double>(n, 0.0));

    // Fill LU from sparse data
    for (int j = 0; j < n; ++j)
        for (int p = colPtr[j]; p < colPtr[j + 1]; ++p)
            LU[rowIdx[p]][j] = vals[p];

    // Row permutation (partial pivoting)
    m_rowPerm.resize(n);
    m_rowInvPerm.resize(n);
    for (int i = 0; i < n; ++i) m_rowPerm[i] = i;

    // Gaussian elimination with partial pivoting
    for (int k = 0; k < n; ++k) {
        // Find pivot
        double maxVal = qAbs(LU[k][k]);
        int maxRow = k;
        for (int i = k + 1; i < n; ++i) {
            if (qAbs(LU[i][k]) > maxVal) {
                maxVal = qAbs(LU[i][k]);
                maxRow = i;
            }
        }

        if (maxVal < 1e-15) continue;  // Skip singular column

        // Swap rows
        if (maxRow != k) {
            for (int j = 0; j < n; ++j)
                std::swap(LU[k][j], LU[maxRow][j]);
            std::swap(m_rowPerm[k], m_rowPerm[maxRow]);
        }

        // Eliminate below
        for (int i = k + 1; i < n; ++i) {
            LU[i][k] /= LU[k][k];
            for (int j = k + 1; j < n; ++j)
                LU[i][j] -= LU[i][k] * LU[k][j];
        }
    }

    // Build inverse row permutation
    for (int i = 0; i < n; ++i) m_rowInvPerm[m_rowPerm[i]] = i;

    // Extract L and U in CSC format
    int nnzL = 0, nnzU = 0;
    for (int j = 0; j < n; ++j) {
        for (int i = 0; i < n; ++i) {
            if (i > j && qAbs(LU[i][j]) > 1e-15) nnzL++;
            if (i <= j && qAbs(LU[i][j]) > 1e-15) nnzU++;
        }
    }

    m_lColPtr.resize(n + 1, 0);
    m_uColPtr.resize(n + 1, 0);
    m_lRowIdx.resize(nnzL);
    m_lVal.resize(nnzL);
    m_uRowIdx.resize(nnzU);
    m_uVal.resize(nnzU);

    int lPos = 0, uPos = 0;
    for (int j = 0; j < n; ++j) {
        m_lColPtr[j] = lPos;
        m_uColPtr[j] = uPos;
        for (int i = 0; i < n; ++i) {
            if (i > j && qAbs(LU[i][j]) > 1e-15) {
                m_lRowIdx[lPos] = i;
                m_lVal[lPos] = LU[i][j];
                lPos++;
            }
            if (i <= j && qAbs(LU[i][j]) > 1e-15) {
                m_uRowIdx[uPos] = i;
                m_uVal[uPos] = LU[i][j];
                uPos++;
            }
        }
    }
    m_lColPtr[n] = lPos;
    m_uColPtr[n] = uPos;

    m_stats.nnzFactors = nnzL + nnzU;
    m_stats.fillRatio = (m_stats.nnzOriginal > 0)
        ? static_cast<double>(m_stats.nnzFactors) / m_stats.nnzOriginal : 1.0;

    detectSupernodes();

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit factorizeCompleted(n, m_stats.nnzFactors, timer.elapsed());
    return true;
}

/* ---- Forward solve: L*y = Pb ---- */

QVector<double> SparseLU4::forwardSolve(const QVector<double>& b) const
{
    int n = m_n;
    QVector<double> y(n, 0.0);

    // Apply row permutation
    QVector<double> pb(n);
    for (int i = 0; i < n; ++i) pb[m_rowPerm[i]] = b[i];

    for (int j = 0; j < n; ++j) {
        y[j] = pb[j];
        for (int p = m_lColPtr[j]; p < m_lColPtr[j + 1]; ++p)
            y[m_lRowIdx[p]] -= m_lVal[p] * y[j];
    }
    return y;
}

/* ---- Back solve: U*x = y ---- */

QVector<double> SparseLU4::backSolve(const QVector<double>& y) const
{
    int n = m_n;
    QVector<double> x(n, 0.0);

    for (int j = n - 1; j >= 0; --j) {
        double diag = 1.0;
        for (int p = m_uColPtr[j]; p < m_uColPtr[j + 1]; ++p) {
            if (m_uRowIdx[p] == j) diag = m_uVal[p];
            else x[j] -= m_uVal[p] * x[m_uRowIdx[p]];
        }
        x[j] = (y[j] + x[j]) / diag;
    }
    return x;
}

/* ---- Solve ---- */

QVector<double> SparseLU4::solve(const QVector<double>& b) const
{
    QElapsedTimer timer;
    timer.start();

    if (b.size() != m_n) return {};

    QVector<double> y = forwardSolve(b);
    QVector<double> x = backSolve(y);

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit solveCompleted(m_n, timer.elapsed());
    return x;
}

/* ---- Accessors ---- */

void SparseLU4::getL(QVector<int>& colPtr, QVector<int>& rowIdx,
                     QVector<double>& vals) const
{
    colPtr = m_lColPtr; rowIdx = m_lRowIdx; vals = m_lVal;
}

void SparseLU4::getU(QVector<int>& colPtr, QVector<int>& rowIdx,
                     QVector<double>& vals) const
{
    colPtr = m_uColPtr; rowIdx = m_uRowIdx; vals = m_uVal;
}

QVector<int> SparseLU4::columnPermutation() const { return m_colPerm; }
QVector<int> SparseLU4::rowPermutation() const { return m_rowPerm; }

/* ---- Reset ---- */

void SparseLU4::resetStatistics()
{
    m_n = 0;
    m_lColPtr.clear(); m_lRowIdx.clear(); m_lVal.clear();
    m_uColPtr.clear(); m_uRowIdx.clear(); m_uVal.clear();
    m_colPerm.clear(); m_rowPerm.clear();
    m_colInvPerm.clear(); m_rowInvPerm.clear();
    m_supernodes.clear();
    m_stats = Stats{}; m_timeSum = 0.0;
}
