/**
 * @file SVD9.cpp
 * @brief SVD9 实现
 *
 * 实现奇异值分解：Golub-Kahan双对角化与隐式QR移位稳定分解。
 */

#include "utils/matrix276/SVD9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SVD9::SVD9(QObject *parent)
    : QObject(parent) {}

SVD9::~SVD9() = default;

/* ---- Configuration ---- */

void SVD9::setTolerance(double tol) { m_tol = qBound(1e-16, tol, 1.0); }
void SVD9::setMaxIterations(int iters) { m_maxIter = qBound(10, iters, 10000); }

/* ---- Householder reflection ---- */

void SVD9::householder(const QVector<double>& x, QVector<double>& v, double& beta) const
{
    int n = x.size();
    v = x;
    double sigma = 0.0;
    for (int i = 1; i < n; ++i) sigma += x[i] * x[i];

    if (sigma < m_tol * m_tol) {
        beta = 0.0;
        v.fill(0.0);
        return;
    }

    double mu = qSqrt(x[0] * x[0] + sigma);
    double x0 = (x[0] <= 0) ? x[0] - mu : -sigma / (x[0] + mu);
    beta = 2.0 * x0 * x0 / (sigma + x0 * x0);
    v[0] = 1.0;
    for (int i = 1; i < n; ++i) v[i] = x[i] / x0;
}

/* ---- Apply Householder from left ---- */

void SVD9::applyHouseholderLeft(QVector<QVector<double>>& A, const QVector<double>& v,
                                 double beta, int rStart, int cStart) const
{
    int rows = A.size();
    int cols = (rows > 0) ? A[0].size() : 0;
    int vLen = v.size();

    for (int j = cStart; j < cols; ++j) {
        double dot = 0.0;
        for (int i = 0; i < vLen; ++i)
            dot += v[i] * A[rStart + i][j];
        for (int i = 0; i < vLen; ++i)
            A[rStart + i][j] -= beta * v[i] * dot;
    }
}

/* ---- Apply Householder from right ---- */

void SVD9::applyHouseholderRight(QVector<QVector<double>>& A, const QVector<double>& v,
                                  double beta, int rStart, int cStart) const
{
    int rows = A.size();
    int vLen = v.size();

    for (int i = rStart; i < rows; ++i) {
        double dot = 0.0;
        for (int j = 0; j < vLen; ++j)
            dot += A[i][cStart + j] * v[j];
        for (int j = 0; j < vLen; ++j)
            A[i][cStart + j] -= beta * v[j] * dot;
    }
}

/* ---- Golub-Kahan bidiagonalization ---- */

void SVD9::bidiagonalize(QVector<QVector<double>>& B,
                           QVector<QVector<double>>& U,
                           QVector<QVector<double>>& VT) const
{
    int m = B.size();
    int n = (m > 0) ? B[0].size() : 0;
    int k = qMin(m, n);

    // Initialize U = I_m, VT = I_n
    U.assign(m, QVector<double>(m, 0.0));
    VT.assign(n, QVector<double>(n, 0.0));
    for (int i = 0; i < m; ++i) U[i][i] = 1.0;
    for (int i = 0; i < n; ++i) VT[i][i] = 1.0;

    for (int i = 0; i < k; ++i) {
        // Left Householder: zero out below diagonal in column i
        QVector<double> col(m - i);
        for (int r = i; r < m; ++r) col[r - i] = B[r][i];

        QVector<double> v;
        double beta;
        householder(col, v, beta);
        if (beta > 0) {
            applyHouseholderLeft(B, v, beta, i, i);
            applyHouseholderLeft(U, v, beta, i, 0);
        }

        // Right Householder: zero out right of superdiagonal in row i
        if (i < n - 2) {
            QVector<double> row(n - i - 1);
            for (int c = i + 1; c < n; ++c) row[c - i - 1] = B[i][c];

            QVector<double> vr;
            double betaR;
            householder(row, vr, betaR);
            if (betaR > 0) {
                applyHouseholderRight(B, vr, betaR, 0, i + 1);
                applyHouseholderRight(VT, vr, betaR, 0, i + 1);
            }
        }
    }
}

