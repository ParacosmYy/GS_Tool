/**
 * @file SchurDecomposition5.cpp
 * @brief SchurDecomposition5 实现
 *
 * 实现Schur分解：QR迭代、特征值聚类重排序、不变子空间提取。
 */

#include "utils/matrix195/SchurDecomposition5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SchurDecomposition5::SchurDecomposition5(QObject *parent) : QObject(parent) {}
SchurDecomposition5::~SchurDecomposition5() = default;

/* ---- Configuration ---- */

void SchurDecomposition5::setMaxIterations(int iter)
{
    m_maxIterations = qMax(10, iter);
}
void SchurDecomposition5::setTolerance(double tol)
{
    m_tolerance = qMax(1e-15, tol);
}

/* ---- Matrix utilities ---- */

QVector<QVector<double>> SchurDecomposition5::identity(int n) const
{
    QVector<QVector<double>> I(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) I[i][i] = 1.0;
    return I;
}

QVector<QVector<double>> SchurDecomposition5::transpose(
    const QVector<QVector<double>>& A) const
{
    int m = A.size(), n = A.isEmpty() ? 0 : A[0].size();
    QVector<QVector<double>> T(n, QVector<double>(m));
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < n; ++j)
            T[j][i] = A[i][j];
    return T;
}

QVector<QVector<double>> SchurDecomposition5::matMul(
    const QVector<QVector<double>>& A,
    const QVector<QVector<double>>& B) const
{
    int m = A.size();
    int n = B.isEmpty() ? 0 : B[0].size();
    int p = B.size();
    QVector<QVector<double>> C(m, QVector<double>(n, 0.0));
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < n; ++j)
            for (int k = 0; k < p; ++k)
                C[i][j] += A[i][k] * B[k][j];
    return C;
}

/* ---- Householder QR step (in-place on H, accumulate Q) ---- */

void SchurDecomposition5::householderQR(
    QVector<QVector<double>>& R,
    QVector<QVector<double>>& QAccum) const
{
    int n = R.size();
    for (int k = 0; k < n - 1; ++k) {
        // Build Householder vector from column k below diagonal
        double norm = 0.0;
        for (int i = k; i < n; ++i)
            norm += R[i][k] * R[i][k];
        norm = qSqrt(norm);
        if (norm < m_tolerance) continue;

        double sign = (R[k][k] >= 0) ? 1.0 : -1.0;
        double alpha = sign * norm;
        R[k][k] -= alpha;

        // Normalize
        double vnorm = 0.0;
        for (int i = k; i < n; ++i)
            vnorm += R[i][k] * R[i][k];
        if (vnorm < m_tolerance * m_tolerance) { R[k][k] += alpha; continue; }

        // Apply: R = (I - 2vv^T/vnorm) * R
        for (int j = k; j < n; ++j) {
            double dot = 0.0;
            for (int i = k; i < n; ++i)
                dot += R[i][k] * R[i][j];
            dot *= 2.0 / vnorm;
            for (int i = k; i < n; ++i)
                R[i][j] -= dot * R[i][k];
        }

        // Accumulate Q: QAccum = QAccum * (I - 2vv^T/vnorm)
        for (int j = 0; j < n; ++j) {
            double dot = 0.0;
            for (int i = k; i < n; ++i)
                dot += QAccum[j][i] * R[i][k];
            dot *= 2.0 / vnorm;
            for (int i = k; i < n; ++i)
                QAccum[j][i] -= dot * R[i][k];
        }

        R[k][k] = alpha;
        for (int i = k + 1; i < n; ++i)
            R[i][k] = 0.0;
    }
}

/* ---- Francis double shift QR step ---- */

