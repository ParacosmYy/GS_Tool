/**
 * @file SVD11.cpp
 * @brief SVD11 实现
 *
 * 实现奇异值分解：双对角化与隐式QR位移实现高相对精度奇异值计算。
 */

#include "utils/matrix304/SVD11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SVD11::SVD11(QObject *parent)
    : QObject(parent) {}

SVD11::~SVD11() = default;

/* ---- Configuration ---- */

void SVD11::setMaxIterations(int iter) { m_maxIter = qBound(50, iter, 10000); }
void SVD11::setConvergenceTolerance(double tol) { m_tol = qBound(1e-16, tol, 1.0); }
void SVD11::setThreshold(double threshold) { m_threshold = qBound(1e-16, threshold, 1.0); }

/* ---- Householder reflection ---- */

double SVD11::householder(QVector<double>& x, QVector<double>& v) const
{
    int n = x.size();
    double sigma = 0.0;
    for (int i = 1; i < n; ++i) sigma += x[i] * x[i];

    v = x;
    double beta = 0.0;

    if (sigma < 1e-300) {
        v.fill(0.0);
        return beta;
    }

    double mu = qSqrt(x[0] * x[0] + sigma);
    double v0 = (x[0] <= 0.0) ? x[0] - mu : -sigma / (x[0] + mu);
    beta = 2.0 * v0 * v0 / (sigma + v0 * v0);
    v[0] = v0;
    for (int i = 1; i < n; ++i) v[i] = x[i] / v0;

    return beta;
}

/* ---- Bidiagonalization ---- */

void SVD11::bidiagonalize(QVector<QVector<double>>& B,
                            QVector<QVector<double>>& U,
                            QVector<QVector<double>>& V) const
{
    int m = B.size();
    int n = B[0].size();
    int k = qMin(m, n);

    for (int i = 0; i < k; ++i) {
        // Left Householder: zero out below diagonal in column i
        if (i < m - 1) {
            QVector<double> col(m - i);
            for (int j = i; j < m; ++j) col[j - i] = B[j][i];

            QVector<double> v;
            double beta = householder(col, v);

            if (beta > 0.0) {
                // B[i:m, i:n] -= beta * v * (v^T * B[i:m, i:n])
                for (int j = i; j < n; ++j) {
                    double dot = 0.0;
                    for (int r = i; r < m; ++r) dot += v[r - i] * B[r][j];
                    for (int r = i; r < m; ++r) B[r][j] -= beta * v[r - i] * dot;
                }
                // Update U
                for (int j = 0; j < m; ++j) {
                    double dot = 0.0;
                    for (int r = i; r < m; ++r) dot += v[r - i] * U[j][r];
                    for (int r = i; r < m; ++r) U[j][r] -= beta * v[r - i] * dot;
                }
            }
        }

        // Right Householder: zero out right of superdiagonal in row i
        if (i < n - 2) {
            QVector<double> row(n - i - 1);
            for (int j = i + 1; j < n; ++j) row[j - i - 1] = B[i][j];

            QVector<double> v;
            double beta = householder(row, v);

            if (beta > 0.0) {
                for (int j = i; j < m; ++j) {
                    double dot = 0.0;
                    for (int c = i + 1; c < n; ++c) dot += v[c - i - 1] * B[j][c];
                    for (int c = i + 1; c < n; ++c) B[j][c] -= beta * v[c - i - 1] * dot;
                }
                for (int j = 0; j < n; ++j) {
                    double dot = 0.0;
                    for (int c = i + 1; c < n; ++c) dot += v[c - i - 1] * V[j][c];
                    for (int c = i + 1; c < n; ++c) V[j][c] -= beta * v[c - i - 1] * dot;
                }
            }
        }
    }
}

/* ---- Apply Givens rotation from left ---- */

void SVD11::applyGivensLeft(QVector<QVector<double>>& M, int i, int j,
                              double c, double s) const
{
    int cols = M[0].size();
    for (int k = 0; k < cols; ++k) {
        double a = M[i][k], b = M[j][k];
        M[i][k] = c * a + s * b;
        M[j][k] = -s * a + c * b;
    }
}

