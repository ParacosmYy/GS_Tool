/**
 * @file SchurDecomposition10.cpp
 * @brief SchurDecomposition10 实现
 *
 * 实现Schur分解：Francis双移QR迭代隐式Wilkinson移位稳定实Schur形式。
 */

#include "utils/matrix266/SchurDecomposition10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SchurDecomposition10::SchurDecomposition10(QObject *parent)
    : QObject(parent) {}
SchurDecomposition10::~SchurDecomposition10() = default;

/* ---- Configuration ---- */

void SchurDecomposition10::setMaxIterations(int maxIter) { m_maxIter = qMax(10, maxIter); }
void SchurDecomposition10::setTolerance(double tol) { m_tol = qMax(1e-16, tol); }

/* ---- Matrix helpers ---- */

QVector<QVector<double>> SchurDecomposition10::identity(int n) const
{
    QVector<QVector<double>> I(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) I[i][i] = 1.0;
    return I;
}

QVector<QVector<double>> SchurDecomposition10::matMul(
    const QVector<QVector<double>>& A, const QVector<QVector<double>>& B) const
{
    int m = A.size(), n = B[0].size(), k = B.size();
    QVector<QVector<double>> C(m, QVector<double>(n, 0.0));
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < n; ++j)
            for (int p = 0; p < k; ++p)
                C[i][j] += A[i][p] * B[p][j];
    return C;
}

QVector<QVector<double>> SchurDecomposition10::matTranspose(
    const QVector<QVector<double>>& A) const
{
    int m = A.size(), n = A[0].size();
    QVector<QVector<double>> T(n, QVector<double>(m));
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < n; ++j)
            T[j][i] = A[i][j];
    return T;
}

bool SchurDecomposition10::isNegligible(double val, double ref) const
{
    return qAbs(val) <= m_tol * qMax(qAbs(ref), 1.0);
}

/* ---- Hessenberg reduction via Householder ---- */

void SchurDecomposition10::householderLeft(QVector<QVector<double>>& A, int col,
                                             int rowStart, int rowEnd)
{
    int n = A.size();
    // Compute Householder vector from A[rowStart..n-1][col]
    double norm = 0.0;
    for (int i = rowStart; i < n; ++i) norm += A[i][col] * A[i][col];
    norm = qSqrt(norm);
    if (norm < 1e-16) return;

    double sign = (A[rowStart][col] >= 0) ? 1.0 : -1.0;
    double v0 = A[rowStart][col] + sign * norm;

    // Apply reflection: A = (I - 2*v*v'/v'v) * A
    for (int j = col; j < n; ++j) {
        double dot = v0 * A[rowStart][j];
        for (int i = rowStart + 1; i < n; ++i)
            dot += A[i][col] * A[i][j]; // Using original column as v
        dot *= 2.0 / (v0 * v0 + norm * norm);
        A[rowStart][j] -= v0 * dot;
        for (int i = rowStart + 1; i < n; ++i)
            A[i][j] -= A[i][col] * dot;
    }
}

void SchurDecomposition10::householderRight(QVector<QVector<double>>& A, int col,
                                              int rowStart, int rowEnd)
{
    int n = A.size();
    double norm = 0.0;
    for (int i = rowStart; i < n; ++i) norm += A[col][i] * A[col][i];
    norm = qSqrt(norm);
    if (norm < 1e-16) return;

    double sign = (A[col][rowStart] >= 0) ? 1.0 : -1.0;
    double v0 = A[col][rowStart] + sign * norm;

    for (int i = 0; i <= col; ++i) {
        double dot = v0 * A[i][rowStart];
        for (int j = rowStart + 1; j < n; ++j)
            dot += A[i][j] * A[col][j];
        dot *= 2.0 / (v0 * v0 + norm * norm);
        A[i][rowStart] -= v0 * dot;
        for (int j = rowStart + 1; j < n; ++j)
            A[i][j] -= A[col][j] * dot;
    }
}

