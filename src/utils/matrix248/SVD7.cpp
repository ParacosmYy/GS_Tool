/**
 * @file SVD7.cpp
 * @brief SVD7 实现
 *
 * 实现奇异值分解：Golub-Kahan双对角化与隐式零移位QR迭代。
 */

#include "utils/matrix248/SVD7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SVD7::SVD7(QObject *parent) : QObject(parent) {}
SVD7::~SVD7() = default;

/* ---- Configuration ---- */

void SVD7::setMaxIterations(int iters) { m_maxIter = qMax(10, iters); }
void SVD7::setTolerance(double tol) { m_tol = qMax(1e-15, tol); }

/* ---- Identity matrix ---- */

QVector<QVector<double>> SVD7::identity(int n)
{
    QVector<QVector<double>> I(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) I[i][i] = 1.0;
    return I;
}

/* ---- Givens rotation ---- */

void SVD7::givensRotation(double a, double b, double& c, double& s)
{
    if (qFabs(b) < 1e-30) { c = 1.0; s = 0.0; return; }
    if (qFabs(b) > qFabs(a)) {
        double t = -a / b;
        s = 1.0 / qSqrt(1.0 + t * t);
        c = s * t;
    } else {
        double t = -b / a;
        c = 1.0 / qSqrt(1.0 + t * t);
        s = c * t;
    }
}

/* ---- Householder column reflection ---- */

void SVD7::householderCol(QVector<QVector<double>>& M, int col, int startRow,
                            QVector<double>& v, double& beta)
{
    int m = M.size();
    int n = m > 0 ? M[0].size() : 0;
    double sigma = 0.0;
    double x0 = M[startRow][col];

    v.resize(m);
    v.fill(0.0);
    v[startRow] = 1.0;

    for (int i = startRow + 1; i < m; ++i) {
        v[i] = M[i][col];
        sigma += M[i][col] * M[i][col];
    }

    if (sigma < 1e-30) { beta = 0.0; return; }

    double mu = qSqrt(x0 * x0 + sigma);
    double v0 = (x0 <= 0.0) ? x0 - mu : -sigma / (x0 + mu);
    beta = 2.0 * v0 * v0 / (sigma + v0 * v0);
    for (int i = startRow; i < m; ++i)
        v[i] /= v0;
}

/* ---- Apply Householder from left: M = (I - β·vv^T)·M ---- */

void SVD7::applyHouseholderLeft(QVector<QVector<double>>& M, const QVector<double>& v,
                                  double beta, int colStart)
{
    int m = M.size();
    int n = m > 0 ? M[0].size() : 0;
    for (int j = colStart; j < n; ++j) {
        double dot = 0.0;
        for (int i = 0; i < m; ++i) dot += v[i] * M[i][j];
        for (int i = 0; i < m; ++i) M[i][j] -= beta * v[i] * dot;
    }
}

/* ---- Apply Householder from right: M = M·(I - β·vv^T) ---- */

void SVD7::applyHouseholderRight(QVector<QVector<double>>& M, const QVector<double>& v,
                                   double beta, int rowStart)
{
    int m = M.size();
    int n = m > 0 ? M[0].size() : 0;
    for (int i = rowStart; i < m; ++i) {
        double dot = 0.0;
        for (int j = 0; j < n; ++j) dot += M[i][j] * v[j];
        for (int j = 0; j < n; ++j) M[i][j] -= beta * dot * v[j];
    }
}

/* ---- Golub-Kahan bidiagonalization ---- */

void SVD7::bidiagonalize(QVector<QVector<double>>& B,
                            QVector<QVector<double>>& U,
                            QVector<QVector<double>>& V)
{
    int m = B.size();
    int n = m > 0 ? B[0].size() : 0;
    U = identity(m);
    V = identity(n);

    int minDim = qMin(m, n);
    for (int k = 0; k < minDim; ++k) {
        // Left Householder: zero below B[k][k]
        QVector<double> v;
        double beta;
        householderCol(B, k, k, v, beta);
        if (beta > 0.0) {
            applyHouseholderLeft(B, v, beta, k);
            applyHouseholderLeft(U, v, beta, 0);
        }

        // Right Householder: zero right of B[k][k+1]
        if (k < n - 2) {
            // Build vector from row k, columns k+1..n-1
            QVector<double> rv(n, 0.0);
            double sigma = 0.0;
            double x0 = B[k][k + 1];
            rv[k + 1] = 1.0;
            for (int j = k + 2; j < n; ++j) {
                rv[j] = B[k][j];
                sigma += B[k][j] * B[k][j];
            }
            if (sigma > 1e-30) {
                double mu = qSqrt(x0 * x0 + sigma);
                double v0 = (x0 <= 0.0) ? x0 - mu : -sigma / (x0 + mu);
                double beta2 = 2.0 * v0 * v0 / (sigma + v0 * v0);
                for (int j = k + 1; j < n; ++j) rv[j] /= v0;
                applyHouseholderRight(B, rv, beta2, k);
                applyHouseholderRight(V, rv, beta2, 0);
            }
        }
    }
}

/* ---- Implicit zero-shift QR sweep ---- */

