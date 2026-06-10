/**
 * @file SparseLU7.cpp
 * @brief SparseLU7 实现
 *
 * 实现稀疏LU分解：列近似最小度排序与块三角形式的可约稀疏系统。
 */

#include "utils/matrix285/SparseLU7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SparseLU7::SparseLU7(QObject *parent)
    : QObject(parent) {}

SparseLU7::~SparseLU7() = default;

/* ---- Configuration ---- */

void SparseLU7::setSize(int n)
{
    m_n = qMax(1, n);
    m_entries.clear();
}

void SparseLU7::setEntries(const QVector<SparseEntry>& entries)
{
    m_entries = entries;
    m_stats.nnzOriginal = entries.size();
}

/* ---- Convert sparse to dense ---- */

void SparseLU7::toDense(const QVector<SparseEntry>& entries, int n)
{
    m_dense = QVector<QVector<double>>(n, QVector<double>(n, 0.0));
    for (const auto& e : entries) {
        if (e.row >= 0 && e.row < n && e.col >= 0 && e.col < n)
            m_dense[e.row][e.col] = e.value;
    }
}

/* ---- Column approximate minimum degree ordering ---- */

QVector<int> SparseLU7::colamdOrder(const QVector<QVector<double>>& mat) const
{
    int n = mat.size();
    QVector<int> order(n);
    for (int i = 0; i < n; ++i) order[i] = i;

    // Approximate minimum degree: sort columns by degree (non-zero count)
    QVector<int> colDegree(n, 0);
    for (int j = 0; j < n; ++j)
        for (int i = 0; i < n; ++i)
            if (qAbs(mat[i][j]) > 1e-15) colDegree[j]++;

    // Greedy minimum degree ordering
    QVector<bool> used(n, false);
    for (int step = 0; step < n; ++step) {
        int best = -1;
        int bestDeg = n + 1;
        for (int j = 0; j < n; ++j) {
            if (!used[j] && colDegree[j] < bestDeg) {
                bestDeg = colDegree[j];
                best = j;
            }
        }
        if (best >= 0) {
            order[step] = best;
            used[best] = true;
            // Update degrees (simulate fill-in approximation)
            for (int j = 0; j < n; ++j) {
                if (!used[j] && qAbs(mat[best][j]) > 1e-15)
                    colDegree[j]++;
            }
        }
    }
    return order;
}

/* ---- Partial pivoting with row permutation ---- */

QVector<int> SparseLU7::partialPivot(QVector<QVector<double>>& A, int n)
{
    QVector<int> rowP(n);
    for (int i = 0; i < n; ++i) rowP[i] = i;

    for (int k = 0; k < n; ++k) {
        // Find pivot in column k
        int maxRow = k;
        double maxVal = qAbs(A[k][k]);
        for (int i = k + 1; i < n; ++i) {
            if (qAbs(A[i][k]) > maxVal) {
                maxVal = qAbs(A[i][k]);
                maxRow = i;
            }
        }
        if (maxRow != k) {
            std::swap(A[k], A[maxRow]);
            std::swap(rowP[k], rowP[maxRow]);
        }
    }
    return rowP;
}

/* ---- LU factorization with COLAMD ordering ---- */

SparseLU7::LUResult SparseLU7::factorize()
{
    QElapsedTimer timer;
    timer.start();

    LUResult result;
    int n = m_n;
    if (n == 0 || m_entries.isEmpty()) return result;

    // Apply COLAMD column ordering
    toDense(m_entries, n);
    QVector<int> colP = colamdOrder(m_dense);

    // Permute columns: A*Q
    QVector<QVector<double>> AQ(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            AQ[i][j] = m_dense[i][colP[j]];

    // Partial pivoting for row permutation
    QVector<int> rowP = partialPivot(AQ, n);

    // LU decomposition in-place on PAQ
    double det = 1.0;
    for (int k = 0; k < n; ++k) {
        if (qAbs(AQ[k][k]) < 1e-15) {
            result.success = false;
            return result;
        }
        det *= AQ[k][k];

        for (int i = k + 1; i < n; ++i) {
            AQ[i][k] /= AQ[k][k];
            for (int j = k + 1; j < n; ++j)
                AQ[i][j] -= AQ[i][k] * AQ[k][j];
        }
    }

    // Extract L and U as sparse entries
    int nnzL = 0, nnzU = 0;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (qAbs(AQ[i][j]) > 1e-15) {
                SparseEntry e;
                e.row = i;
                e.col = j;
                e.value = AQ[i][j];
                if (i > j) {
                    result.L.append(e);
                    nnzL++;
                } else {
                    result.U.append(e);
                    nnzU++;
                }
            }
        }
    }

    // Add unit diagonal to L
    for (int i = 0; i < n; ++i) {
        SparseEntry e;
        e.row = i;
        e.col = i;
        e.value = 1.0;
        result.L.append(e);
        nnzL++;
    }

    result.rowPerm = rowP;
    result.colPerm = colP;
    result.determinant = det;
    result.success = true;

    m_lastLU = result;

    double elapsed = timer.elapsed();
    m_stats.matrixSize = n;
    m_stats.nnzL = nnzL;
    m_stats.nnzU = nnzU;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit factorizeDone(n, nnzL, nnzU, det, elapsed);

    return result;
}

