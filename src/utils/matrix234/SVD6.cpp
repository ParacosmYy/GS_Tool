/**
 * @file SVD6.cpp
 * @brief SVD6 实现
 *
 * 实现SVD奇异值分解：Golub-Kahan双对角化与隐式零位移QR迭代。
 */

#include "utils/matrix234/SVD6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SVD6::SVD6(QObject *parent) : QObject(parent) {}
SVD6::~SVD6() = default;

/* ---- Householder reflection ---- */

void SVD6::householder(const QVector<double>& x, QVector<double>& v,
                         double& beta) const
{
    int n = x.size();
    if (n == 0) { beta = 0.0; return; }

    double sigma = 0.0;
    v = x;
    for (int i = 1; i < n; ++i)
        sigma += x[i] * x[i];

    if (sigma < 1e-30) { beta = 0.0; return; }

    double mu = qSqrt(x[0] * x[0] + sigma);
    v[0] = (x[0] <= 0.0) ? (x[0] - mu) : (-sigma / (x[0] + mu));

    beta = 2.0 * v[0] * v[0] / (sigma + v[0] * v[0]);
    for (auto& vi : v) vi /= v[0];
}

/* ---- Apply Householder from left ---- */

void SVD6::applyHouseholderLeft(QVector<QVector<double>>& M, int row,
                                  int col, const QVector<double>& v,
                                  double beta)
{
    int rows = M.size();
    int cols = (rows > 0) ? M[0].size() : 0;
    int m = rows - row;
    int n = cols - col;

    // w = beta * v^T * M
    QVector<double> w(n, 0.0);
    for (int j = 0; j < n; ++j)
        for (int i = 0; i < m; ++i)
            w[j] += v[i] * M[row + i][col + j];
    for (int j = 0; j < n; ++j) w[j] *= beta;

    // M = M - v * w^T
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < n; ++j)
            M[row + i][col + j] -= v[i] * w[j];
}

/* ---- Apply Householder from right ---- */

void SVD6::applyHouseholderRight(QVector<QVector<double>>& M, int row,
                                   int col, const QVector<double>& v,
                                   double beta)
{
    int rows = M.size();
    int cols = (rows > 0) ? M[0].size() : 0;
    int m = rows - row;
    int n = cols - col;

    QVector<double> w(m, 0.0);
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < n; ++j)
            w[i] += M[row + i][col + j] * v[j];
    for (int i = 0; i < m; ++i) w[i] *= beta;

    for (int i = 0; i < m; ++i)
        for (int j = 0; j < n; ++j)
            M[row + i][col + j] -= w[i] * v[j];
}

/* ---- Golub-Kahan bidiagonalization ---- */

void SVD6::bidiagonalize(QVector<QVector<double>>& B,
                           QVector<QVector<double>>& Q,
                           QVector<QVector<double>>& P)
{
    int m = B.size();
    int n = (m > 0) ? B[0].size() : 0;
    int minDim = qMin(m, n);

    Q = QVector<QVector<double>>(m, QVector<double>(m, 0.0));
    P = QVector<QVector<double>>(n, QVector<double>(n, 0.0));
    for (int i = 0; i < m; ++i) Q[i][i] = 1.0;
    for (int i = 0; i < n; ++i) P[i][i] = 1.0;

    for (int k = 0; k < minDim; ++k) {
        // Left Householder: zero below diagonal in column k
        QVector<double> col(m - k);
        for (int i = k; i < m; ++i) col[i - k] = B[i][k];
        QVector<double> v;
        double beta;
        householder(col, v, beta);
        if (beta > 0.0) {
            applyHouseholderLeft(B, k, k, v, beta);
            applyHouseholderLeft(Q, 0, k, v, beta);
        }

        // Right Householder: zero right of superdiagonal in row k
        if (k < n - 2) {
            QVector<double> row(n - k - 1);
            for (int j = k + 1; j < n; ++j) row[j - k - 1] = B[k][j];
            QVector<double> v2;
            double beta2;
            householder(row, v2, beta2);
            if (beta2 > 0.0) {
                applyHouseholderRight(B, k, k + 1, v2, beta2);
                applyHouseholderRight(P, 0, k + 1, v2, beta2);
            }
        }
    }
}