void SVD7::implicitQRQSweep(QVector<QVector<double>>& B,
                               QVector<QVector<double>>& U,
                               QVector<QVector<double>>& V)
{
    int n = qMin(B.size(), B[0].size());
    // Find lowest unreduced subdiagonal
    int q = n - 1;
    while (q > 0 && qFabs(B[q][q - 1]) <= m_tol * (qFabs(B[q][q]) + qFabs(B[q - 1][q - 1])))
        q--;
    if (q == 0) return;

    // Chase bulge with Givens rotations
    double x = B[0][0], z = B[1][0];
    for (int k = 0; k < q; ++k) {
        double c, s;
        givensRotation(x, z, c, s);

        // Apply Givens from right to B
        int rows = B.size();
        for (int i = 0; i < rows; ++i) {
            double t1 = B[i][k], t2 = (k + 1 < B[0].size()) ? B[i][k + 1] : 0.0;
            B[i][k] = c * t1 - s * t2;
            if (k + 1 < B[0].size()) B[i][k + 1] = s * t1 + c * t2;
        }
        // Apply Givens from right to V
        int vn = V.size();
        for (int i = 0; i < vn; ++i) {
            double t1 = V[i][k], t2 = V[i][k + 1];
            V[i][k] = c * t1 - s * t2;
            V[i][k + 1] = s * t1 + c * t2;
        }

        if (k < q) {
            x = B[k][k];
            z = (k + 1 < B.size()) ? B[k + 1][k] : 0.0;
        }

        if (k < q - 1) {
            givensRotation(x, z, c, s);
            // Apply from left
            int cols = B[0].size();
            for (int j = 0; j < cols; ++j) {
                double t1 = B[k][j], t2 = B[k + 1][j];
                B[k][j] = c * t1 - s * t2;
                B[k + 1][j] = s * t1 + c * t2;
            }
            // Apply from left to U
            int ucols = U[0].size();
            for (int j = 0; j < ucols; ++j) {
                double t1 = U[k][j], t2 = U[k + 1][j];
                U[k][j] = c * t1 - s * t2;
                U[k + 1][j] = s * t1 + c * t2;
            }
            if (k + 2 < B.size()) {
                x = B[k][k + 1];
                z = B[k][k + 2];
            }
        }
    }
}

/* ---- Compute SVD ---- */

void SVD7::compute(const QVector<QVector<double>>& A)
{
    QElapsedTimer timer;
    timer.start();

    m_m = A.size();
    m_n = m_m > 0 ? A[0].size() : 0;
    if (m_m == 0 || m_n == 0) return;

    // Work on a copy
    QVector<QVector<double>> B = A;
    QVector<QVector<double>> U, V;
    bidiagonalize(B, U, V);

    // QR iteration on bidiagonal
    int minDim = qMin(m_m, m_n);
    int iter = 0;
    while (iter < m_maxIter) {
        // Check convergence: all subdiagonal elements small
        bool converged = true;
        for (int i = 1; i < minDim; ++i) {
            if (qFabs(B[i][i - 1]) > m_tol * (qFabs(B[i][i]) + qFabs(B[i - 1][i - 1]))) {
                converged = false;
                break;
            }
        }
        if (converged) break;
        implicitQRSweep(B, U, V);
        iter++;
    }

    // Extract singular values (diagonal of B)
    m_S.resize(minDim);
    for (int i = 0; i < minDim; ++i) m_S[i] = qFabs(B[i][i]);

    // Sort singular values descending
    QVector<int> order(minDim);
    for (int i = 0; i < minDim; ++i) order[i] = i;
    std::sort(order.begin(), order.end(), [this](int a, int b) { return m_S[a] > m_S[b]; });

    QVector<double> sortedS(minDim);
    for (int i = 0; i < minDim; ++i) sortedS[i] = m_S[order[i]];
    m_S = sortedS;

    m_U = U;
    m_V = V;

    // Compute rank and condition number
    int rank = 0;
    double maxS = m_S.isEmpty() ? 0.0 : m_S[0];
    for (int i = 0; i < m_S.size(); ++i)
        if (m_S[i] > m_tol * maxS * qMax(m_m, m_n)) rank++;

    double minNonZero = rank > 0 ? m_S[rank - 1] : 0.0;
    double cond = (minNonZero > 0.0 && maxS > 0.0) ? maxS / minNonZero
                   : std::numeric_limits<double>::infinity();

    m_stats.numRows = m_m;
    m_stats.numCols = m_n;
    m_stats.rank = rank;
    m_stats.conditionNumber = cond;
    m_stats.iterationsUsed = iter;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit decompositionCompleted(rank, cond, timer.elapsed());
}

/* ---- Accessors ---- */

QVector<QVector<double>> SVD7::matrixU() const { return m_U; }
QVector<double> SVD7::singularValues() const { return m_S; }
QVector<QVector<double>> SVD7::matrixV() const { return m_V; }

QVector<QVector<double>> SVD7::reconstruct() const
{
    int m = m_U.size();
    int n = m_V.size();
    int r = m_S.size();
    QVector<QVector<double>> result(m, QVector<double>(n, 0.0));
    for (int k = 0; k < r; ++k)
        for (int i = 0; i < m; ++i)
            for (int j = 0; j < n; ++j)
                result[i][j] += m_U[i][k] * m_S[k] * m_V[j][k];
    return result;
}

QVector<QVector<double>> SVD7::pseudoInverse() const
{
    int n = m_V.size();
    int m = m_U.size();
    int r = m_S.size();
    QVector<QVector<double>> result(n, QVector<double>(m, 0.0));
    for (int k = 0; k < r; ++k) {
        if (m_S[k] < m_tol) continue;
        double invS = 1.0 / m_S[k];
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < m; ++j)
                result[i][j] += m_V[i][k] * invS * m_U[j][k];
    }
    return result;
}

/* ---- Reset ---- */

void SVD7::resetStatistics()
{
    m_U.clear(); m_S.clear(); m_V.clear();
    m_m = 0; m_n = 0;
    m_stats = Stats{}; m_timeSum = 0.0;
}
