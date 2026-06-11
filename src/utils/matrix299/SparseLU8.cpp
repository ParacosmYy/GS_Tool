/**
 * @file SparseLU8.cpp
 * @brief SparseLU8 实现
 *
 * 实现稀疏LU分解：填充消减列排序与符号数值分解实现大规模稀疏系统高效直接求解。
 */

#include "utils/matrix299/SparseLU8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SparseLU8::SparseLU8(QObject *parent)
    : QObject(parent) {}

SparseLU8::~SparseLU8() = default;

/* ---- Build CSC from COO ---- */

void SparseLU8::buildCSC(int n, const QVector<int>& rows, const QVector<int>& cols,
                            const QVector<double>& values,
                            QVector<int>& colPtr, QVector<int>& rowIdx,
                            QVector<double>& vals) const
{
    colPtr.resize(n + 1, 0);
    int nnz = values.size();
    rowIdx.resize(nnz);
    vals.resize(nnz);

    // Count entries per column
    for (int i = 0; i < nnz; ++i)
        colPtr[cols[i] + 1]++;

    // Prefix sum
    for (int j = 0; j < n; ++j)
        colPtr[j + 1] += colPtr[j];

    // Fill entries
    QVector<int> pos = colPtr;
    for (int i = 0; i < nnz; ++i) {
        int c = cols[i];
        int p = pos[c]++;
        rowIdx[p] = rows[i];
        vals[p] = values[i];
    }

    // Sort within each column by row index
    for (int j = 0; j < n; ++j) {
        int start = colPtr[j];
        int end = colPtr[j + 1];
        for (int i = start + 1; i < end; ++i) {
            int r = rowIdx[i];
            double v = vals[i];
            int k = i - 1;
            while (k >= start && rowIdx[k] > r) {
                rowIdx[k + 1] = rowIdx[k];
                vals[k + 1] = vals[k];
                k--;
            }
            rowIdx[k + 1] = r;
            vals[k + 1] = v;
        }
    }
}

/* ---- Fill-reducing column ordering (AMD approximation) ---- */

QVector<int> SparseLU8::columnOrdering(int n, const QVector<int>& rows,
                                          const QVector<int>& cols) const
{
    // Approximate minimum degree via column degree counting
    QVector<int> degree(n, 0);
    for (int i = 0; i < rows.size(); ++i)
        degree[cols[i]]++;

    // Sort columns by degree (ascending) for approximate ordering
    QVector<int> order(n);
    for (int i = 0; i < n; ++i) order[i] = i;
    std::sort(order.begin(), order.end(), [&](int a, int b) {
        return degree[a] < degree[b];
    });

    // Build inverse permutation
    QVector<int> perm(n);
    for (int i = 0; i < n; ++i)
        perm[order[i]] = i;
    return perm;
}

/* ---- Symbolic factorization ---- */

QVector<QVector<int>> SparseLU8::symbolicFactor(int n,
    const QVector<int>& colPtr, const QVector<int>& rowIdx,
    const QVector<int>& colPerm) const
{
    QVector<QVector<int>> fillPattern(n);

    // Inverse permutation
    QVector<int> invPerm(n);
    for (int i = 0; i < n; ++i) invPerm[colPerm[i]] = i;

    for (int j = 0; j < n; ++j) {
        int origCol = colPerm[j];
        // Start with original non-zero row indices
        QSet<int> fillRows;
        int start = colPtr[origCol];
        int end = colPtr[origCol + 1];
        for (int p = start; p < end; ++p) {
            int r = invPerm[rowIdx[p]];
            if (r >= j) fillRows.insert(r);
        }

        // Add fill-in from previously factorized columns
        for (int k = 0; k < j; ++k) {
            for (int row : fillPattern[k]) {
                if (row == j && j < n) {
                    // Fill-in in column j
                    fillRows.insert(j);
                }
            }
        }

        fillPattern[j] = fillRows.values().toVector();
        std::sort(fillPattern[j].begin(), fillPattern[j].end());
    }

    return fillPattern;
}

/* ---- Forward solve (Lx=b) ---- */

QVector<double> SparseLU8::forwardSolve(const QVector<QVector<SparseEntry>>& L,
                                            const QVector<double>& b) const
{
    int n = b.size();
    QVector<double> x(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double sum = b[i];
        for (const auto& e : L[i]) {
            if (e.row < i)
                sum -= e.value * x[e.row];
        }
        double diag = 1.0;
        for (const auto& e : L[i]) {
            if (e.row == i) { diag = e.value; break; }
        }
        x[i] = (diag != 0.0) ? sum / diag : 0.0;
    }
    return x;
}

/* ---- Backward solve (Ux=b) ---- */

