/**
 * @file SVD8.cpp
 * @brief SVD8 实现
 *
 * 实现奇异值分解：随机化范围搜索与幂迭代截断低秩近似。
 */

#include "utils/matrix262/SVD8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>
#include <random>

/* ---- Construction / Destruction ---- */

SVD8::SVD8(QObject *parent) : QObject(parent) {}
SVD8::~SVD8() = default;

/* ---- Configuration ---- */

void SVD8::setTargetRank(int rank) { m_targetRank = qMax(1, rank); }
void SVD8::setPowerIterations(int iters) { m_powerIters = qMax(0, iters); }
void SVD8::setOversampling(int p) { m_oversampling = qMax(0, p); }

/* ---- Random Gaussian matrix ---- */

QVector<QVector<double>> SVD8::randomGaussian(int rows, int cols) const
{
    std::mt19937 rng(42);
    std::normal_distribution<double> dist(0.0, 1.0);
    QVector<QVector<double>> mat(rows, QVector<double>(cols));
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            mat[i][j] = dist(rng);
    return mat;
}

/* ---- Matrix multiply ---- */

QVector<QVector<double>> SVD8::matMul(const QVector<QVector<double>>& A,
                                        const QVector<QVector<double>>& B) const
{
    int m = A.size();
    int p = B.size();
    if (m == 0 || p == 0) return {};
    int n = B[0].size();
    int k = A[0].size();

    QVector<QVector<double>> C(m, QVector<double>(n, 0.0));
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < n; ++j)
            for (int l = 0; l < k; ++l)
                C[i][j] += A[i][l] * B[l][j];
    return C;
}

/* ---- QR decomposition (modified Gram-Schmidt) ---- */

void SVD8::qrDecompose(const QVector<QVector<double>>& A,
                        QVector<QVector<double>>& Q,
                        QVector<QVector<double>>& R) const
{
    int m = A.size();
    if (m == 0) return;
    int n = A[0].size();
    int minDim = qMin(m, n);

    Q = QVector<QVector<double>>(m, QVector<double>(minDim, 0.0));
    R = QVector<QVector<double>>(minDim, QVector<double>(n, 0.0));

    // Transpose A to column-major for easier processing
    QVector<QVector<double>> cols(n, QVector<double>(m));
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < n; ++j)
            cols[j][i] = A[i][j];

    QVector<QVector<double>> qCols(minDim, QVector<double>(m));

    for (int k = 0; k < minDim; ++k) {
        QVector<double> v = cols[k];
        for (int i = 0; i < k; ++i) {
            double dot = 0.0;
            for (int j = 0; j < m; ++j) dot += qCols[i][j] * v[j];
            R[i][k] = dot;
            for (int j = 0; j < m; ++j) v[j] -= dot * qCols[i][j];
        }
        double norm = 0.0;
        for (double val : v) norm += val * val;
        norm = qSqrt(norm);
        R[k][k] = norm;
        if (norm > 1e-14) {
            for (int j = 0; j < m; ++j) qCols[k][j] = v[j] / norm;
        }
    }

    // Store Q
    for (int i = 0; i < m; ++i)
        for (int k = 0; k < minDim; ++k)
            Q[i][k] = qCols[k][i];
}

/* ---- Randomized range finder ---- */

QVector<QVector<double>> SVD8::rangeFinder(const QVector<QVector<double>>& A, int rank) const
{
    int m = A.size();
    int n = A[0].size();
    int k = rank + m_oversampling;

    // Random projection: Y = A * Omega
    auto omega = randomGaussian(n, k);
    QVector<QVector<double>> Y(m, QVector<double>(k, 0.0));
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < k; ++j)
            for (int l = 0; l < n; ++l)
                Y[i][j] += A[i][l] * omega[l][j];

    // Power iteration for better approximation
    for (int iter = 0; iter < m_powerIters; ++iter) {
        QVector<QVector<double>> Q, R;
        qrDecompose(Y, Q, R);
        // Y = A * (A^T * Q)
        auto ATQ = QVector<QVector<double>>(n, QVector<double>(Q[0].size(), 0.0));
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < static_cast<int>(Q[0].size()); ++j)
                for (int l = 0; l < m; ++l)
                    ATQ[i][j] += A[l][i] * Q[l][j];

        Y = QVector<QVector<double>>(m, QVector<double>(ATQ[0].size(), 0.0));
        for (int i = 0; i < m; ++i)
            for (int j = 0; j < static_cast<int>(ATQ[0].size()); ++j)
                for (int l = 0; l < n; ++l)
                    Y[i][j] += A[i][l] * ATQ[l][j];
    }

    QVector<QVector<double>> Q, R;
    qrDecompose(Y, Q, R);
    return Q;
}

/* ---- Jacobi eigenvalue decomposition ---- */