/* ---- Golub-Kahan SVD step ---- */

void SVD6::gkSVDStep(const QVector<double>& d, const QVector<double>& e,
                        int lo, int hi, double& shift,
                        double& cosL, double& sinL,
                        double& cosR, double& sinR) const
{
    // Compute Wilkinson shift from trailing 2x2 of bidiagonal
    double dn = d[hi];
    double en1 = (hi > lo) ? e[hi - 1] : 0.0;
    double dn1 = (hi > lo) ? d[hi - 1] : 0.0;
    double en2 = (hi > lo + 1) ? e[hi - 2] : 0.0;

    // Eigenvalues of trailing 2x2: [dn1^2+en2^2, dn1*en1; dn1*en1, en1^2+dn^2]
    double a = dn1 * dn1 + en2 * en2;
    double b = dn1 * en1;
    double c = en1 * en1 + dn * dn;

    double trace = a + c;
    double det = a * c - b * b;
    double disc = qSqrt(qMax(0.0, trace * trace / 4.0 - det));
    double lambda1 = trace / 2.0 + disc;
    double lambda2 = trace / 2.0 - disc;

    // Choose shift closer to c
    shift = (qAbs(lambda1 - c) < qAbs(lambda2 - c)) ? lambda1 : lambda2;

    // Initial rotation from right
    double y = d[lo] * d[lo] - shift;
    double z = d[lo] * e[lo];
    double r = qSqrt(y * y + z * z);
    cosR = (r > 1e-30) ? y / r : 1.0;
    sinR = (r > 1e-30) ? z / r : 0.0;
    cosL = 1.0;
    sinL = 0.0;
}

/* ---- Implicit zero-shift QR iteration ---- */

void SVD6::implicitQRStep(QVector<double>& diagonal,
                             QVector<double>& superDiag,
                             QVector<QVector<double>>& V,
                             int lo, int hi)
{
    int n = diagonal.size();

    // Compute shift using Golub-Kahan step
    double shift, cosL, sinL, cosR, sinR;
    gkSVDStep(diagonal, superDiag, lo, hi, shift, cosL, sinL, cosR, sinR);

    // Chase the bulge
    double f = diagonal[lo];
    double g = superDiag[lo];

    for (int k = lo; k < hi; ++k) {
        // Right rotation
        double r = qSqrt(f * f + g * g);
        double cs = (r > 1e-30) ? f / r : 1.0;
        double sn = (r > 1e-30) ? g / r : 0.0;

        if (k > lo) superDiag[k - 1] = r;

        // Update diagonal and superdiagonal
        f = cs * diagonal[k] + sn * superDiag[k];
        double tmp = -sn * diagonal[k] + cs * superDiag[k];
        superDiag[k] = tmp;
        g = sn * ((k + 1 < n) ? diagonal[k + 1] : 0.0);

        // Update V columns (right singular vectors)
        for (int i = 0; i < n; ++i) {
            double v1 = V[i][k];
            double v2 = V[i][k + 1];
            V[i][k] = cs * v1 + sn * v2;
            V[i][k + 1] = -sn * v1 + cs * v2;
        }

        // Left rotation
        r = qSqrt(f * f + g * g);
        cs = (r > 1e-30) ? f / r : 1.0;
        sn = (r > 1e-30) ? g / r : 0.0;
        diagonal[k] = r;

        if (k + 1 < n) {
            f = cs * superDiag[k] + sn * diagonal[k + 1];
            diagonal[k + 1] = -sn * superDiag[k] + cs * diagonal[k + 1];
            g = sn * ((k + 2 < n) ? superDiag[k + 1] : 0.0);
            superDiag[k] = f;
        }
    }
}

/* ---- Compute SVD ---- */

