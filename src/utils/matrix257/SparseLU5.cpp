/**
 * @file SparseLU5.cpp
 * @brief SparseLU5 实现
 *
 * 实现稀疏LU分解：阈值主元选取与超节点消去树符号/数值分解。
 */

#include "utils/matrix257/SparseLU5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SparseLU5::SparseLU5(QObject *parent) : QObject(parent) {}
SparseLU5::~SparseLU5() = default;

/* ---- Configuration ---- */

void SparseLU5::setPivotThreshold(double threshold)
{
    m_pivotThresh = qBound(0.0, threshold, 1.0);
    m_stats.pivotThreshold = m_pivotThresh;
}

/* ---- Build elimination tree ---- */

QVector<int> SparseLU5::buildEliminationTree(
    const QVector<QVector<int>>& colPattern) const
{
    int n = m_n;
    QVector<int> parent(n, -1);
    QVector<int> ancestor(n, -1);

    for (int j = 0; j < n; ++j) {
        for (int i : colPattern[j]) {
            if (i >= j) continue;
            // Path compression: find root of i in ancestor tree
            int r = i;
            while (ancestor[r] >= 0 && ancestor[r] != j) {
                int next = ancestor[r];
                ancestor[r] = j;
                r = next;
            }
            if (ancestor[r] < 0) {
                ancestor[r] = j;
                parent[r] = j;
            }
        }
    }
    return parent;
}

/* ---- Postorder traversal ---- */

QVector<int> SparseLU5::postorder(const QVector<int>& parent) const
{
    int n = parent.size();
    QVector<int> order;
    order.reserve(n);

    // Count children
    QVector<QVector<int>> children(n);
    int root = -1;
    for (int i = 0; i < n; ++i) {
        if (parent[i] < 0) root = i;
        else children[parent[i]].append(i);
    }

    // Iterative postorder using stack
    QVector<QPair<int, bool>> stack;
    if (root >= 0) stack.append({root, false});

    while (!stack.isEmpty()) {
        auto [node, visited] = stack.last();
        stack.removeLast();
        if (visited) {
            order.append(node);
        } else {
            stack.append({node, true});
            for (int c : children[node])
                stack.append({c, false});
        }
    }
    return order;
}

/* ---- Identify supernodes ---- */

void SparseLU5::identifySupernodes(const QVector<int>& postOrder)
{
    m_supernodeStart.clear();
    m_supernodeStart.append(0);

    for (int i = 1; i < postOrder.size(); ++i) {
        // Two consecutive columns form a supernode if they share
        // the same pattern below the diagonal
        bool isSuper = true;
        int col = postOrder[i];
        int prevCol = postOrder[i - 1];
        // Simple heuristic: adjacent columns with similar nonzero count
        int colNz = 0, prevNz = 0;
        for (int r = col + 1; r < m_n; ++r)
            if (qAbs(m_U[r][col]) > 1e-15) colNz++;
        for (int r = prevCol + 1; r < m_n; ++r)
            if (qAbs(m_U[r][prevCol]) > 1e-15) prevNz++;
        if (qAbs(colNz - prevNz) > 2) isSuper = false;
        if (!isSuper) m_supernodeStart.append(i);
    }
    m_supernodeStart.append(postOrder.size());
    m_stats.numSupernodes = m_supernodeStart.size() - 1;
}

/* ---- Partial (threshold) pivoting ---- */

int SparseLU5::selectPivot(int col, const QVector<QVector<double>>& A) const
{
    int bestRow = col;
    double bestVal = qAbs(A[col][col]);
    double maxInCol = bestVal;

    // Find maximum in column below diagonal
    for (int i = col + 1; i < m_n; ++i) {
        double v = qAbs(A[i][col]);
        if (v > maxInCol) maxInCol = v;
    }

    if (maxInCol < 1e-15) return -1; // Singular

    // Threshold criterion: accept pivot if |A[i][col]| >= threshold * max
    double threshold = m_pivotThresh * maxInCol;
    for (int i = col; i < m_n; ++i) {
        if (qAbs(A[i][col]) >= threshold) {
            return i; // Accept first eligible for sparsity
        }
    }
    return bestRow;
}

/* ---- Factorize ---- */