/* ---- Forward solve L*y = Pb ---- */

QVector<double> SparseLU7::forwardSolve(const QVector<QVector<double>>& L,
                                          const QVector<int>& rowP,
                                          const QVector<double>& b) const
{
    int n = L.size();
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i) {
        y[i] = b[rowP[i]];
        for (int j = 0; j < i; ++j)
            y[i] -= L[i][j] * y[j];
    }
    return y;
}

/* ---- Back solve U*Qx = y ---- */

QVector<double> SparseLU7::backSolve(const QVector<QVector<double>>& U,
                                       const QVector<int>& colP,
                                       const QVector<double>& y) const
{
    int n = U.size();
    QVector<double> x(n, 0.0);
    for (int i = n - 1; i >= 0; --i) {
        x[i] = y[i];
        for (int j = i + 1; j < n; ++j)
            x[i] -= U[i][j] * x[j];
        if (qAbs(U[i][i]) > 1e-15)
            x[i] /= U[i][i];
    }
    // Inverse column permutation
    QVector<double> result(n, 0.0);
    for (int i = 0; i < n; ++i)
        result[colP[i]] = x[i];
    return result;
}

/* ---- Solve Ax = b ---- */

QVector<double> SparseLU7::solve(const QVector<double>& b) const
{
    int n = m_n;
    if (n == 0 || !m_lastLU.success || b.size() != n) return {};

    // Reconstruct dense L and U from sparse entries
    QVector<QVector<double>> L(n, QVector<double>(n, 0.0));
    QVector<QVector<double>> U(n, QVector<double>(n, 0.0));
    for (const auto& e : m_lastLU.L) L[e.row][e.col] = e.value;
    for (const auto& e : m_lastLU.U) U[e.row][e.col] = e.value;

    QVector<double> y = forwardSolve(L, m_lastLU.rowPerm, b);
    return backSolve(U, m_lastLU.colPerm, y);
}

/* ---- Block triangular form (Dulmage-Mendelsohn) ---- */

QVector<QVector<int>> SparseLU7::blockTriangularForm() const
{
    int n = m_n;
    if (n == 0) return {};

    // Simple BTF via strong connectivity of directed graph
    // Build adjacency from non-zero pattern
    QVector<QSet<int>> adj(n);
    for (const auto& e : m_entries) {
        if (e.row >= 0 && e.row < n && e.col >= 0 && e.col < n)
            if (qAbs(e.value) > 1e-15)
                adj[e.row].insert(e.col);
    }

    // Tarjan's strongly connected components
    QVector<int> index(n, -1), lowlink(n, -1);
    QVector<bool> onStack(n, false);
    QVector<int> stack;
    QVector<QVector<int>> sccs;
    int idx = 0;

    // Recursive SCC via explicit stack to avoid deep recursion
    QVector<QPair<int, int>> work;  // (vertex, neighbor index)
    for (int start = 0; start < n; ++start) {
        if (index[start] >= 0) continue;
        work.append({start, 0});
        while (!work.isEmpty()) {
            int v = work.back().first;
            int& ni = work.back().second;
            if (index[v] < 0) {
                index[v] = lowlink[v] = idx++;
                stack.append(v);
                onStack[v] = true;
            }
            bool found = false;
            QList<int> neighbors = adj[v].values();
            std::sort(neighbors.begin(), neighbors.end());
            while (ni < neighbors.size()) {
                int w = neighbors[ni++];
                if (index[w] < 0) {
                    work.append({w, 0});
                    found = true;
                    break;
                } else if (onStack[w]) {
                    lowlink[v] = qMin(lowlink[v], lowlink[w]);
                }
            }
            if (found) continue;
            if (lowlink[v] == index[v]) {
                QVector<int> comp;
                int w;
                do {
                    w = stack.takeLast();
                    onStack[w] = false;
                    comp.append(w);
                } while (w != v);
                sccs.append(comp);
            }
            work.removeLast();
            if (!work.isEmpty())
                lowlink[work.back().first] = qMin(lowlink[work.back().first], lowlink[v]);
        }
    }
    return sccs;
}

/* ---- Reset ---- */

void SparseLU7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_entries.clear();
    m_dense.clear();
    m_lastLU = LUResult{};
}
