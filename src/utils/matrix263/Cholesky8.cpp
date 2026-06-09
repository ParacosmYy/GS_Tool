/**
 * @file Cholesky8.cpp
 * @brief Cholesky8 实现
 *
 * 实现Cholesky分解：稀疏符号分析与超节点块因子化缓存优化。
 */

#include "utils/matrix263/Cholesky8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

Cholesky8::Cholesky8(QObject *parent)
    : QObject(parent) {}
Cholesky8::~Cholesky8() = default;

/* ---- Set matrix (CSC format) ---- */

void Cholesky8::setMatrix(int n, const QVector<int>& colPtr,
                           const QVector<int>& rowIdx,
                           const QVector<double>& values)
{
    m_n = n;
    m_colPtr = colPtr;
    m_rowIdx = rowIdx;
    m_values = values;
    m_symbolicDone = false;
    m_factorized = false;
}

/* ---- Elimination tree ---- */

QVector<int> Cholesky8::eliminationTree() const
{
    QVector<int> parent(m_n, -1);
    for (int j = 0; j < m_n; ++j) {
        for (int p = m_colPtr[j]; p < m_colPtr[j + 1]; ++p) {
            int i = m_rowIdx[p];
            if (i < j) {
                // Find root of i's subtree and set parent
                int root = i;
                while (parent[root] != -1 && parent[root] != j) {
                    int next = parent[root];
                    parent[root] = j;  // Path compression
                    root = next;
                }
                if (parent[root] == -1) parent[root] = j;
            }
        }
    }
    return parent;
}

/* ---- Post-order traversal ---- */

QVector<int> Cholesky8::postOrder(const QVector<int>& parent) const
{
    int n = parent.size();
    QVector<int> order;
    order.reserve(n);
    QVector<int> visited(n, 0);

    // Iterative post-order
    for (int root = 0; root < n; ++root) {
        if (parent[root] != -1) continue;
        QVector<int> stack;
        stack.append(root);
        while (!stack.isEmpty()) {
            int node = stack.last();
            if (visited[node] == 0) {
                visited[node] = 1;
                // Push children in reverse order
                for (int i = n - 1; i >= 0; --i)
                    if (parent[i] == node) stack.append(i);
            } else if (visited[node] == 1) {
                visited[node] = 2;
                order.append(node);
                stack.removeLast();
            } else {
                stack.removeLast();
            }
        }
    }
    return order;
}

/* ---- Identify supernodes ---- */

void Cholesky8::identifySupernodes(const QVector<int>& parent, const QVector<int>& post)
{
    m_superMembership.resize(m_n, -1);
    m_supernodes.clear();

    int sn = 0;
    int i = 0;
    while (i < m_n) {
        int start = post[i];
        int end = start + 1;
        // Merge consecutive nodes if they form a fundamental supernode
        while (i + 1 < m_n) {
            int nextNode = post[i + 1];
            bool isChild = (parent[end - 1] == nextNode);
            if (isChild && nextNode == end) {
                end++;
                i++;
            } else {
                break;
            }
        }
        Supernode snDesc;
        snDesc.startRow = start;
        snDesc.endRow = end;
        // Determine column indices for the block
        for (int r = start; r < end; ++r)
            snDesc.columnIndices.append(r);
        for (int r = start; r < end; ++r)
            m_superMembership[r] = sn;
        m_supernodes.append(snDesc);
        sn++;
        i++;
    }
}

/* ---- Symbolic analysis ---- */

bool Cholesky8::symbolicAnalysis()
{
    if (m_n == 0 || m_colPtr.isEmpty()) return false;

    QVector<int> parent = eliminationTree();
    QVector<int> post = postOrder(parent);
    identifySupernodes(parent, post);

    // Compute column counts for L
    QVector<int> colCount(m_n, 0);
    for (int j = 0; j < m_n; ++j) {
        colCount[j] = 1;  // Diagonal
        for (int p = m_colPtr[j]; p < m_colPtr[j + 1]; ++p) {
            if (m_rowIdx[p] > j) colCount[j]++;
        }
    }
    // Propagate through elimination tree
    for (int j = 0; j < m_n; ++j) {
        if (parent[j] != -1)
            colCount[parent[j]] += qMax(0, colCount[j] - 1);
    }

    // Build L column pointers
    m_LcolPtr.resize(m_n + 1);
    m_LcolPtr[0] = 0;
    for (int j = 0; j < m_n; ++j)
        m_LcolPtr[j + 1] = m_LcolPtr[j] + colCount[j];

    m_LrowIdx.resize(m_LcolPtr[m_n]);
    m_Lvalues.resize(m_LcolPtr[m_n]);

    // Populate row indices (pattern)
    for (int j = 0; j < m_n; ++j) {
        int pos = m_LcolPtr[j];
        m_LrowIdx[pos++] = j;  // Diagonal first
        for (int p = m_colPtr[j]; p < m_colPtr[j + 1]; ++p) {
            if (m_rowIdx[p] > j) m_LrowIdx[pos++] = m_rowIdx[p];
        }
        std::sort(m_LrowIdx.begin() + m_LcolPtr[j] + 1, m_LrowIdx.begin() + pos);
    }

    m_symbolicDone = true;
    m_stats.numSupernodes = m_supernodes.size();
    m_stats.numNonZeros = m_LcolPtr[m_n];
    return true;
}

/* ---- Dense Cholesky for supernode block ---- */