bool SparseLU5::factorize(int n, const QVector<Triplet>& entries)
{
    QElapsedTimer timer;
    timer.start();

    m_n = n;
    if (n <= 0) return false;

    // Build dense workspace from sparse entries
    QVector<QVector<double>> A(n, QVector<double>(n, 0.0));
    int nnz = 0;
    for (const auto& t : entries) {
        if (t.row >= 0 && t.row < n && t.col >= 0 && t.col < n) {
            A[t.row][t.col] = t.value;
            nnz++;
        }
    }

    m_L = QVector<QVector<double>>(n, QVector<double>(n, 0.0));
    m_U = QVector<QVector<double>>(n, QVector<double>(n, 0.0));
    m_perm.resize(n);
    m_colPerm.resize(n);
    for (int i = 0; i < n; ++i) { m_perm[i] = i; m_colPerm[i] = i; }

    // Build column pattern for elimination tree
    QVector<QVector<int>> colPattern(n);
    for (const auto& t : entries) {
        if (t.row > t.col) colPattern[t.col].append(t.row);
    }

    QVector<int> etree = buildEliminationTree(colPattern);
    QVector<int> postOrder = postorder(etree);

    // Gaussian elimination with threshold pivoting
    int numPivots = 0;
    int fillin = 0;

    for (int k = 0; k < n; ++k) {
        // Select pivot
        int pivotRow = selectPivot(k, A);
        if (pivotRow < 0) return false; // Singular

        // Swap rows if needed
        if (pivotRow != k) {
            for (int j = 0; j < n; ++j)
                std::swap(A[k][j], A[pivotRow][j]);
            std::swap(m_perm[k], m_perm[pivotRow]);
        }

        double pivot = A[k][k];
        if (qAbs(pivot) < 1e-15) return false;

        // Eliminate below
        for (int i = k + 1; i < n; ++i) {
            if (qAbs(A[i][k]) < 1e-15) continue;
            double factor = A[i][k] / pivot;
            m_L[i][k] = factor;

            for (int j = k; j < n; ++j) {
                if (qAbs(A[k][j]) > 1e-15 && qAbs(A[i][j]) < 1e-15)
                    fillin++;
                A[i][j] -= factor * A[k][j];
            }
        }
        numPivots++;
    }

    // Extract L and U
    for (int i = 0; i < n; ++i) {
        m_L[i][i] = 1.0;
        for (int j = 0; j <= i; ++j)
            m_U[j][i > j ? j : i] = A[j][i]; // Upper part
        for (int j = 0; j < n; ++j) {
            if (j <= i) m_U[i][j] = A[i][j];
        }
    }

    identifySupernodes(postOrder);

    m_stats.matrixSize = n;
    m_stats.numNonzeros = nnz;
    m_stats.numFillin = fillin;
    m_stats.numPivots = numPivots;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit factorizationCompleted(n, fillin, timer.elapsed());
    return true;
}

/* ---- Forward solve: L*y = Pb ---- */

QVector<double> SparseLU5::forwardSolve(const QVector<double>& b) const
{
    int n = m_n;
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i) {
        y[i] = b[m_perm[i]];
        for (int j = 0; j < i; ++j)
            y[i] -= m_L[i][j] * y[j];
    }
    return y;
}

/* ---- Backward solve: U*x = y ---- */

QVector<double> SparseLU5::backwardSolve(const QVector<double>& y) const
{
    int n = m_n;
    QVector<double> x(n, 0.0);
    for (int i = n - 1; i >= 0; --i) {
        x[i] = y[i];
        for (int j = i + 1; j < n; ++j)
            x[i] -= m_U[i][j] * x[j];
        if (qAbs(m_U[i][i]) < 1e-15) x[i] = 0.0;
        else x[i] /= m_U[i][i];
    }
    return x;
}

/* ---- Solve Ax = b ---- */

QVector<double> SparseLU5::solve(const QVector<double>& b) const
{
    if (b.size() != m_n) return {};
    QVector<double> y = forwardSolve(b);
    return backwardSolve(y);
}

/* ---- Diagonal ---- */

QVector<double> SparseLU5::diagonal() const
{
    QVector<double> d(m_n);
    for (int i = 0; i < m_n; ++i) d[i] = m_U[i][i];
    return d;
}

/* ---- Reset ---- */

void SparseLU5::resetStatistics()
{
    m_L.clear();
    m_U.clear();
    m_perm.clear();
    m_colPerm.clear();
    m_supernodeStart.clear();
    m_n = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
