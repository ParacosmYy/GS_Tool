/**
 * @file SparseQR.cpp
 * @brief SparseQR 实现
 *
 * 实现稀疏QR分解：列消去树、符号/数值分解、Householder反射、丢弃容差。
 */

#include "utils/matrix183/SparseQR.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SparseQR::SparseQR(QObject *parent) : QObject(parent) {}
SparseQR::~SparseQR() = default;

/* ---- Configuration ---- */

void SparseQR::setDropTolerance(double tol) { m_dropTol = qMax(0.0, tol); }

/* ---- Convert sparse to dense ---- */

QVector<QVector<double>> SparseQR::toDense(const SparseMatrix& A) const
{
    QVector<QVector<double>> D(A.rows, QVector<double>(A.cols, 0.0));
    for (int j = 0; j < A.cols; ++j) {
        for (int p = A.colPtr[j]; p < A.colPtr[j + 1]; ++p) {
            if (A.rowIdx[p] >= 0 && A.rowIdx[p] < A.rows)
                D[A.rowIdx[p]][j] = A.values[p];
        }
    }
    return D;
}

/* ---- Column elimination tree ---- */

QVector<int> SparseQR::eliminationTree(const SparseMatrix& A) const
{
    int n = A.cols;
    QVector<int> parent(n, -1);
    QVector<int> ancestor(n, -1);

    for (int j = 0; j < n; ++j) {
        for (int p = A.colPtr[j]; p < A.colPtr[j + 1]; ++p) {
            int i = A.rowIdx[p];
            if (i < 0 || i >= n) continue; // Skip rows beyond square part
            // Path compression: find root of i in elimination tree
            int q = ancestor[i];
            while (q != -1 && q < j) {
                int next = ancestor[q];
                ancestor[q] = j;
                if (next == -1) { parent[q] = j; }
                q = next;
            }
            if (q == -1) {
                parent[i] = j;
                ancestor[i] = j;
            }
        }
        ancestor[j] = -1;
    }
    return parent;
}

/* ---- Symbolic analysis ---- */

void SparseQR::symbolicAnalysis(const SparseMatrix& A)
{
    m_etree = eliminationTree(A);
    m_perm.resize(A.cols);
    for (int i = 0; i < A.cols; ++i) m_perm[i] = i;
}

/* ---- Dense Householder QR ---- */

void SparseQR::denseQR(QVector<QVector<double>>& A, QVector<double>& tau)
{
    int m = A.size();
    if (m == 0) return;
    int n = A[0].size();
    int minDim = qMin(m, n);
    tau.resize(minDim);
    m_householder.resize(minDim);

    for (int k = 0; k < minDim; ++k) {
        // Extract column k below diagonal
        int len = m - k;
        QVector<double> v(len);
        double norm = 0.0;
        for (int i = 0; i < len; ++i) {
            v[i] = A[k + i][k];
            norm += v[i] * v[i];
        }
        norm = qSqrt(norm);

        if (norm < 1e-15) {
            tau[k] = 0.0;
            m_householder[k] = v;
            continue;
        }

        // Compute Householder vector
        double alpha = (v[0] >= 0) ? norm : -norm;
        v[0] += alpha;
        double vnorm = 0.0;
        for (double x : v) vnorm += x * x;
        tau[k] = 2.0 * v[0] * v[0] / vnorm;
        if (vnorm > 1e-15) {
            double s = qSqrt(vnorm);
            for (auto& x : v) x /= s;
        }

        // Apply reflection: A[k:m, k:n] -= tau * v * (v^T * A[k:m, k:n])
        for (int j = k; j < n; ++j) {
            double dot = 0.0;
            for (int i = 0; i < len; ++i)
                dot += v[i] * A[k + i][j];
            for (int i = 0; i < len; ++i)
                A[k + i][j] -= tau[k] * v[i] * dot;
        }

        m_householder[k] = v;
    }
}

/* ---- Apply Q^T to vector ---- */