bool SVD6::compute(const QVector<QVector<double>>& A)
{
    QElapsedTimer timer;
    timer.start();

    m_rows = A.size();
    m_cols = (m_rows > 0) ? A[0].size() : 0;
    if (m_rows == 0 || m_cols == 0) return false;

    // Copy A to working matrix B
    QVector<QVector<double>> B = A;

    // Step 1: Bidiagonalization
    QVector<QVector<double>> Q, P;
    bidiagonalize(B, Q, P);

    // Extract diagonal and superdiagonal
    int minDim = qMin(m_rows, m_cols);
    QVector<double> diag(minDim), superDiag(minDim - 1, 0.0);
    for (int i = 0; i < minDim; ++i) {
        diag[i] = B[i][i];
        if (i < minDim - 1) superDiag[i] = B[i][i + 1];
    }

    // Step 2: Implicit zero-shift QR iteration
    int maxIter = 100 * minDim;
    for (int iter = 0; iter < maxIter; ++iter) {
        // Find upper bound of unreduced block
        int hi = minDim - 1;
        while (hi > 0 && qAbs(superDiag[hi - 1]) < 1e-12 * (qAbs(diag[hi - 1]) + qAbs(diag[hi])))
            hi--;

        if (hi == 0) break;

        int lo = hi - 1;
        while (lo > 0 && qAbs(superDiag[lo - 1]) >= 1e-12 * (qAbs(diag[lo - 1]) + qAbs(diag[lo])))
            lo--;

        implicitQRStep(diag, superDiag, P, lo, hi);
    }

    // Store results
    m_S = diag;
    m_U = Q;
    m_VT.resize(m_cols);
    for (int i = 0; i < m_cols; ++i) {
        m_VT[i].resize(m_cols);
        for (int j = 0; j < m_cols; ++j)
            m_VT[i][j] = P[j][i];  // Transpose P to get V^T
    }

    // Make singular values non-negative
    for (int i = 0; i < minDim; ++i) {
        if (m_S[i] < 0.0) {
            m_S[i] = -m_S[i];
            for (int j = 0; j < m_cols; ++j)
                m_VT[i][j] = -m_VT[i][j];
        }
    }

    int rank = estimateRank();
    double condNumber = (m_S[0] > 0.0 && m_S[rank - 1] > 0.0)
                        ? m_S[0] / m_S[rank - 1] : 0.0;

    m_stats.matrixRows = m_rows;
    m_stats.matrixCols = m_cols;
    m_stats.rank = rank;
    m_stats.conditionNumber = condNumber;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit computeCompleted(rank, condNumber, timer.elapsed());
    return true;
}

/* ---- Get U ---- */

QVector<QVector<double>> SVD6::matrixU() const { return m_U; }

/* ---- Get singular values ---- */

QVector<double> SVD6::singularValues() const { return m_S; }

/* ---- Get V^T ---- */

QVector<QVector<double>> SVD6::matrixVT() const { return m_VT; }

/* ---- Reconstruct ---- */

QVector<QVector<double>> SVD6::reconstruct(int rank) const
{
    int r = (rank < 0) ? qMin(m_rows, m_cols) : qMin(rank, m_S.size());
    QVector<QVector<double>> result(m_rows, QVector<double>(m_cols, 0.0));

    for (int k = 0; k < r; ++k) {
        double s = m_S[k];
        for (int i = 0; i < m_rows; ++i) {
            double u = m_U[i][k];
            for (int j = 0; j < m_cols; ++j)
                result[i][j] += s * u * m_VT[k][j];
        }
    }
    return result;
}

/* ---- Pseudo-inverse ---- */

QVector<QVector<double>> SVD6::pseudoInverse(double threshold) const
{
    int r = estimateRank(threshold);
    QVector<QVector<double>> pinv(m_cols, QVector<double>(m_rows, 0.0));

    for (int k = 0; k < r; ++k) {
        double invS = (qAbs(m_S[k]) > threshold) ? 1.0 / m_S[k] : 0.0;
        for (int i = 0; i < m_cols; ++i) {
            double v = m_VT[k][i];
            for (int j = 0; j < m_rows; ++j)
                pinv[i][j] += invS * v * m_U[j][k];
        }
    }
    return pinv;
}

/* ---- Estimate rank ---- */

int SVD6::estimateRank(double threshold) const
{
    if (m_S.isEmpty()) return 0;
    double maxS = m_S[0];
    for (double s : m_S)
        if (s > maxS) maxS = s;

    int rank = 0;
    for (double s : m_S)
        if (s > threshold * maxS) rank++;
    return rank;
}

/* ---- Reset ---- */

void SVD6::resetStatistics()
{
    m_U.clear();
    m_S.clear();
    m_VT.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