void SchurDecomposition5::francisStep(QVector<QVector<double>>& H,
                                       QVector<QVector<double>>& QAccum,
                                       int lo, int hi)
{
    int n = H.size();
    // Implicit double shift using trace and determinant of trailing 2x2
    double s = H[hi-1][hi-1] + H[hi][hi];
    double t = H[hi-1][hi-1] * H[hi][hi] - H[hi-1][hi] * H[hi][hi-1];

    // First column of (H^2 - sH + tI)
    double x = H[lo][lo] * H[lo][lo] + H[lo][lo+1] * H[lo+1][lo] - s * H[lo][lo] + t;
    double y = H[lo+1][lo] * (H[lo][lo] + H[lo+1][lo+1] - s);
    double z = H[lo+1][lo] * H[lo+2][lo+1];

    for (int k = lo; k <= hi - 2; ++k) {
        // Build Householder to zero out y, z
        double norm = qSqrt(x*x + y*y + z*z);
        if (norm < m_tolerance) { x = H[k+1][k]; y = H[k+2][k]; continue; }

        double d = (x >= 0) ? norm : -norm;
        double v0 = x + d;
        double vnorm = v0*v0 + y*y + z*z;
        if (vnorm < m_tolerance * m_tolerance) continue;

        double scale = 2.0 / vnorm;
        int r1 = qMax(lo, k - 1);
        int r2 = qMin(hi, k + 3);

        // Apply from left: H = P * H
        for (int j = r1; j < n; ++j) {
            double dot = v0 * H[k][j] + y * H[k+1][j] + z * H[k+2][j];
            dot *= scale;
            H[k][j] -= dot * v0;
            H[k+1][j] -= dot * y;
            H[k+2][j] -= dot * z;
        }

        // Apply from right: H = H * P
        for (int i = 0; i <= r2; ++i) {
            double dot = v0 * H[i][k] + y * H[i][k+1] + z * H[i][k+2];
            dot *= scale;
            H[i][k] -= dot * v0;
            H[i][k+1] -= dot * y;
            H[i][k+2] -= dot * z;
        }

        // Accumulate Q
        for (int i = 0; i < n; ++i) {
            double dot = v0 * QAccum[i][k] + y * QAccum[i][k+1] + z * QAccum[i][k+2];
            dot *= scale;
            QAccum[i][k] -= dot * v0;
            QAccum[i][k+1] -= dot * y;
            QAccum[i][k+2] -= dot * z;
        }

        x = H[k+1][k];
        y = (k + 2 <= hi) ? H[k+2][k] : 0.0;
        z = (k + 3 <= hi) ? H[k+3][k] : 0.0;
    }

    // Last 2x2 Householder
    {
        int k = hi - 1;
        double norm = qSqrt(x*x + y*y);
        if (norm > m_tolerance && k + 1 <= hi) {
            double d = (x >= 0) ? norm : -norm;
            double v0 = x + d;
            double vnorm = v0*v0 + y*y;
            if (vnorm > m_tolerance * m_tolerance) {
                double scale = 2.0 / vnorm;
                for (int j = k; j < n; ++j) {
                    double dot = v0 * H[k][j] + y * H[k+1][j];
                    dot *= scale;
                    H[k][j] -= dot * v0;
                    H[k+1][j] -= dot * y;
                }
                for (int i = 0; i <= hi; ++i) {
                    double dot = v0 * H[i][k] + y * H[i][k+1];
                    dot *= scale;
                    H[i][k] -= dot * v0;
                    H[i][k+1] -= dot * y;
                }
                for (int i = 0; i < n; ++i) {
                    double dot = v0 * QAccum[i][k] + y * QAccum[i][k+1];
                    dot *= scale;
                    QAccum[i][k] -= dot * v0;
                    QAccum[i][k+1] -= dot * y;
                }
            }
        }
    }
}

/* ---- Main decomposition ---- */

bool SchurDecomposition5::decompose(const QVector<QVector<double>>& A)
{
    QElapsedTimer timer;
    timer.start();

    int n = A.size();
    if (n == 0) return false;
    m_size = n;
    m_T = A;
    m_Q = identity(n);

    // Reduce to upper Hessenberg form via Householder reflections
    for (int k = 0; k < n - 2; ++k) {
        double norm = 0.0;
        for (int i = k + 1; i < n; ++i)
            norm += m_T[i][k] * m_T[i][k];
        norm = qSqrt(norm);
        if (norm < m_tolerance) continue;

        double sign = (m_T[k+1][k] >= 0) ? 1.0 : -1.0;
        double alpha = sign * norm;
        m_T[k+1][k] -= alpha;

        double vnorm = 0.0;
        for (int i = k + 1; i < n; ++i)
            vnorm += m_T[i][k] * m_T[i][k];
        if (vnorm < m_tolerance * m_tolerance) { m_T[k+1][k] += alpha; continue; }

        double scale = 2.0 / vnorm;
        // H = P * H * P
        for (int j = k; j < n; ++j) {
            double dot = 0.0;
            for (int i = k + 1; i < n; ++i) dot += m_T[i][k] * m_T[i][j];
            dot *= scale;
            for (int i = k + 1; i < n; ++i) m_T[i][j] -= dot * m_T[i][k];
        }
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = k + 1; j < n; ++j) dot += m_T[i][j] * m_T[j][k];
            dot *= scale;
            for (int j = k + 1; j < n; ++j) m_T[i][j] -= dot * m_T[j][k];
        }
        for (int i = 0; i < n; ++i) {
            double dot = m_Q[i][k+1];
            for (int j = k + 2; j < n; ++j) dot += m_Q[i][j] * m_T[j][k];
            dot *= scale;
            m_Q[i][k+1] -= dot * m_T[k+1][k];
            for (int j = k + 2; j < n; ++j) m_Q[i][j] -= dot * m_T[j][k];
        }

        m_T[k+1][k] = alpha;
        for (int i = k + 2; i < n; ++i) m_T[i][k] = 0.0;
    }

    // QR iteration with Francis double shift
    int hi = n - 1;
    int iterations = 0;
    while (hi > 0 && iterations < m_maxIterations) {
        // Check for convergence (subdiagonal element near zero)
        if (qAbs(m_T[hi][hi-1]) <= m_tolerance *
            (qAbs(m_T[hi-1][hi-1]) + qAbs(m_T[hi][hi]))) {
            m_T[hi][hi-1] = 0.0;
            hi--;
        } else if (hi >= 2 && qAbs(m_T[hi-1][hi-2]) <= m_tolerance *
                   (qAbs(m_T[hi-2][hi-2]) + qAbs(m_T[hi-1][hi-1]))) {
            m_T[hi-1][hi-2] = 0.0;
            hi -= 2;
        } else {
            int lo = hi - 1;
            while (lo > 0 && qAbs(m_T[lo][lo-1]) > m_tolerance *
                   (qAbs(m_T[lo-1][lo-1]) + qAbs(m_T[lo][lo])))
                lo--;
            francisStep(m_T, m_Q, lo, hi);
        }
        iterations++;
    }

    m_stats.totalDecompositions++;
    m_stats.matrixSize = n;
    m_stats.qrIterations = iterations;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    emit decompositionCompleted(n, iterations, timer.elapsed());
    return true;
}