/* ---- Givens rotation ---- */

void SVD9::givens(double a, double b, double& c, double& s) const
{
    if (qAbs(b) < m_tol) { c = 1.0; s = 0.0; return; }
    if (qAbs(a) < m_tol) { c = 0.0; s = (b > 0) ? 1.0 : -1.0; return; }
    double r = qSqrt(a * a + b * b);
    c = a / r;
    s = -b / r;
}

/* ---- Implicit QR shift on bidiagonal ---- */

void SVD9::implicitQRShift(QVector<double>& diag, QVector<double>& superdiag,
                             QVector<QVector<double>>& U, QVector<QVector<double>>& VT,
                             int lo, int hi)
{
    int n = diag.size();
    int maxIters = m_maxIter;

    for (int iter = 0; iter < maxIters; ++iter) {
        // Check convergence of superdiagonal elements
        for (int i = lo; i < hi; ++i) {
            if (qAbs(superdiag[i]) <= m_tol * (qAbs(diag[i]) + qAbs(diag[i + 1])))
                superdiag[i] = 0.0;
        }

        // Find largest unreduced block
        int q = hi;
        while (q > lo && qAbs(superdiag[q - 1]) <= m_tol) q--;
        if (q == lo) break;

        // Wilkinson shift
        double d = (diag[hi - 1] * diag[hi - 1] + superdiag[hi - 1] * superdiag[hi - 1]
                    - diag[hi] * diag[hi]) * 0.5;
        double f = (diag[hi] * superdiag[hi - 1]);
        double mu = qSqrt(d * d + f * f);
        double shift = (diag[lo] * diag[lo] - diag[hi] * diag[hi] +
                        superdiag[hi - 1] * superdiag[hi - 1]) /
                       (diag[lo] * superdiag[lo] + (mu != 0 ? (f > 0 ? mu : -mu) : 0));

        // Chase bulge with Givens rotations
        double x = diag[lo] * diag[lo] - shift;
        double z = diag[lo] * superdiag[lo];

        for (int k = lo; k < q; ++k) {
            double c, s;
            givens(x, z, c, s);

            // Right rotation on B
            double tmp1 = c * diag[k] - s * superdiag[k];
            double tmp2 = s * diag[k] + c * superdiag[k];
            if (k > lo) superdiag[k - 1] = tmp1;
            diag[k] = tmp2;

            if (k < q - 1) {
                double newZ = -s * superdiag[k + 1];
                superdiag[k + 1] = c * superdiag[k + 1];
                x = superdiag[k];
                z = newZ;
            }

            // Apply to VT
            for (int i = 0; i < n; ++i) {
                double v1 = VT[k][i], v2 = VT[k + 1][i];
                VT[k][i] = c * v1 - s * v2;
                VT[k + 1][i] = s * v1 + c * v2;
            }

            // Left rotation
            givens(diag[k], z, c, s);
            diag[k] = c * diag[k] - s * z;
            tmp1 = c * superdiag[k + (k < q - 1 ? 1 : 0)];
            tmp2 = s * diag[k + 1];
            if (k < q - 1) {
                superdiag[k] = tmp1;
                diag[k + 1] = -s * superdiag[k + 1] + c * diag[k + 1];
                superdiag[k + 1] = c * superdiag[k + 1];
                x = superdiag[k];
                z = s * diag[k + 1];
            }

            // Apply to U
            for (int i = 0; i < m_rows; ++i) {
                double u1 = U[i][k], u2 = U[i][k + 1];
                U[i][k] = c * u1 - s * u2;
                U[i][k + 1] = s * u1 + c * u2;
            }
        }
    }
}

/* ---- Main compute ---- */