void Cholesky8::denseCholesky(QVector<double>& block, int blockSize) const
{
    for (int j = 0; j < blockSize; ++j) {
        // Diagonal element
        double sum = block[j * blockSize + j];
        for (int k = 0; k < j; ++k) {
            double lik = block[k * blockSize + j];
            sum -= lik * lik;
        }
        if (sum <= 0.0) sum = 1e-12;  // Regularize
        block[j * blockSize + j] = qSqrt(sum);
        double invDiag = 1.0 / block[j * blockSize + j];
        // Off-diagonal elements
        for (int i = j + 1; i < blockSize; ++i) {
            sum = block[j * blockSize + i];
            for (int k = 0; k < j; ++k)
                sum -= block[k * blockSize + j] * block[k * blockSize + i];
            block[j * blockSize + i] = sum * invDiag;
        }
    }
}

/* ---- Supernodal factorization ---- */

void Cholesky8::supernodalFactorize()
{
    // Copy A values to L
    m_Lvalues.fill(0.0);
    for (int j = 0; j < m_n; ++j) {
        for (int p = m_colPtr[j]; p < m_colPtr[j + 1]; ++p) {
            int row = m_rowIdx[p];
            if (row >= j) {
                // Find position in L column
                for (int lp = m_LcolPtr[j]; lp < m_LcolPtr[j + 1]; ++lp) {
                    if (m_LrowIdx[lp] == row) {
                        m_Lvalues[lp] = m_values[p];
                        break;
                    }
                }
            }
        }
    }

    // Factorize column by column
    for (int j = 0; j < m_n; ++j) {
        // Update from previous columns
        for (int k = 0; k < j; ++k) {
            // Find L[j][k] if exists
            double ljk = 0.0;
            for (int lp = m_LcolPtr[k]; lp < m_LcolPtr[k + 1]; ++lp) {
                if (m_LrowIdx[lp] == j) { ljk = m_Lvalues[lp]; break; }
            }
            if (qFuzzyIsNull(ljk)) continue;

            // Scatter update
            for (int lp = m_LcolPtr[k]; lp < m_LcolPtr[k + 1]; ++lp) {
                int row = m_LrowIdx[lp];
                if (row < j) continue;
                for (int tp = m_LcolPtr[j]; tp < m_LcolPtr[j + 1]; ++tp) {
                    if (m_LrowIdx[tp] == row) {
                        m_Lvalues[tp] -= ljk * m_Lvalues[lp];
                        break;
                    }
                }
            }
        }

        // Diagonal
        double diag = 0.0;
        for (int lp = m_LcolPtr[j]; lp < m_LcolPtr[j + 1]; ++lp) {
            if (m_LrowIdx[lp] == j) { diag = m_Lvalues[lp]; break; }
        }
        if (diag <= 0.0) diag = 1e-12;
        double sqrtDiag = qSqrt(diag);
        double invDiag = 1.0 / sqrtDiag;

        // Scale column
        for (int lp = m_LcolPtr[j]; lp < m_LcolPtr[j + 1]; ++lp) {
            if (m_LrowIdx[lp] == j) {
                m_Lvalues[lp] = sqrtDiag;
            } else {
                m_Lvalues[lp] *= invDiag;
            }
        }
    }
}

/* ---- Factorize ---- */

bool Cholesky8::factorize()
{
    QElapsedTimer timer;
    timer.start();

    if (!m_symbolicDone) {
        if (!symbolicAnalysis()) return false;
    }

    supernodalFactorize();
    m_factorized = true;

    double elapsed = timer.elapsed();
    m_stats.matrixSize = m_n;
    m_stats.factorizationTimeMs = elapsed;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit factorizationCompleted(m_n, m_supernodes.size(), elapsed);
    return true;
}

/* ---- Solve L*x = b (forward) ---- */

QVector<double> Cholesky8::solveLower(const QVector<double>& b) const
{
    QVector<double> x = b;
    for (int j = 0; j < m_n; ++j) {
        for (int lp = m_LcolPtr[j]; lp < m_LcolPtr[j + 1]; ++lp) {
            if (m_LrowIdx[lp] == j) {
                x[j] /= m_Lvalues[lp];
            } else {
                x[m_LrowIdx[lp]] -= m_Lvalues[lp] * x[j];
            }
        }
    }
    return x;
}

/* ---- Solve L'*x = b (backward) ---- */

QVector<double> Cholesky8::solveUpper(const QVector<double>& b) const
{
    QVector<double> x = b;
    for (int j = m_n - 1; j >= 0; --j) {
        for (int lp = m_LcolPtr[j]; lp < m_LcolPtr[j + 1]; ++lp) {
            if (m_LrowIdx[lp] != j) {
                x[j] -= m_Lvalues[lp] * x[m_LrowIdx[lp]];
            }
        }
        // Divide by diagonal
        for (int lp = m_LcolPtr[j]; lp < m_LcolPtr[j + 1]; ++lp) {
            if (m_LrowIdx[lp] == j) { x[j] /= m_Lvalues[lp]; break; }
        }
    }
    return x;
}

/* ---- Full solve ---- */

QVector<double> Cholesky8::solve(const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_factorized) {
        if (!factorize()) return QVector<double>();
    }

    QVector<double> y = solveLower(b);
    QVector<double> x = solveUpper(y);

    double elapsed = timer.elapsed();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit solveCompleted(m_n, elapsed);
    return x;
}

/* ---- Accessors ---- */

QVector<Cholesky8::Supernode> Cholesky8::supernodes() const { return m_supernodes; }

/* ---- Reset ---- */

void Cholesky8::resetStatistics()
{
    m_colPtr.clear(); m_rowIdx.clear(); m_values.clear();
    m_LcolPtr.clear(); m_LrowIdx.clear(); m_Lvalues.clear();
    m_supernodes.clear(); m_superMembership.clear();
    m_symbolicDone = false; m_factorized = false;
    m_n = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
