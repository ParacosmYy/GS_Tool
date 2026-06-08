/**
 * @file SVD5.cpp
 * @brief SVD5 实现
 *
 * 实现奇异值分解：Golub-Kahan双对角化、单侧Jacobi旋转、Householder反射。
 */

#include "utils/matrix220/SVD5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SVD5::SVD5(QObject *parent) : QObject(parent) {}
SVD5::~SVD5() = default;

/* ---- Configuration ---- */

void SVD5::setParameters(int maxIter, double tol)
{
    m_maxIter = qMax(10, maxIter);
    m_tol = qMax(1e-15, tol);
}

/* ---- Off-diagonal norm ---- */

double SVD5::offDiagonalNorm(const QVector<QVector<double>>& M) const
{
    double norm = 0.0;
    int rows = M.size();
    for (int i = 0; i < rows; ++i) {
        int cols = M[i].size();
        for (int j = 0; j < cols; ++j) {
            if (i != j) norm += M[i][j] * M[i][j];
        }
    }
    return qSqrt(norm);
}

/* ---- Apply Householder reflection ---- */

void SVD5::applyHouseholder(QVector<QVector<double>>& M, int col,
                              QVector<QVector<double>>& Q, int rowStart) const
{
    int m = M.size();
    int n = (m > 0) ? M[0].size() : 0;
    if (col >= n || rowStart >= m) return;

    // Build Householder vector from column below diagonal
    int len = m - rowStart;
    if (len <= 0) return;

    double norm = 0.0;
    for (int i = rowStart; i < m; ++i)
        norm += M[i][col] * M[i][col];
    norm = qSqrt(norm);

    if (norm < 1e-15) return;

    double sign = (M[rowStart][col] >= 0) ? 1.0 : -1.0;
    double alpha = sign * norm;
    QVector<double> v(len, 0.0);
    v[0] = M[rowStart][col] + alpha;
    for (int i = 1; i < len; ++i)
        v[i] = M[rowStart + i][col];

    double vNorm2 = 0.0;
    for (double vi : v) vNorm2 += vi * vi;
    if (vNorm2 < 1e-30) return;

    double beta = 2.0 / vNorm2;

    // Apply H = I - beta * v * v^T to M
    for (int j = col; j < n; ++j) {
        double dot = 0.0;
        for (int k = 0; k < len; ++k)
            dot += v[k] * M[rowStart + k][j];
        for (int k = 0; k < len; ++k)
            M[rowStart + k][j] -= beta * v[k] * dot;
    }

    // Apply to Q
    int qCols = (Q.size() > 0) ? Q[0].size() : 0;
    for (int j = 0; j < qCols; ++j) {
        double dot = 0.0;
        for (int k = 0; k < len; ++k)
            dot += v[k] * Q[rowStart + k][j];
        for (int k = 0; k < len; ++k)
            Q[rowStart + k][j] -= beta * v[k] * dot;
    }
}

/* ---- Golub-Kahan bidiagonalization ---- */

void SVD5::bidiagonalize(QVector<QVector<double>>& B,
                           QVector<QVector<double>>& U,
                           QVector<QVector<double>>& V) const
{
    int m = B.size();
    int n = (m > 0) ? B[0].size() : 0;
    int minDim = qMin(m, n);

    for (int i = 0; i < minDim; ++i) {
        // Left Householder: zero below diagonal in column i
        applyHouseholder(B, i, U, i);

        // Right Householder: zero right of superdiagonal in row i
        if (i < n - 2) {
            // Transpose for row operation
            QVector<QVector<double>> Bt(n, QVector<double>(m));
            for (int r = 0; r < m; ++r)
                for (int c = 0; c < n; ++c)
                    Bt[c][r] = B[r][c];

            QVector<QVector<double>> Vt(n, QVector<double>(n, 0.0));
            for (int r = 0; r < n; ++r)
                for (int c = 0; c < n; ++c)
                    Vt[r][c] = V[c][r];

            applyHouseholder(Bt, i + 1, Vt, i + 1);

            // Transpose back
            for (int r = 0; r < n; ++r)
                for (int c = 0; c < n; ++c)
                    V[c][r] = Vt[r][c];
            for (int r = 0; r < m; ++r)
                for (int c = 0; c < n; ++c)
                    B[r][c] = Bt[c][r];
        }
    }
}

/* ---- Jacobi pair rotation ---- */

void SVD5::jacobiPair(int p, int q, const QVector<QVector<double>>& B,
                        double& cs, double& sn) const
{
    double bp = 0.0, bq = 0.0;
    // Extract diagonal and superdiagonal for 2x2 subproblem
    int minDim = qMin(B.size(), B[0].size());
    if (p < minDim) bp = B[p][p];
    if (q < minDim) bq = B[q][q];

    double off = 0.0;
    if (p < minDim && q < B[p].size()) off = B[p][q];

    double tau = (bq - bp) / (2.0 * off);
    double t = (tau >= 0 ? 1.0 : -1.0) / (qAbs(tau) + qSqrt(1.0 + tau * tau));
    cs = 1.0 / qSqrt(1.0 + t * t);
    sn = t * cs;
}

/* ---- One-sided Jacobi sweep ---- */

