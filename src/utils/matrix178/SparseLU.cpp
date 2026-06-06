/**
 * @file SparseLU.cpp
 * @brief SparseLU 实现
 *
 * 实现稀疏LU分解：AMD排序、符号/数值两阶段分解、三角求解。
 */

#include "utils/matrix178/SparseLU.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SparseLU::SparseLU(QObject *parent)
    : QObject(parent)
{
}

SparseLU::~SparseLU() = default;

/* ---- Configuration ---- */

void SparseLU::setMatrix(int n, const QVector<Triplet>& triplets)
{
    m_n = n;
    m_triplets = triplets;
    buildCSC();
}

/* ---- Build CSC from triplets ---- */

void SparseLU::buildCSC()
{
    /* Sort triplets by (col, row) */
    QVector<Triplet> sorted = m_triplets;
    std::sort(sorted.begin(), sorted.end(),
              [](const Triplet& a, const Triplet& b) {
                  return (a.col < b.col) || (a.col == b.col && a.row < b.row);
              });

    /* Count nonzeros per column */
    m_colPtr.resize(m_n + 1, 0);
    m_rowIdx.clear();
    m_values.clear();

    /* Count entries per column */
    QVector<int> colCount(m_n, 0);
    for (const auto& t : sorted) {
        if (t.row >= 0 && t.row < m_n && t.col >= 0 && t.col < m_n)
            colCount[t.col]++;
    }

    /* Build colPtr */
    m_colPtr[0] = 0;
    for (int j = 0; j < m_n; ++j)
        m_colPtr[j + 1] = m_colPtr[j] + colCount[j];

    m_rowIdx.resize(m_colPtr[m_n]);
    m_values.resize(m_colPtr[m_n]);

    /* Fill row indices and values */
    QVector<int> pos = m_colPtr;
    for (const auto& t : sorted) {
        if (t.row >= 0 && t.row < m_n && t.col >= 0 && t.col < m_n) {
            int idx = pos[t.col]++;
            m_rowIdx[idx] = t.row;
            m_values[idx] = t.value;
        }
    }
}

/* ---- AMD ordering (approximate minimum degree) ---- */

QVector<int> SparseLU::amdOrdering() const
{
    int n = m_n;
    QVector<int> perm(n);
    QVector<int> degree(n);
    QVector<bool> eliminated(n, false);

    /* Initialize degrees from CSC structure */
    for (int i = 0; i < n; ++i) {
        int count = 0;
        for (int j = 0; j < n; ++j) {
            if (j == i) continue;
            for (int p = m_colPtr[j]; p < m_colPtr[j + 1]; ++p) {
                if (m_rowIdx[p] == i) { count++; break; }
            }
        }
        degree[i] = count;
    }

    /* Greedy minimum degree selection */
    QVector<bool> eliminated(n, false);
    for (int step = 0; step < n; ++step) {
        /* Find uneliminated node with minimum degree */
        int minDeg = n + 1;
        int minNode = step;
        for (int i = 0; i < n; ++i) {
            if (!eliminated[i] && degree[i] < minDeg) {
                minDeg = degree[i];
                minNode = i;
            }
        }
        perm[step] = minNode;
        eliminated[minNode] = true;

        /* Update neighbor degrees (approximate) */
        for (int j = 0; j < n; ++j) {
            if (eliminated[j]) continue;
            bool isNeighbor = false;
            for (int p = m_colPtr[j]; p < m_colPtr[j + 1]; ++p) {
                if (m_rowIdx[p] == minNode) { isNeighbor = true; break; }
            }
            for (int p = m_colPtr[minNode]; p < m_colPtr[minNode + 1]; ++p) {
                if (m_rowIdx[p] == j) { isNeighbor = true; break; }
            }
            if (isNeighbor) degree[j]--;
        }
    }

    return perm;
}

/* ---- Symbolic factorization ---- */