/* ---- Apply Givens rotation from right ---- */

void SVD11::applyGivensRight(QVector<QVector<double>>& M, int i, int j,
                               double c, double s) const
{
    int rows = M.size();
    for (int k = 0; k < rows; ++k) {
        double a = M[k][i], b = M[k][j];
        M[k][i] = c * a + s * b;
        M[k][j] = -s * a + c * b;
    }
}

/* ---- Check convergence of off-diagonal ---- */

bool SVD11::isConverged(const QVector<double>& superDiag, int lo, int hi) const
{
    for (int i = lo; i < hi; ++i) {
        if (qAbs(superDiag[i]) > m_tol * (qAbs(superDiag[lo]) + qAbs(superDiag[hi])))
            return false;
    }
    return true;
}

/* ---- Implicit QR shift step ---- */

void SVD11::implicitQRStep(QVector<double>& d, QVector<double>& e,
                              QVector<QVector<double>>& U, QVector<QVector<double>>& V,
                              int lo, int hi) const
{
    // Wilkinson shift
    double dd = (d[hi - 1] * d[hi - 1] - d[hi] * d[hi]) / (2.0 * e[hi - 1]);
    double signDd = (dd >= 0.0) ? 1.0 : -1.0;
    double mu = d[hi] * d[hi] - e[hi - 1] * e[hi - 1] /
        (dd + signDd * qSqrt(dd * dd + e[hi - 1] * e[hi - 1]));

    double x = d[lo] * d[lo] - mu;
    double z = d[lo] * e[lo];

    for (int k = lo; k < hi; ++k) {
        // Givens rotation to zero z
        double r = qSqrt(x * x + z * z);
        double c = (qAbs(r) > 1e-300) ? x / r : 1.0;
        double s = (qAbs(r) > 1e-300) ? z / r : 0.0;

        if (k > lo) e[k - 1] = r;

        // Apply to bidiagonal
        double dk = d[k], ek = e[k];
        double t1 = c * dk + s * ek;
        double t2 = -s * dk + c * ek;
        d[k] = t1;

        if (k < hi) {
            e[k] = t2;
            x = e[k] * c;
            z = d[k + 1] * s;
            d[k + 1] = d[k + 1] * c;
        }

        // Accumulate into V
        applyGivensRight(V, k, k + 1, c, s);
        // Accumulate into U
        applyGivensRight(U, k, k + 1, c, s);
    }
}

/* ---- Main SVD decomposition ---- */