bool SVD5::jacobiSweep(QVector<QVector<double>>& B,
                         QVector<QVector<double>>& V) const
{
    int n = qMin(B.size(), B[0].size());
    double offNorm = 0.0;

    for (int p = 0; p < n - 1; ++p) {
        for (int q = p + 1; q < n; ++q) {
            // Compute off-diagonal contribution
            double apq = (p < B.size() && q < B[p].size()) ? B[p][q] : 0.0;
            double aqp = (q < B.size() && p < B[q].size()) ? B[q][p] : 0.0;
            double off = qSqrt(apq * apq + aqp * aqp);
            if (off < 1e-15) continue;

            offNorm += off * off;

            double cs, sn;
            jacobiPair(p, q, B, cs, sn);

            // Apply Givens rotation to rows p, q of B
            int cols = B[0].size();
            for (int j = 0; j < cols; ++j) {
                double bp = B[p][j], bq = B[q][j];
                B[p][j] = cs * bp + sn * bq;
                B[q][j] = -sn * bp + cs * bq;
            }

            // Apply to V
            int vCols = V[0].size();
            for (int j = 0; j < vCols; ++j) {
                double vp = V[p][j], vq = V[q][j];
                V[p][j] = cs * vp + sn * vq;
                V[q][j] = -sn * vp + cs * vq;
            }
        }
    }

    return offNorm < m_tol;
}

/* ---- Compute SVD ---- */

void SVD5::compute(const QVector<QVector<double>>& A)
{
    QElapsedTimer timer;
    timer.start();

    m_rows = A.size();
    m_cols = (m_rows > 0) ? A[0].size() : 0;
    if (m_rows == 0 || m_cols == 0) return;

    // Initialize U = I, V = I, B = A
    QVector<QVector<double>> B = A;
    QVector<QVector<double>> U(m_rows, QVector<double>(m_rows, 0.0));
    QVector<QVector<double>> V(m_cols, QVector<double>(m_cols, 0.0));
    for (int i = 0; i < m_rows; ++i) U[i][i] = 1.0;
    for (int i = 0; i < m_cols; ++i) V[i][i] = 1.0;

    // Step 1: Bidiagonalize
    bidiagonalize(B, U, V);

    // Step 2: One-sided Jacobi iterations
    int iter = 0;
    for (iter = 0; iter < m_maxIter; ++iter) {
        if (jacobiSweep(B, V)) break;
    }

    // Extract singular values from diagonal of B
    int minDim = qMin(m_rows, m_cols);
    m_S.resize(minDim);
    for (int i = 0; i < minDim; ++i) {
        m_S[i] = qAbs(B[i][i]);
    }

    // Sort singular values descending
    QVector<int> order(minDim);
    for (int i = 0; i < minDim; ++i) order[i] = i;
    std::sort(order.begin(), order.end(),
              [this](int a, int b) { return m_S[a] > m_S[b]; });

    QVector<double> sortedS(minDim);
    for (int i = 0; i < minDim; ++i) sortedS[i] = m_S[order[i]];
    m_S = sortedS;

    // Reorder U and V columns
    m_U.resize(m_rows, QVector<double>(minDim));
    m_V.resize(m_cols, QVector<double>(minDim));
    for (int i = 0; i < m_rows; ++i)
        for (int j = 0; j < minDim; ++j)
            m_U[i][j] = U[i][order[j]];
    for (int i = 0; i < m_cols; ++i)
        for (int j = 0; j < minDim; ++j)
            m_V[i][j] = V[order[j]][i];

    // Stats
    m_stats.rows = m_rows;
    m_stats.cols = m_cols;
    m_stats.iterations = iter;
    m_stats.conditionNumber = (m_S.last() > 1e-15) ? m_S.first() / m_S.last() : 0.0;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit computationCompleted(minDim, iter, m_stats.conditionNumber, timer.elapsed());
}

/* ---- Accessors ---- */

QVector<QVector<double>> SVD5::matrixU() const { return m_U; }
QVector<double> SVD5::singularValues() const { return m_S; }
QVector<QVector<double>> SVD5::matrixV() const { return m_V; }

/* ---- Reconstruct ---- */

QVector<QVector<double>> SVD5::reconstruct(int rank) const
{
    int r = (rank < 0) ? qMin(m_rows, m_cols) : qMin(rank, m_S.size());
    QVector<QVector<double>> result(m_rows, QVector<double>(m_cols, 0.0));
    for (int k = 0; k < r; ++k) {
        for (int i = 0; i < m_rows; ++i) {
            for (int j = 0; j < m_cols; ++j) {
                result[i][j] += m_U[i][k] * m_S[k] * m_V[j][k];
            }
        }
    }
    return result;
}

/* ---- Pseudo-inverse ---- */

QVector<QVector<double>> SVD5::pseudoInverse(double threshold) const
{
    int r = m_S.size();
    QVector<QVector<double>> result(m_cols, QVector<double>(m_rows, 0.0));
    for (int k = 0; k < r; ++k) {
        double invS = (m_S[k] > threshold) ? 1.0 / m_S[k] : 0.0;
        for (int i = 0; i < m_cols; ++i) {
            for (int j = 0; j < m_rows; ++j) {
                result[i][j] += m_V[i][k] * invS * m_U[j][k];
            }
        }
    }
    return result;
}

/* ---- Reset ---- */

void SVD5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_U.clear();
    m_S.clear();
    m_V.clear();
    m_rows = 0;
    m_cols = 0;
}