void SVD9::compute(const QVector<QVector<double>>& A)
{
    QElapsedTimer timer;
    timer.start();

    m_rows = A.size();
    m_cols = (m_rows > 0) ? A[0].size() : 0;
    int n = qMin(m_rows, m_cols);

    if (m_rows == 0 || m_cols == 0) return;

    // Copy A to B for in-place bidiagonalization
    QVector<QVector<double>> B = A;
    QVector<QVector<double>> U, VT;
    bidiagonalize(B, U, VT);

    // Extract bidiagonal
    QVector<double> diag(n);
    QVector<double> superdiag(n, 0.0);
    for (int i = 0; i < n; ++i) {
        diag[i] = B[i][i];
        if (i < n - 1) superdiag[i] = B[i][i + 1];
    }

    // Apply implicit QR shift to find singular values
    implicitQRShift(diag, superdiag, U, VT, 0, n - 1);

    // Sort singular values in descending order
    for (int i = 0; i < n; ++i) {
        diag[i] = qAbs(diag[i]);
        if (diag[i] < m_tol) diag[i] = 0.0;
    }

    // Sort by descending singular value
    for (int i = 0; i < n - 1; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (diag[j] > diag[i]) {
                std::swap(diag[i], diag[j]);
                for (int r = 0; r < m_rows; ++r) std::swap(U[r][i], U[r][j]);
                for (int c = 0; c < m_cols; ++c) std::swap(VT[i][c], VT[j][c]);
            }
        }
    }

    m_U = U;
    m_S = diag;
    m_VT = VT;

    // Compute effective rank and condition number
    int effRank = 0;
    double maxS = (n > 0) ? diag[0] : 0.0;
    for (int i = 0; i < n; ++i) {
        if (diag[i] > m_tol * maxS) effRank++;
    }

    double minNonZero = maxS;
    for (int i = 0; i < n; ++i) {
        if (diag[i] > m_tol * maxS) { minNonZero = diag[i]; break; }
    }
    double condNum = (minNonZero > m_tol) ? maxS / minNonZero : 1e18;

    double elapsed = timer.elapsed();
    m_stats.rows = m_rows;
    m_stats.cols = m_cols;
    m_stats.rank = effRank;
    m_stats.conditionNumber = condNum;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit decompositionDone(m_rows, m_cols, effRank, condNum, elapsed);
}

/* ---- Accessors ---- */

QVector<QVector<double>> SVD9::matrixU() const { return m_U; }
QVector<double> SVD9::singularValues() const { return m_S; }
QVector<QVector<double>> SVD9::matrixVT() const { return m_VT; }
int SVD9::rank() const { return m_stats.rank; }

/* ---- Pseudoinverse: A^+ = V * diag(1/sigma_i) * U^T ---- */

QVector<QVector<double>> SVD9::pseudoinverse() const
{
    int n = qMin(m_rows, m_cols);
    if (n == 0) return {};

    double maxS = (m_S.size() > 0) ? m_S[0] : 1.0;
    double thresh = m_tol * maxS;

    // V * S^+
    QVector<QVector<double>> VSPlus(m_cols, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        double sinv = (m_S[i] > thresh) ? 1.0 / m_S[i] : 0.0;
        for (int j = 0; j < m_cols; ++j)
            VSPlus[j][i] = VT[i][j] * sinv;
    }

    // (V * S^+) * U^T
    QVector<QVector<double>> result(m_cols, QVector<double>(m_rows, 0.0));
    for (int i = 0; i < m_cols; ++i)
        for (int j = 0; j < m_rows; ++j)
            for (int k = 0; k < n; ++k)
                result[i][j] += VSPlus[i][k] * U[j][k];

    return result;
}

/* ---- Reset ---- */

void SVD9::resetStatistics()
{
    m_U.clear();
    m_S.clear();
    m_VT.clear();
    m_rows = 0;
    m_cols = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