void SparseLU::symbolicPhase()
{
    /* Simplified symbolic phase: determine L/U nonzero patterns
       using left-looking approach with elimination tree */
    m_lColPtr.resize(m_n + 1, 0);
    m_uColPtr.resize(m_n + 1, 0);

    /* For each column, predict fill-in pattern */
    QVector<QVector<int>> lPattern(m_n);
    QVector<QVector<int>> uPattern(m_n);

    for (int j = 0; j < m_n; ++j) {
        /* L pattern: rows below diagonal in column j */
        /* U pattern: columns to the right in row j */
        QVector<bool> lRows(m_n, false);
        QVector<bool> uCols(m_n, false);

        /* Original nonzeros */
        for (int p = m_colPtr[j]; p < m_colPtr[j + 1]; ++p) {
            int i = m_rowIdx[p];
            if (i > j) lRows[i] = true;
            if (i <= j) uCols[i] = true; /* Including diagonal */
        }

        /* Add fill from previous columns */
        for (int k = 0; k < j; ++k) {
            bool hasL_kj = false;
            for (int p = m_colPtr[j]; p < m_colPtr[j + 1]; ++p) {
                if (m_rowIdx[p] == k) { hasL_kj = true; break; }
            }
            if (!hasL_kj) continue;
            /* Fill: L[i,j] gets L[i,k] for i > j */
            for (int r : lPattern[k]) {
                if (r > j) lRows[r] = true;
            }
        }

        for (int i = 0; i < m_n; ++i) {
            if (lRows[i]) lPattern[j].append(i);
            if (uCols[i]) uPattern[j].append(i);
        }
    }

    /* Build CSC colPtr for L and U */
    m_lColPtr[0] = 0;
    m_uColPtr[0] = 0;
    for (int j = 0; j < m_n; ++j) {
        m_lColPtr[j + 1] = m_lColPtr[j] + lPattern[j].size() + 1; /* +1 for diag */
        m_uColPtr[j + 1] = m_uColPtr[j] + uPattern[j].size();
    }

    /* Allocate L structure */
    m_lRowIdx.resize(m_lColPtr[m_n]);
    m_lValues.resize(m_lColPtr[m_n], 0.0);
    m_uRowIdx.resize(m_uColPtr[m_n]);
    m_uValues.resize(m_uColPtr[m_n], 0.0);

    /* Fill row indices */
    int pos = 0;
    for (int j = 0; j < m_n; ++j) {
        m_lRowIdx[pos++] = j; /* Diagonal */
        for (int r : lPattern[j])
            m_lRowIdx[pos++] = r;
    }
    pos = 0;
    for (int j = 0; j < m_n; ++j) {
        for (int c : uPattern[j])
            m_uRowIdx[pos++] = c;
    }
}

/* ---- Numeric factorization ---- */

void SparseLU::numericPhase()
{
    /* Left-looking LU factorization */
    m_lValues.fill(0.0);
    m_uValues.fill(0.0);

    QVector<double> col(m_n, 0.0);

    for (int j = 0; j < m_n; ++j) {
        /* Scatter column j of A */
        col.fill(0.0);
        for (int p = m_colPtr[j]; p < m_colPtr[j + 1]; ++p)
            col[m_rowIdx[p]] = m_values[p];

        /* Apply previous columns */
        for (int k = 0; k < j; ++k) {
            double l_jk = 0.0;
            /* Find L[j,k] — but L stores lower, so find L[k][j] row */
            /* In CSC, column k of L contains row indices > k */
            /* We need L[j,k] which is entry in column k, row j */
            for (int p = m_lColPtr[k]; p < m_lColPtr[k + 1]; ++p) {
                if (m_lRowIdx[p] == j) { l_jk = m_lValues[p]; break; }
            }
            if (qAbs(l_jk) < 1e-30) continue;

            /* Update col[j..n-1] -= L[j,k] * U[k, j..] */
            for (int p = m_uColPtr[k]; p < m_uColPtr[k + 1]; ++p) {
                int c = m_uRowIdx[p];
                if (c >= j) col[c] -= l_jk * m_uValues[p];
            }
        }

        /* Extract L and U from col */
        double diag = col[j];
        if (qAbs(diag) < 1e-15) diag = 1e-15; /* Pivot */

        /* Store U column j */
        int uPos = m_uColPtr[j];
        for (int p = m_uColPtr[j]; p < m_uColPtr[j + 1]; ++p) {
            int r = m_uRowIdx[p];
            m_uValues[p] = col[r];
        }

        /* Store L column j: L[j,j]=1, L[i,j]=col[i]/diag */
        int lPos = m_lColPtr[j];
        for (int p = m_lColPtr[j]; p < m_lColPtr[j + 1]; ++p) {
            int r = m_lRowIdx[p];
            if (r == j)
                m_lValues[p] = 1.0;
            else
                m_lValues[p] = col[r] / diag;
        }
    }
}