QVector<double> SparseQR::applyQTranspose(const QVector<double>& b) const
{
    int m = m_rows;
    int n = qMin(m_rows, m_cols);
    QVector<double> result = b;

    for (int k = 0; k < n; ++k) {
        const auto& v = m_householder[k];
        int len = v.size();
        double t = m_tau[k];

        double dot = 0.0;
        for (int i = 0; i < len; ++i)
            dot += v[i] * result[k + i];
        for (int i = 0; i < len; ++i)
            result[k + i] -= t * v[i] * dot;
    }
    return result;
}

/* ---- Back-solve R x = y ---- */

QVector<double> SparseQR::backSolve(const QVector<double>& y) const
{
    int n = m_R.cols;
    QVector<double> x(n, 0.0);

    // Use dense form of R for back-substitution
    auto Rdense = toDense(m_R);
    for (int i = n - 1; i >= 0; --i) {
        double sum = y[i];
        for (int j = i + 1; j < n; ++j)
            sum -= Rdense[i][j] * x[j];
        if (qAbs(Rdense[i][i]) > 1e-15)
            x[i] = sum / Rdense[i][i];
    }
    return x;
}

/* ---- Main factorize ---- */

bool SparseQR::factorize(const SparseMatrix& A)
{
    QElapsedTimer timer;
    timer.start();

    m_rows = A.rows;
    m_cols = A.cols;

    if (A.rows == 0 || A.cols == 0) return false;

    // Symbolic analysis
    symbolicAnalysis(A);

    // Convert to dense and perform Householder QR
    auto dense = toDense(A);
    int nnzOrig = 0;
    for (int j = 0; j < A.cols; ++j)
        nnzOrig += A.colPtr[j + 1] - A.colPtr[j];

    denseQR(dense, m_tau);

    // Extract R (upper triangular) and apply drop tolerance
    int n = A.cols;
    m_R.rows = A.rows;
    m_R.cols = n;
    m_R.colPtr.resize(n + 1, 0);

    // Count nonzeros in R after dropping
    for (int j = 0; j < n; ++j) {
        for (int i = 0; i <= qMin(j, A.rows - 1); ++i) {
            if (qAbs(dense[i][j]) > m_dropTol)
                m_R.colPtr[j + 1]++;
        }
    }
    // Prefix sum
    for (int j = 0; j < n; ++j)
        m_R.colPtr[j + 1] += m_R.colPtr[j];

    int nnz = m_R.colPtr[n];
    m_R.rowIdx.resize(nnz);
    m_R.values.resize(nnz);

    // Fill R
    QVector<int> pos(n, 0);
    for (int j = 0; j < n; ++j) {
        for (int i = 0; i <= qMin(j, A.rows - 1); ++i) {
            if (qAbs(dense[i][j]) > m_dropTol) {
                int idx = m_R.colPtr[j] + pos[j];
                m_R.rowIdx[idx] = i;
                m_R.values[idx] = dense[i][j];
                pos[j]++;
            }
        }
    }

    m_stats.totalFactorizations++;
    m_stats.matrixRows = A.rows;
    m_stats.matrixCols = A.cols;
    m_stats.nnzOriginal = nnzOrig;
    m_stats.nnzFactor = nnz;
    m_stats.dropRatio = (nnzOrig > 0) ? static_cast<double>(nnz) / nnzOrig : 0.0;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFactorizations;

    emit factorizationCompleted(A.rows, A.cols, nnz);
    return true;
}

/* ---- Solve Ax = b ---- */

QVector<double> SparseQR::solve(const QVector<double>& b) const
{
    if (b.size() != m_rows) return {};

    // Apply Q^T * b
    auto Qtb = applyQTranspose(b);

    // Back-solve R * x = Q^T * b
    return backSolve(Qtb);
}

/* ---- Get R matrix ---- */

SparseQR::SparseMatrix SparseQR::factorR() const { return m_R; }

/* ---- Reset ---- */

void SparseQR::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