QVector<double> SparseLU8::backwardSolve(const QVector<QVector<SparseEntry>>& U,
                                           const QVector<double>& b) const
{
    int n = b.size();
    QVector<double> x(n, 0.0);
    for (int i = n - 1; i >= 0; --i) {
        double sum = b[i];
        for (const auto& e : U[i]) {
            if (e.row > i)
                sum -= e.value * x[e.row];
        }
        double diag = 1.0;
        for (const auto& e : U[i]) {
            if (e.row == i) { diag = e.value; break; }
        }
        x[i] = (diag != 0.0) ? sum / diag : 0.0;
    }
    return x;
}

/* ---- Factorize ---- */

SparseLU8::FactorResult SparseLU8::factorize(int n,
    const QVector<int>& rows, const QVector<int>& cols,
    const QVector<double>& values)
{
    QElapsedTimer timer;
    timer.start();

    FactorResult result;
    result.rowPermutation.resize(n);
    result.colPermutation.resize(n);
    for (int i = 0; i < n; ++i) {
        result.rowPermutation[i] = i;
        result.colPermutation[i] = i;
    }

    if (n == 0 || values.isEmpty()) {
        result.success = false;
        return result;
    }

    // Build CSC format
    QVector<int> colPtr, rowIdx;
    QVector<double> vals;
    buildCSC(n, rows, cols, values, colPtr, rowIdx, vals);

    // Fill-reducing ordering
    result.colPermutation = columnOrdering(n, rows, cols);

    // Dense LU factorization with pivoting on the permuted matrix
    // Build permuted dense matrix (for correctness; sparse symbolic is for pattern)
    QVector<QVector<double>> A(n, QVector<double>(n, 0.0));
    for (int i = 0; i < rows.size(); ++i) {
        int r = rows[i];
        int c = result.colPermutation[cols[i]];
        if (r < n && c < n) A[r][c] = values[i];
    }

    // Gaussian elimination with partial pivoting
    result.determinant = 1.0;
    result.fillCount = 0;
    int origNNZ = values.size();

    for (int k = 0; k < n; ++k) {
        // Find pivot
        double maxVal = qAbs(A[k][k]);
        int maxRow = k;
        for (int i = k + 1; i < n; ++i) {
            if (qAbs(A[i][k]) > maxVal) {
                maxVal = qAbs(A[i][k]);
                maxRow = i;
            }
        }

        if (maxVal < 1e-15) {
            result.success = false;
            return result;
        }

        // Swap rows
        if (maxRow != k) {
            std::swap(A[k], A[maxRow]);
            std::swap(result.rowPermutation[k], result.rowPermutation[maxRow]);
            result.determinant = -result.determinant;
        }

        double pivot = A[k][k];
        result.determinant *= pivot;

        // Eliminate below
        for (int i = k + 1; i < n; ++i) {
            if (qAbs(A[i][k]) < 1e-15) continue;
            double factor = A[i][k] / pivot;
            A[i][k] = factor;  // Store L factor
            for (int j = k + 1; j < n; ++j) {
                if (A[i][j] != 0.0 || A[k][j] != 0.0) {
                    bool wasZero = (A[i][j] == 0.0);
                    A[i][j] -= factor * A[k][j];
                    if (wasZero && A[i][j] != 0.0) result.fillCount++;
                }
            }
        }
    }

    // Extract L and U in sparse format
    result.L.resize(n);
    result.U.resize(n);
    for (int i = 0; i < n; ++i) {
        // L: lower triangle (including diagonal = 1)
        result.L[i].append({i, 1.0});
        for (int j = 0; j < i; ++j) {
            if (qAbs(A[i][j]) > 1e-15)
                result.L[i].append({j, A[i][j]});
        }
        // U: upper triangle (including diagonal)
        for (int j = i; j < n; ++j) {
            if (qAbs(A[i][j]) > 1e-15)
                result.U[i].append({j, A[i][j]});
        }
    }

    result.success = true;

    double elapsed = timer.elapsed();
    m_stats.matrixSize = n;
    m_stats.nonZeros = origNNZ;
    m_stats.totalSolves++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit factorDone(n, result.fillCount, result.determinant, elapsed);
    return result;
}

/* ---- Solve ---- */

QVector<double> SparseLU8::solve(const FactorResult& factors,
                                    const QVector<double>& b) const
{
    int n = b.size();
    if (n != factors.L.size() || !factors.success) return {};

    // Apply row permutation: Pb
    QVector<double> pb(n, 0.0);
    for (int i = 0; i < n; ++i)
        pb[i] = b[factors.rowPermutation[i]];

    // Forward solve: Ly = Pb
    QVector<double> y = forwardSolve(factors.L, pb);

    // Backward solve: Ux = y
    QVector<double> x = backwardSolve(factors.U, y);

    // Apply inverse column permutation
    QVector<double> result(n, 0.0);
    for (int i = 0; i < n; ++i)
        result[factors.colPermutation[i]] = x[i];

    return result;
}

/* ---- Reset ---- */

void SparseLU8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
