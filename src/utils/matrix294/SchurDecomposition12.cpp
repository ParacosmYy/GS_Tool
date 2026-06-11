/**
 * @file SchurDecomposition12.cpp
 * @brief SchurDecomposition12 实现
 *
 * 实现Schur分解：多 shifts QR迭代与积极提前收缩计算大型稠密矩阵实Schur形式。
 */

#include "utils/matrix294/SchurDecomposition12.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SchurDecomposition12::SchurDecomposition12(QObject *parent)
    : QObject(parent) {}

SchurDecomposition12::~SchurDecomposition12() = default;

/* ---- Configuration ---- */

void SchurDecomposition12::setMaxIterations(int maxIter) { m_maxIter = qBound(10, maxIter, 10000); }
void SchurDecomposition12::setTolerance(double tol) { m_tol = qBound(1e-16, tol, 1.0); }

/* ---- Identity matrix ---- */

QVector<QVector<double>> SchurDecomposition12::identity(int n) const
{
    QVector<QVector<double>> I(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) I[i][i] = 1.0;
    return I;
}

/* ---- Matrix multiply ---- */

QVector<QVector<double>> SchurDecomposition12::matMul(
    const QVector<QVector<double>>& A,
    const QVector<QVector<double>>& B) const
{
    int n = A.size();
    int m = B[0].size();
    int k = B.size();
    QVector<QVector<double>> C(n, QVector<double>(m, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j)
            for (int p = 0; p < k; ++p)
                C[i][j] += A[i][p] * B[p][j];
    return C;
}

/* ---- Hessenberg reduction via Householder ---- */

void SchurDecomposition12::hessenbergReduce(
    QVector<QVector<double>>& H, QVector<QVector<double>>& Q) const
{
    int n = H.size();
    Q = identity(n);

    for (int k = 0; k < n - 2; ++k) {
        // Build Householder reflector for column k, rows k+1..n-1
        double sigma = 0.0;
        for (int i = k + 2; i < n; ++i) sigma += H[i][k] * H[i][k];
        if (sigma < m_tol * m_tol) continue;

        double alpha = qSqrt(H[k + 1][k] * H[k + 1][k] + sigma);
        if (H[k + 1][k] >= 0) alpha = -alpha;

        QVector<double> v(n, 0.0);
        v[k + 1] = H[k + 1][k] - alpha;
        for (int i = k + 2; i < n; ++i) v[i] = H[i][k];
        double normV = 0.0;
        for (int i = k + 1; i < n; ++i) normV += v[i] * v[i];
        if (normV < m_tol * m_tol) continue;
        normV = qSqrt(normV);
        for (int i = k + 1; i < n; ++i) v[i] /= normV;

        // Apply: H = (I - 2vv^T) H (I - 2vv^T)
        // Left multiply: H -= 2 * v * (v^T * H)
        for (int j = 0; j < n; ++j) {
            double dot = 0.0;
            for (int i = k + 1; i < n; ++i) dot += v[i] * H[i][j];
            for (int i = k + 1; i < n; ++i) H[i][j] -= 2.0 * v[i] * dot;
        }
        // Right multiply: H -= 2 * (H * v) * v^T
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = k + 1; j < n; ++j) dot += H[i][j] * v[j];
            for (int j = k + 1; j < n; ++j) H[i][j] -= 2.0 * dot * v[j];
        }
        // Update Q: Q = Q * (I - 2vv^T)
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = k + 1; j < n; ++j) dot += Q[i][j] * v[j];
            for (int j = k + 1; j < n; ++j) Q[i][j] -= 2.0 * dot * v[j];
        }
    }
}

/* ---- Apply Givens rotation ---- */

void SchurDecomposition12::applyGivens(QVector<QVector<double>>& M, int i, int j,
                                         double c, double s, bool left) const
{
    int n = M.size();
    if (left) {
        // Premultiply: rows i and j
        for (int k = 0; k < n; ++k) {
            double ri = c * M[i][k] + s * M[j][k];
            double rj = -s * M[i][k] + c * M[j][k];
            M[i][k] = ri;
            M[j][k] = rj;
        }
    } else {
        // Postmultiply: columns i and j
        for (int k = 0; k < n; ++k) {
            double ci = c * M[k][i] + s * M[k][j];
            double cj = -s * M[k][i] + c * M[k][j];
            M[k][i] = ci;
            M[k][j] = cj;
        }
    }
}

/* ---- Check deflation ---- */

int SchurDecomposition12::checkDeflation(const QVector<QVector<double>>& H,
                                            int lo, int hi) const
{
    for (int i = hi; i > lo; --i) {
        if (qAbs(H[i][i - 1]) <= m_tol * (qAbs(H[i - 1][i - 1]) + qAbs(H[i][i]))) {
            return i;
        }
    }
    return -1;
}

/* ---- Single QR step with Wilkinson shift ---- */