void SchurDecomposition10::hessenbergReduce(QVector<QVector<double>>& H,
                                              QVector<QVector<double>>& Q)
{
    int n = H.size();
    Q = identity(n);

    for (int k = 0; k < n - 2; ++k) {
        // Compute Householder to zero out H[k+2:n, k]
        double norm = 0.0;
        for (int i = k + 1; i < n; ++i) norm += H[i][k] * H[i][k];
        norm = qSqrt(norm);
        if (norm < 1e-16) continue;

        double sign = (H[k + 1][k] >= 0) ? 1.0 : -1.0;
        double alpha = H[k + 1][k] + sign * norm;
        QVector<double> v(n - k - 1);
        v[0] = alpha;
        for (int i = 1; i < v.size(); ++i) v[i] = H[k + 1 + i][k];

        double vNorm2 = 0.0;
        for (double vi : v) vNorm2 += vi * vi;
        if (vNorm2 < 1e-30) continue;

        // Apply from left: H = P * H
        for (int j = k; j < n; ++j) {
            double dot = 0.0;
            for (int i = 0; i < v.size(); ++i) dot += v[i] * H[k + 1 + i][j];
            dot *= 2.0 / vNorm2;
            for (int i = 0; i < v.size(); ++i) H[k + 1 + i][j] -= v[i] * dot;
        }

        // Apply from right: H = H * P
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = 0; j < v.size(); ++j) dot += v[j] * H[i][k + 1 + j];
            dot *= 2.0 / vNorm2;
            for (int j = 0; j < v.size(); ++j) H[i][k + 1 + j] -= v[j] * dot;
        }

        // Accumulate Q: Q = Q * P
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = 0; j < v.size(); ++j) dot += v[j] * Q[i][k + 1 + j];
            dot *= 2.0 / vNorm2;
            for (int j = 0; j < v.size(); ++j) Q[i][k + 1 + j] -= v[j] * dot;
        }
    }
}

/* ---- Wilkinson 2x2 shift ---- */

void SchurDecomposition10::wilkinsonShift2x2(const QVector<QVector<double>>& H, int n,
                                                double& s1, double& s2) const
{
    // Bottom-right 2x2 block eigenvalues
    double a = H[n - 2][n - 2], b = H[n - 2][n - 1];
    double c = H[n - 1][n - 2], d = H[n - 1][n - 1];
    double tr = a + d;
    double det = a * d - b * c;
    double disc = tr * tr - 4.0 * det;
    if (disc >= 0) {
        double sq = qSqrt(disc);
        s1 = (tr + sq) / 2.0;
        s2 = (tr - sq) / 2.0;
    } else {
        // Complex conjugate pair: use real part
        s1 = tr / 2.0;
        s2 = s1;  // Double shift with conjugate pair
    }
}

/* ---- Francis double-shift QR step ---- */

void SchurDecomposition10::francisDoubleShift(QVector<QVector<double>>& H,
                                                QVector<QVector<double>>& Q,
                                                int ilo, int ihi)
{
    int n = H.size();
    if (ihi - ilo < 2) return;

    // Compute shifts from bottom 2x2 block
    double s1, s2;
    wilkinsonShift2x2(H, ihi + 1, s1, s2);

    // First column of (H - s1*I)(H - s2*I)
    double x = H[ilo][ilo] * H[ilo][ilo] + H[ilo][ilo + 1] * H[ilo + 1][ilo]
               - (s1 + s2) * H[ilo][ilo] + s1 * s2;
    double y = H[ilo + 1][ilo] * (H[ilo][ilo] + H[ilo + 1][ilo + 1] - s1 - s2);
    double z = H[ilo + 1][ilo] * H[ilo + 2][ilo + 1];

    // Chase the bulge
    for (int k = ilo; k <= ihi - 2; ++k) {
        // Determine Householder to zero out [x, y, z]
        double norm = qSqrt(x * x + y * y + z * z);
        if (norm < 1e-16) break;

        int r = qMax(ilo, k);
        double sign = (x >= 0) ? 1.0 : -1.0;
        QVector<double> v = {x + sign * norm, y, z};
        double vNorm2 = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
        if (vNorm2 < 1e-30) break;

        // Apply from left
        int colStart = qMax(k - 1, ilo);
        for (int j = colStart; j < n; ++j) {
            double dot = v[0] * H[k][j] + v[1] * H[k + 1][j] + v[2] * H[k + 2][j];
            dot *= 2.0 / vNorm2;
            H[k][j] -= v[0] * dot;
            H[k + 1][j] -= v[1] * dot;
            H[k + 2][j] -= v[2] * dot;
        }

        // Apply from right
        int rowEnd = qMin(k + 4, ihi + 1);
        for (int i = 0; i <= rowEnd; ++i) {
            double dot = v[0] * H[i][k] + v[1] * H[i][k + 1] + v[2] * H[i][k + 2];
            dot *= 2.0 / vNorm2;
            H[i][k] -= v[0] * dot;
            H[i][k + 1] -= v[1] * dot;
            H[i][k + 2] -= v[2] * dot;
        }

        // Accumulate Q
        for (int i = 0; i < n; ++i) {
            double dot = v[0] * Q[i][k] + v[1] * Q[i][k + 1] + v[2] * Q[i][k + 2];
            dot *= 2.0 / vNorm2;
            Q[i][k] -= v[0] * dot;
            Q[i][k + 1] -= v[1] * dot;
            Q[i][k + 2] -= v[2] * dot;
        }

        // Prepare for next iteration
        x = H[k + 1][k];
        y = H[k + 2][k];
        if (k + 3 <= ihi) z = H[k + 3][k];
        else z = 0.0;
    }

    // Final 2x2 Givens rotation
    if (ihi - ilo >= 1) {
        int k = ihi - 1;
        double norm = qSqrt(x * x + y * y);
        if (norm > 1e-16) {
            double c = x / norm, s = y / norm;

            for (int j = k; j < n; ++j) {
                double t1 = H[k][j], t2 = H[k + 1][j];
                H[k][j] = c * t1 + s * t2;
                H[k + 1][j] = -s * t1 + c * t2;
            }
            for (int i = 0; i <= ihi + 1; ++i) {
                double t1 = H[i][k], t2 = H[i][k + 1];
                H[i][k] = c * t1 + s * t2;
                H[i][k + 1] = -s * t1 + c * t2;
            }
            for (int i = 0; i < n; ++i) {
                double t1 = Q[i][k], t2 = Q[i][k + 1];
                Q[i][k] = c * t1 + s * t2;
                Q[i][k + 1] = -s * t1 + c * t2;
            }
        }
    }

    m_stats.numDoubleShifts++;
}

