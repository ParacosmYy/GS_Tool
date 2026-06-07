/**
 * @file SparseLU3.cpp
 * @brief SparseLU3 实现
 *
 * 实现稀疏LU分解：贪心近似最小度排序、层级ILU丢弃容差、稀疏三角求解。
 */

#include "utils/matrix215/SparseLU3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>
#include <cstdlib>

/* ---- Construction / Destruction ---- */

SparseLU3::SparseLU3(QObject *parent) : QObject(parent) {}
SparseLU3::~SparseLU3() = default;

/* ---- Configuration ---- */

void SparseLU3::setFillLevel(int level) { m_fillLevel = qMax(0, level); }
void SparseLU3::setDropTolerance(double tol) { m_dropTol = qMax(0.0, tol); }

/* ---- Approximate minimum degree ordering ---- */

QVector<int> SparseLU3::approxMinDegreeOrder(
    int n, const QVector<int>& rowPtr, const QVector<int>& colIdx) const
{
    QVector<int> degree(n, 0);
    QVector<bool> eliminated(n, false);

    // Compute initial degrees
    for (int i = 0; i < n; ++i) {
        QSet<int> neighbors;
        for (int j = rowPtr[i]; j < rowPtr[i + 1]; ++j) {
            int c = colIdx[j];
            if (c != i) neighbors.insert(c);
        }
        degree[i] = neighbors.size();
    }

    QVector<int> order;
    order.reserve(n);

    for (int step = 0; step < n; ++step) {
        // Find minimum degree among non-eliminated
        int minDeg = std::numeric_limits<int>::max();
        int minV = 0;
        for (int v = 0; v < n; ++v) {
            if (!eliminated[v] && degree[v] < minDeg) {
                minDeg = degree[v];
                minV = v;
            }
        }

        order.append(minV);
        eliminated[minV] = true;

        // Update degrees of neighbors (approximate)
        for (int j = rowPtr[minV]; j < rowPtr[minV + 1]; ++j) {
            int c = colIdx[j];
            if (!eliminated[c] && c != minV) {
                degree[c] = qMax(0, degree[c] - 1);
            }
        }
    }

    return order;
}

/* ---- Permute matrix ---- */

void SparseLU3::permuteMatrix(int n, const QVector<int>& rowPtr,
                               const QVector<int>& colIdx,
                               const QVector<double>& values)
{
    // Build permuted matrix using inverse permutation
    m_L.resize(n);
    m_U.resize(n);

    for (int i = 0; i < n; ++i) {
        int origRow = m_invPerm[i];
        m_L[i].clear();
        m_U[i].clear();

        for (int j = rowPtr[origRow]; j < rowPtr[origRow + 1]; ++j) {
            int origCol = colIdx[j];
            double val = values[j];
            int newCol = m_perm[origCol];

            Entry e;
            e.col = newCol;
            e.val = val;
            e.level = 0;

            if (newCol < i) {
                m_L[i].append(e);
            } else {
                m_U[i].append(e);
            }
        }

        // Sort entries by column
        auto cmp = [](const Entry& a, const Entry& b) { return a.col < b.col; };
        std::sort(m_L[i].begin(), m_L[i].end(), cmp);
        std::sort(m_U[i].begin(), m_U[i].end(), cmp);
    }
}

/* ---- Eliminate one row ---- */

void SparseLU3::eliminateRow(int k)
{
    // Ensure diagonal entry in U
    double diag = 0.0;
    for (auto& e : m_U[k]) {
        if (e.col == k) { diag = e.val; e.val = 1.0; break; }
    }

    if (qAbs(diag) < 1e-14) return;

    // Scale L row by diagonal
    for (auto& e : m_L[k]) {
        e.val /= diag;
    }

    // Update subsequent rows
    for (int i = k + 1; i < m_n; ++i) {
        // Find L[i][k]
        double lik = 0.0;
        int likLevel = 0;
        for (const auto& e : m_L[i]) {
            if (e.col == k) { lik = e.val; likLevel = e.level; break; }
        }
        if (qAbs(lik) < 1e-30) continue;

        // Update U[i] = U[i] - lik * U[k]
        for (const auto& uk : m_U[k]) {
            int col = uk.col;
            if (col <= k) continue;

            double contribution = lik * uk.val;
            int newLevel = qMax(likLevel, uk.level) + 1;

            // Apply ILU drop tolerance
            if (newLevel > m_fillLevel || qAbs(contribution) < m_dropTol)
                continue;

            // Find or insert entry
            bool found = false;
            for (auto& e : m_U[i]) {
                if (e.col == col) {
                    e.val -= contribution;
                    e.level = qMin(e.level, newLevel);
                    found = true;
                    break;
                }
            }
            if (!found) {
                Entry ne;
                ne.col = col;
                ne.val = -contribution;
                ne.level = newLevel;
                m_U[i].append(ne);
            }
        }
    }
}