/* ---- Full factorize ---- */

bool SparseLU::factorize()
{
    QElapsedTimer timer;
    timer.start();

    m_perm = amdOrdering();
    m_invPerm.resize(m_n);
    for (int i = 0; i < m_n; ++i) m_invPerm[m_perm[i]] = i;

    symbolicPhase();
    numericPhase();

    int nnzOrig = m_colPtr[m_n];
    int nnzFact = m_lColPtr[m_n] + m_uColPtr[m_n];

    m_stats.totalFactorizations++;
    m_stats.matrixSize = m_n;
    m_stats.nnzOriginal = nnzOrig;
    m_stats.nnzFactor = nnzFact;
    m_stats.fillRatio = (nnzOrig > 0) ? static_cast<double>(nnzFact) / nnzOrig : 0.0;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFactorizations;

    emit factorizationCompleted(m_n, nnzFact, m_stats.fillRatio);
    return true;
}

bool SparseLU::symbolicFactorize()
{
    m_perm = amdOrdering();
    m_invPerm.resize(m_n);
    for (int i = 0; i < m_n; ++i) m_invPerm[m_perm[i]] = i;
    symbolicPhase();
    return true;
}

bool SparseLU::numericFactorize()
{
    numericPhase();
    return true;
}

/* ---- Permute vector ---- */

QVector<double> SparseLU::permute(const QVector<double>& v) const
{
    int n = v.size();
    QVector<double> pv(n, 0.0);
    for (int i = 0; i < n; ++i)
        pv[i] = v[m_perm[i]];
    return pv;
}

/* ---- Forward solve: Ly = b ---- */

QVector<double> SparseLU::forwardSolve(const QVector<double>& b) const
{
    int n = b.size();
    QVector<double> y(n, 0.0);
    for (int j = 0; j < n; ++j) {
        double sum = b[j];
        for (int p = m_lColPtr[j]; p < m_lColPtr[j + 1]; ++p) {
            int r = m_lRowIdx[p];
            if (r < j) sum -= m_lValues[p] * y[r];
        }
        /* L[j,j] = 1.0 */
        y[j] = sum;
    }
    return y;
}

/* ---- Backward solve: Ux = y ---- */

QVector<double> SparseLU::backwardSolve(const QVector<double>& y) const
{
    int n = y.size();
    QVector<double> x(n, 0.0);
    for (int j = n - 1; j >= 0; --j) {
        double sum = y[j];
        for (int p = m_uColPtr[j]; p < m_uColPtr[j + 1]; ++p) {
            int c = m_uRowIdx[p];
            if (c > j) sum -= m_uValues[p] * x[c];
        }
        /* Find U[j,j] diagonal */
        double diag = 1.0;
        for (int p = m_uColPtr[j]; p < m_uColPtr[j + 1]; ++p) {
            if (m_uRowIdx[p] == j) { diag = m_uValues[p]; break; }
        }
        x[j] = sum / (qAbs(diag) < 1e-30 ? 1e-30 : diag);
    }
    return x;
}

/* ---- Solve Ax = b ---- */

QVector<double> SparseLU::solve(const QVector<double>& b) const
{
    if (b.size() != m_n) return {};
    QVector<double> pb = permute(b);
    QVector<double> y = forwardSolve(pb);
    QVector<double> x = backwardSolve(y);
    /* Inverse permute */
    QVector<double> result(m_n, 0.0);
    for (int i = 0; i < m_n; ++i)
        result[m_perm[i]] = x[i];
    return result;
}

double SparseLU::fillRatio() const { return m_stats.fillRatio; }

void SparseLU::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