/* ---- Extract eigenvalues ---- */

QVector<double> SchurDecomposition10::extractEigenvalues(
    const QVector<QVector<double>>& T) const
{
    int n = T.size();
    QVector<double> eigvals;
    int i = 0;
    while (i < n) {
        if (i == n - 1 || isNegligible(T[i + 1][i], T[i][i])) {
            // Real eigenvalue
            eigvals.append(T[i][i]);
            i++;
        } else {
            // 2x2 block: complex conjugate pair
            double a = T[i][i], b = T[i][i + 1];
            double c = T[i + 1][i], d = T[i + 1][i + 1];
            double tr = a + d;
            double det = a * d - b * c;
            double disc = tr * tr - 4.0 * det;
            if (disc >= 0) {
                eigvals.append((tr + qSqrt(disc)) / 2.0);
                eigvals.append((tr - qSqrt(disc)) / 2.0);
            } else {
                // Store real part twice (conjugate pair)
                eigvals.append(tr / 2.0);
                eigvals.append(tr / 2.0);
            }
            i += 2;
        }
    }
    return eigvals;
}

/* ---- Main decomposition ---- */

SchurDecomposition10::Result SchurDecomposition10::decompose(
    const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    Result result;
    int n = matrix.size();
    if (n == 0 || matrix[0].size() != n) {
        result.converged = false;
        return result;
    }

    // Copy to Hessenberg form
    QVector<QVector<double>> H = matrix;
    QVector<QVector<double>> Q;
    hessenbergReduce(H, Q);

    // Francis QR iteration with implicit double shifts
    int ihi = n - 1;
    int iterations = 0;
    bool converged = true;

    while (ihi > 0 && iterations < m_maxIter) {
        // Deflation: check subdiagonal
        int ilo = ihi;
        while (ilo > 0 && !isNegligible(H[ilo][ilo - 1], H[ilo - 1][ilo - 1] + H[ilo][ilo]))
            ilo--;

        if (ilo == ihi) {
            // 1x1 block converged
            ihi--;
        } else if (ilo == ihi - 1) {
            // 2x2 block converged
            ihi -= 2;
        } else {
            // Apply Francis double-shift step
            francisDoubleShift(H, Q, ilo, ihi);
            iterations++;
        }
    }

    if (ihi > 0) converged = false;

    result.T = H;
    result.Q = Q;
    result.eigenvalues = extractEigenvalues(H);
    result.iterations = iterations;
    result.converged = converged;

    double elapsed = timer.elapsed();
    m_stats.matrixSize = n;
    m_stats.numIterations = iterations;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit decompositionCompleted(n, iterations, converged, elapsed);
    return result;
}

/* ---- Reset ---- */

void SchurDecomposition10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