void SVD8::jacobiEigen(const QVector<QVector<double>>& S,
                        QVector<double>& eigenvalues,
                        QVector<QVector<double>>& eigenvectors) const
{
    int n = S.size();
    QVector<QVector<double>> A = S;
    eigenvectors = QVector<QVector<double>>(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) eigenvectors[i][i] = 1.0;

    const int maxIter = 100;
    for (int iter = 0; iter < maxIter; ++iter) {
        // Find largest off-diagonal element
        double maxOff = 0.0;
        int pi = 0, pj = 1;
        for (int i = 0; i < n; ++i)
            for (int j = i + 1; j < n; ++j)
                if (qFabs(A[i][j]) > maxOff) { maxOff = qFabs(A[i][j]); pi = i; pj = j; }
        if (maxOff < 1e-12) break;

        // Compute rotation
        double theta = (A[pj][pj] - A[pi][pi]) / (2.0 * A[pi][pj]);
        double t = (theta >= 0 ? 1.0 : -1.0) / (qFabs(theta) + qSqrt(1.0 + theta * theta));
        double c = 1.0 / qSqrt(1.0 + t * t);
        double s = t * c;

        // Apply Givens rotation
        for (int i = 0; i < n; ++i) {
            double api = A[i][pi], apj = A[i][pj];
            A[i][pi] = c * api - s * apj;
            A[i][pj] = s * api + c * apj;
        }
        for (int j = 0; j < n; ++j) {
            double api = A[pi][j], apj = A[pj][j];
            A[pi][j] = c * api - s * apj;
            A[pj][j] = s * api + c * apj;
        }
        for (int i = 0; i < n; ++i) {
            double eip = eigenvectors[i][pi], ejp = eigenvectors[i][pj];
            eigenvectors[i][pi] = c * eip - s * ejp;
            eigenvectors[i][pj] = s * eip + c * ejp;
        }
    }

    eigenvalues.resize(n);
    for (int i = 0; i < n; ++i) eigenvalues[i] = A[i][i];
}

/* ---- Main SVD computation ---- */

bool SVD8::compute(const QVector<QVector<double>>& A)
{
    QElapsedTimer timer;
    timer.start();

    m_rows = A.size();
    if (m_rows == 0) return false;
    m_cols = A[0].size();
    if (m_cols == 0) return false;

    int rank = qMin(m_targetRank, qMin(m_rows, m_cols));

    // Step 1: Randomized range finder
    auto Q = rangeFinder(A, rank);
    int qCols = Q[0].size();

    // Step 2: Project A onto Q: B = Q^T * A
    QVector<QVector<double>> B(qCols, QVector<double>(m_cols, 0.0));
    for (int i = 0; i < qCols; ++i)
        for (int j = 0; j < m_cols; ++j)
            for (int l = 0; l < m_rows; ++l)
                B[i][j] += Q[l][i] * A[l][j];

    // Step 3: Compute SVD of small matrix B via eigendecomposition of B*B^T
    auto BBT = QVector<QVector<double>>(qCols, QVector<double>(qCols, 0.0));
    for (int i = 0; i < qCols; ++i)
        for (int j = 0; j < qCols; ++j)
            for (int l = 0; l < m_cols; ++l)
                BBT[i][j] += B[i][l] * B[j][l];

    QVector<double> eigenvalues;
    QVector<QVector<double>> eigvecs;
    jacobiEigen(BBT, eigenvalues, eigvecs);

    // Sort by descending eigenvalue
    QVector<int> order(qCols);
    for (int i = 0; i < qCols; ++i) order[i] = i;
    std::sort(order.begin(), order.end(), [&](int a, int b) {
        return eigenvalues[a] > eigenvalues[b];
    });

    int finalRank = qMin(rank, qCols);
    m_S.resize(finalRank);
    m_Vt = QVector<QVector<double>>(finalRank, QVector<double>(m_cols, 0.0));
    m_U = QVector<QVector<double>>(m_rows, QVector<double>(finalRank, 0.0));

    for (int k = 0; k < finalRank; ++k) {
        int idx = order[k];
        double sv = qSqrt(qMax(0.0, eigenvalues[idx]));
        m_S[k] = sv;

        // V_k = B^T * eigvec_k / sigma_k
        if (sv > 1e-14) {
            for (int j = 0; j < m_cols; ++j) {
                double sum = 0.0;
                for (int i = 0; i < qCols; ++i)
                    sum += B[i][j] * eigvecs[i][idx];
                m_Vt[k][j] = sum / sv;
            }
        }

        // U_k = Q * eigvec_k
        for (int i = 0; i < m_rows; ++i) {
            double sum = 0.0;
            for (int l = 0; l < qCols; ++l)
                sum += Q[i][l] * eigvecs[l][idx];
            m_U[i][k] = sum;
        }
    }

    double elapsed = timer.elapsed();
    m_stats.inputRows = m_rows;
    m_stats.inputCols = m_cols;
    m_stats.targetRank = finalRank;
    m_stats.powerIterations = m_powerIters;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit svdCompleted(finalRank, m_S.isEmpty() ? 0.0 : m_S[0], elapsed);
    return true;
}

/* ---- Accessors ---- */

QVector<QVector<double>> SVD8::matrixU() const { return m_U; }
QVector<double> SVD8::singularValues() const { return m_S; }
QVector<QVector<double>> SVD8::matrixVt() const { return m_Vt; }

QVector<QVector<double>> SVD8::reconstruct() const
{
    int r = m_S.size();
    if (r == 0) return {};
    QVector<QVector<double>> result(m_rows, QVector<double>(m_cols, 0.0));
    for (int k = 0; k < r; ++k)
        for (int i = 0; i < m_rows; ++i)
            for (int j = 0; j < m_cols; ++j)
                result[i][j] += m_U[i][k] * m_S[k] * m_Vt[k][j];
    return result;
}

/* ---- Reset ---- */

void SVD8::resetStatistics()
{
    m_U.clear();
    m_S.clear();
    m_Vt.clear();
    m_rows = 0;
    m_cols = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