/* ---- Swap adjacent Schur blocks ---- */

void SchurDecomposition5::swapBlocks(int k)
{
    // Swap rows/cols k and k+1 in T, and columns k,k+1 in Q
    // Givens rotation to interchange eigenvalues at positions k and k+1
    double a = m_T[k][k], b = m_T[k][k+1];
    double c = m_T[k+1][k], d = m_T[k+1][k+1];

    double tau1 = a - d;
    double tau2 = b + c;
    if (qAbs(tau2) < m_tolerance) return;

    double theta = tau1 / tau2;
    double t = (theta >= 0) ? 1.0 / (theta + qSqrt(1.0 + theta*theta))
                            : -1.0 / (-theta + qSqrt(1.0 + theta*theta));
    double cosV = 1.0 / qSqrt(1.0 + t*t);
    double sinV = t * cosV;

    int n = m_size;
    // Apply Givens from left and right: T = G^T * T * G
    for (int j = 0; j < n; ++j) {
        double t1 = m_T[k][j], t2 = m_T[k+1][j];
        m_T[k][j] = cosV * t1 + sinV * t2;
        m_T[k+1][j] = -sinV * t1 + cosV * t2;
    }
    for (int i = 0; i < n; ++i) {
        double t1 = m_T[i][k], t2 = m_T[i][k+1];
        m_T[i][k] = cosV * t1 + sinV * t2;
        m_T[i][k+1] = -sinV * t1 + cosV * t2;
    }
    for (int i = 0; i < n; ++i) {
        double t1 = m_Q[i][k], t2 = m_Q[i][k+1];
        m_Q[i][k] = cosV * t1 + sinV * t2;
        m_Q[i][k+1] = -sinV * t1 + cosV * t2;
    }
    m_stats.reorderSwaps++;
}

/* ---- Reorder Schur form ---- */

bool SchurDecomposition5::reorder(const QVector<int>& selected)
{
    if (m_size == 0) return false;

    // Bubble selected eigenvalues to top-left via block swaps
    int writePos = 0;
    for (int s : selected) {
        if (s < 0 || s >= m_size) continue;
        // Find current position and bubble to writePos
        int curPos = s;
        while (curPos > writePos) {
            swapBlocks(curPos - 1);
            curPos--;
        }
        writePos++;
    }
    return true;
}

/* ---- Extract invariant subspace ---- */

QVector<QVector<double>> SchurDecomposition5::invariantSubspace(int k) const
{
    if (k <= 0 || k > m_size) return {};
    // First k columns of Q form the invariant subspace basis
    QVector<QVector<double>> V(m_size, QVector<double>(k));
    for (int i = 0; i < m_size; ++i)
        for (int j = 0; j < k; ++j)
            V[i][j] = m_Q[i][j];
    return V;
}

/* ---- Get eigenvalues ---- */

QVector<QPair<double, double>> SchurDecomposition5::eigenvalues() const
{
    QVector<QPair<double, double>> eigs;
    int i = 0;
    while (i < m_size) {
        if (i + 1 < m_size && qAbs(m_T[i+1][i]) > m_tolerance) {
            // Complex conjugate pair from 2x2 block
            double a = m_T[i][i], b = m_T[i][i+1];
            double c = m_T[i+1][i], d = m_T[i+1][i+1];
            double tr = a + d;
            double det = a * d - b * c;
            double disc = tr * tr - 4.0 * det;
            if (disc < 0) {
                double re = tr / 2.0;
                double im = qSqrt(-disc) / 2.0;
                eigs.append({re, im});
                eigs.append({re, -im});
            } else {
                double sq = qSqrt(disc);
                eigs.append({(tr + sq) / 2.0, 0.0});
                eigs.append({(tr - sq) / 2.0, 0.0});
            }
            i += 2;
        } else {
            eigs.append({m_T[i][i], 0.0});
            i++;
        }
    }
    return eigs;
}

/* ---- Reset ---- */

void SchurDecomposition5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_T.clear();
    m_Q.clear();
    m_size = 0;
}