SVD11::SVDResult SVD11::decompose(const QVector<QVector<double>>& A)
{
    QElapsedTimer timer;
    timer.start();

    SVDResult result;
    int m = A.size();
    if (m == 0) return result;
    int n = A[0].size();
    if (n == 0) return result;

    // Copy A into working matrix B, pad to square if needed
    int maxDim = qMax(m, n);
    QVector<QVector<double>> B(maxDim, QVector<double>(maxDim, 0.0));
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < n; ++j)
            B[i][j] = A[i][j];

    // Initialize U = I(m x m), V = I(n x n)
    QVector<QVector<double>> U(m, QVector<double>(m, 0.0));
    QVector<QVector<double>> Vt(n, QVector<double>(n, 0.0));
    for (int i = 0; i < m; ++i) U[i][i] = 1.0;
    for (int i = 0; i < n; ++i) Vt[i][i] = 1.0;

    // Step 1: Bidiagonalize
    bidiagonalize(B, U, Vt);

    // Extract bidiagonal: d[i] = B[i][i], e[i] = B[i][i+1]
    int k = qMin(m, n);
    QVector<double> d(k), e(k - 1);
    for (int i = 0; i < k; ++i) d[i] = B[i][i];
    for (int i = 0; i < k - 1; ++i) e[i] = B[i][i + 1];

    // Step 2: Implicit QR iteration on bidiagonal
    int hi = k - 1;
    int totalIter = 0;

    while (hi > 0 && totalIter < m_maxIter) {
        // Find lo: lowest active block
        int lo = hi - 1;
        while (lo > 0 && qAbs(e[lo - 1]) > m_tol * (qAbs(d[lo]) + qAbs(d[lo - 1])))
            --lo;

        // Check if e[hi-1] converged
        if (qAbs(e[hi - 1]) <= m_tol * (qAbs(d[hi]) + qAbs(d[hi - 1]))) {
            hi--;
            continue;
        }

        implicitQRStep(d, e, U, Vt, lo, hi);
        totalIter++;
    }

    // Make singular values positive and sort descending
    result.singularValues.resize(k);
    for (int i = 0; i < k; ++i) {
        result.singularValues[i] = qAbs(d[i]);
        if (d[i] < 0.0) {
            // Flip sign in V
            for (int j = 0; j < n; ++j) Vt[j][i] = -Vt[j][i];
        }
    }

    // Sort descending (simple selection sort for clarity)
    for (int i = 0; i < k; ++i) {
        int maxIdx = i;
        for (int j = i + 1; j < k; ++j)
            if (result.singularValues[j] > result.singularValues[maxIdx]) maxIdx = j;
        if (maxIdx != i) {
            std::swap(result.singularValues[i], result.singularValues[maxIdx]);
            for (int r = 0; r < m; ++r) std::swap(U[r][i], U[r][maxIdx]);
            for (int r = 0; r < n; ++r) std::swap(Vt[r][i], Vt[r][maxIdx]);
        }
    }

    result.U = U;
    result.V = Vt;
    result.converged = (hi <= 0);
    result.iterations = totalIter;

    // Compute condition number and rank
    double maxSV = (k > 0) ? result.singularValues[0] : 0.0;
    double minSV = (k > 0) ? result.singularValues[k - 1] : 0.0;
    result.conditionNumber = (minSV > m_threshold) ? maxSV / minSV : 1e300;
    result.rank = 0;
    double froSq = 0.0;
    for (int i = 0; i < k; ++i) {
        if (result.singularValues[i] > m_threshold) result.rank++;
        froSq += result.singularValues[i] * result.singularValues[i];
    }
    result.frobeniusNorm = qSqrt(froSq);

    m_stats.totalDecompositions++;
    m_stats.rows = m;
    m_stats.cols = n;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    emit decompositionDone(m, n, result.rank, elapsed);
    return result;
}

/* ---- Reconstruct from k components ---- */

QVector<QVector<double>> SVD11::reconstruct(const SVDResult& svd, int k) const
{
    int m = svd.U.size();
    int n = (svd.V.size() > 0) ? svd.V[0].size() : 0;
    int components = qMin(k, svd.rank);

    QVector<QVector<double>> result(m, QVector<double>(n, 0.0));
    for (int c = 0; c < components; ++c) {
        double sigma = svd.singularValues[c];
        for (int i = 0; i < m; ++i)
            for (int j = 0; j < n; ++j)
                result[i][j] += sigma * svd.U[i][c] * svd.V[j][c];
    }
    return result;
}

/* ---- Pseudoinverse ---- */

QVector<QVector<double>> SVD11::pseudoinverse(const SVDResult& svd) const
{
    int m = svd.U.size();
    int n = (svd.V.size() > 0) ? svd.V[0].size() : 0;
    int k = svd.singularValues.size();

    // A+ = V * Sigma+ * U^T
    QVector<QVector<double>> result(n, QVector<double>(m, 0.0));
    for (int c = 0; c < k; ++c) {
        double invSigma = (svd.singularValues[c] > m_threshold) ?
            1.0 / svd.singularValues[c] : 0.0;
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < m; ++j)
                result[i][j] += invSigma * svd.V[i][c] * svd.U[j][c];
    }
    return result;
}

/* ---- Reset ---- */

void SVD11::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