/* ---- Factorize ---- */

bool SparseLU3::factorize(int n, const QVector<int>& rowPtr,
                           const QVector<int>& colIdx,
                           const QVector<double>& values)
{
    QElapsedTimer timer;
    timer.start();

    m_n = n;
    if (n <= 0) return false;

    // Compute ordering
    m_perm = approxMinDegreeOrder(n, rowPtr, colIdx);
    m_invPerm.resize(n);
    for (int i = 0; i < n; ++i) m_invPerm[m_perm[i]] = i;

    // Permute and copy matrix
    permuteMatrix(n, rowPtr, colIdx, values);

    int nnzOrig = 0;
    for (int i = 0; i < n; ++i) nnzOrig += (rowPtr[i + 1] - rowPtr[i]);

    // Gaussian elimination
    for (int k = 0; k < n - 1; ++k) {
        eliminateRow(k);
    }

    // Count non-zeros
    int nnzL = 0, nnzU = 0;
    for (int i = 0; i < n; ++i) {
        nnzL += m_L[i].size();
        nnzU += m_U[i].size();
    }

    m_stats.matrixSize = n;
    m_stats.nnzOriginal = nnzOrig;
    m_stats.nnzL = nnzL;
    m_stats.nnzU = nnzU;
    m_stats.fillLevel = m_fillLevel;
    m_stats.dropTolerance = m_dropTol;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit factorizationCompleted(nnzL, nnzU, timer.elapsed());

    return true;
}

/* ---- Solve lower triangular ---- */

QVector<double> SparseLU3::solveLower(const QVector<double>& b) const
{
    int n = m_n;
    QVector<double> y(n, 0.0);

    for (int i = 0; i < n; ++i) {
        double sum = b[i];
        for (const auto& e : m_L[i]) {
            sum -= e.val * y[e.col];
        }
        y[i] = sum;  // L has unit diagonal
    }
    return y;
}

/* ---- Solve upper triangular ---- */

QVector<double> SparseLU3::solveUpper(const QVector<double>& y) const
{
    int n = m_n;
    QVector<double> x(n, 0.0);

    for (int i = n - 1; i >= 0; --i) {
        double diag = 1.0;
        double sum = y[i];
        for (const auto& e : m_U[i]) {
            if (e.col == i) diag = e.val;
            else if (e.col > i) sum -= e.val * x[e.col];
        }
        x[i] = (qAbs(diag) > 1e-30) ? sum / diag : 0.0;
    }
    return x;
}

/* ---- Full solve ---- */

QVector<double> SparseLU3::solve(const QVector<double>& b) const
{
    if (b.size() != m_n) return {};

    // Apply permutation to b
    QVector<double> pb(m_n, 0.0);
    for (int i = 0; i < m_n; ++i)
        pb[i] = b[m_invPerm[i]];

    // Forward and back substitution
    auto y = solveLower(pb);
    auto x = solveUpper(y);

    // Apply inverse permutation
    QVector<double> result(m_n, 0.0);
    for (int i = 0; i < m_n; ++i)
        result[m_perm[i]] = x[i];

    return result;
}

/* ---- Get permutation ---- */

QVector<int> SparseLU3::permutation() const { return m_perm; }

/* ---- Reset ---- */

void SparseLU3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_L.clear();
    m_U.clear();
    m_perm.clear();
    m_invPerm.clear();
    m_n = 0;
}