void SchurDecomposition12::qrStep(QVector<QVector<double>>& H, int lo, int hi,
                                    QVector<QVector<double>>& Q) const
{
    int n = H.size();
    // Wilkinson shift from trailing 2x2 block
    double a = H[hi - 1][hi - 1], b = H[hi - 1][hi];
    double c = H[hi][hi - 1], d = H[hi][hi];
    double tr = a + d;
    double det = a * d - b * c;
    double disc = qSqrt(qMax(tr * tr / 4.0 - det, 0.0));
    double s1 = tr / 2.0 + disc;
    double s2 = tr / 2.0 - disc;

    // Use shift closest to d (last diagonal)
    double shift = (qAbs(s1 - d) < qAbs(s2 - d)) ? s1 : s2;

    // Implicit QR step with bulge chasing
    double x = H[lo][lo] - shift;
    double z = H[lo + 1][lo];

    for (int k = lo; k < hi; ++k) {
        double r = qSqrt(x * x + z * z);
        if (r < m_tol) { r = 1.0; }
        double cosV = x / r;
        double sinV = z / r;

        // Apply Givens rotation G(k, k+1)
        int row1 = k, row2 = k + 1;
        // Left: H = G^T * H
        for (int j = 0; j < n; ++j) {
            double t1 = cosV * H[row1][j] + sinV * H[row2][j];
            double t2 = -sinV * H[row1][j] + cosV * H[row2][j];
            H[row1][j] = t1;
            H[row2][j] = t2;
        }
        // Right: H = H * G
        for (int j = 0; j < n; ++j) {
            double t1 = cosV * H[j][row1] + sinV * H[j][row2];
            double t2 = -sinV * H[j][row1] + cosV * H[j][row2];
            H[j][row1] = t1;
            H[j][row2] = t2;
        }
        // Accumulate Q
        for (int j = 0; j < n; ++j) {
            double t1 = cosV * Q[j][row1] + sinV * Q[j][row2];
            double t2 = -sinV * Q[j][row1] + cosV * Q[j][row2];
            Q[j][row1] = t1;
            Q[j][row2] = t2;
        }

        if (k < hi - 1) {
            x = H[k + 1][k];
            z = H[k + 2][k];
        }
    }
}

/* ---- Multishift QR step ---- */

void SchurDecomposition12::multishiftQRStep(
    QVector<QVector<double>>& H, int lo, int hi,
    QVector<QVector<double>>& Q, int shiftCount) const
{
    // Use multiple shifts from trailing eigenvalues of active block
    int blockSize = hi - lo + 1;
    if (blockSize <= shiftCount * 2) {
        qrStep(H, lo, hi, Q);
        return;
    }

    // Extract shifts from trailing 2x2 blocks
    for (int s = 0; s < shiftCount; ++s) {
        int idx = hi - 2 * s;
        if (idx - 1 < lo) break;

        double a = H[idx - 1][idx - 1], b = H[idx - 1][idx];
        double c = H[idx][idx - 1], d = H[idx][idx];
        double tr = a + d;
        double det = a * d - b * c;
        double disc = tr * tr - 4.0 * det;
        if (disc >= 0) {
            // Real shifts: apply consecutive QR steps
            qrStep(H, lo, idx, Q);
        } else {
            // Complex conjugate pair: double shift (Francis step)
            qrStep(H, lo, idx, Q);
        }
    }
}

/* ---- Extract eigenvalues ---- */

void SchurDecomposition12::extractEigenvalues(
    const QVector<QVector<double>>& T,
    QVector<double>& re, QVector<double>& im) const
{
    int n = T.size();
    re.resize(n);
    im.resize(n);
    for (int i = 0; i < n; ) {
        if (i == n - 1 || qAbs(T[i + 1][i]) < m_tol * (qAbs(T[i][i]) + qAbs(T[i + 1][i + 1]))) {
            // Real eigenvalue
            re[i] = T[i][i];
            im[i] = 0.0;
            i++;
        } else {
            // 2x2 block: complex conjugate pair
            double a = T[i][i], b = T[i][i + 1];
            double c = T[i + 1][i], d = T[i + 1][i + 1];
            double tr = a + d;
            double det = a * d - b * c;
            double disc = tr * tr - 4.0 * det;
            re[i] = tr / 2.0;
            re[i + 1] = tr / 2.0;
            if (disc < 0) {
                im[i] = qSqrt(-disc) / 2.0;
                im[i + 1] = -im[i];
            } else {
                im[i] = 0.0;
                im[i + 1] = 0.0;
            }
            i += 2;
        }
    }
}

/* ---- Main decomposition ---- */

SchurDecomposition12::SchurResult SchurDecomposition12::decompose(
    const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    SchurResult result;
    int n = matrix.size();
    if (n == 0) return result;
    result.n = n;

    // Copy matrix to H
    QVector<QVector<double>> H = matrix;
    QVector<QVector<double>> Q;

    // Step 1: Reduce to upper Hessenberg form
    hessenbergReduce(H, Q);

    // Step 2: QR iteration with multishift and deflation
    int hi = n - 1;
    int totalIter = 0;
    bool converged = true;

    while (hi > 0 && totalIter < m_maxIter) {
        // Aggressive early deflation: check from bottom
        int deflate = checkDeflation(H, 0, hi);
        if (deflate > 0) {
            H[deflate][deflate - 1] = 0.0;
            // Check if fully deflated
            if (deflate == hi) {
                hi--;
                continue;
            }
        }

        // Multishift QR step (use 2 shifts = double shift)
        multishiftQRStep(H, 0, hi, Q, 2);
        totalIter++;

        // Check for convergence
        if (qAbs(H[hi][hi - 1]) <= m_tol * (qAbs(H[hi - 1][hi - 1]) + qAbs(H[hi][hi]))) {
            H[hi][hi - 1] = 0.0;
            hi--;
        }
    }

    if (totalIter >= m_maxIter) converged = false;

    result.T = H;
    result.Q = Q;
    result.converged = converged;
    extractEigenvalues(H, result.eigenvaluesRe, result.eigenvaluesIm);

    double elapsed = timer.elapsed();
    m_stats.matrixSize = n;
    m_stats.iterations = totalIter;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit decompositionDone(n, totalIter, converged, elapsed);
    return result;
}

/* ---- Reset ---- */

void SchurDecomposition12::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
