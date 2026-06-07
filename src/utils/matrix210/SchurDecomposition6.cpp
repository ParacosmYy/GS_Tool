/**
 * @file SchurDecomposition6.cpp
 * @brief SchurDecomposition6 实现
 *
 * 实现Schur分解：Hessenberg归约、Francis双移位QR、激进早期收缩。
 */

#include "utils/matrix210/SchurDecomposition6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SchurDecomposition6::SchurDecomposition6(QObject *parent) : QObject(parent) {}
SchurDecomposition6::~SchurDecomposition6() = default;

/* ---- Configuration ---- */

void SchurDecomposition6::setMaxIterations(int maxIter) { m_maxIter = qMax(1, maxIter); }
void SchurDecomposition6::setTolerance(double tol) { m_tol = qMax(1e-15, tol); }

/* ---- Givens rotation ---- */

void SchurDecomposition6::givensRotation(double a, double b, double& c, double& s)
{
    if (qFabs(b) < 1e-30) { c = 1.0; s = 0.0; return; }
    if (qFabs(b) > qFabs(a)) {
        double tau = -a / b;
        s = 1.0 / qSqrt(1.0 + tau * tau);
        c = s * tau;
    } else {
        double tau = -b / a;
        c = 1.0 / qSqrt(1.0 + tau * tau);
        s = c * tau;
    }
}

/* ---- Hessenberg reduction ---- */

void SchurDecomposition6::hessenbergReduce(QVector<QVector<double>>& A,
                                             QVector<QVector<double>>& Q)
{
    int n = A.size();
    Q.assign(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) Q[i][i] = 1.0;

    for (int k = 0; k < n - 2; ++k) {
        // Compute Householder reflector for column k below diagonal
        double norm = 0.0;
        for (int i = k + 1; i < n; ++i) norm += A[i][k] * A[i][k];
        norm = qSqrt(norm);
        if (norm < 1e-30) continue;

        double alpha = (A[k + 1][k] >= 0) ? -norm : norm;
        double beta = qSqrt(2.0 * norm * (norm + qFabs(A[k + 1][k])));
        if (beta < 1e-30) continue;

        QVector<double> v(n, 0.0);
        v[k + 1] = (A[k + 1][k] - alpha) / beta;
        for (int i = k + 2; i < n; ++i) v[i] = A[i][k] / beta;

        // Apply: A = (I - 2vv^T) A (I - 2vv^T)
        for (int j = k; j < n; ++j) {
            double dot = 0.0;
            for (int i = k + 1; i < n; ++i) dot += v[i] * A[i][j];
            for (int i = k + 1; i < n; ++i) A[i][j] -= 2.0 * v[i] * dot;
        }
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = k + 1; j < n; ++j) dot += A[i][j] * v[j];
            for (int j = k + 1; j < n; ++j) A[i][j] -= 2.0 * dot * v[j];
        }
        // Update Q
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = k + 1; j < n; ++j) dot += Q[i][j] * v[j];
            for (int j = k + 1; j < n; ++j) Q[i][j] -= 2.0 * dot * v[j];
        }
    }
}

/* ---- 2x2 Schur form ---- */

void SchurDecomposition6::schur2x2(QVector<QVector<double>>& H, int p, int q,
                                      QVector<QVector<double>>& Q)
{
    int n = H.size();
    double a = H[p][p], b = H[p][q];
    double c = H[q][p], d = H[q][q];
    double tr = a + d;
    double det = a * d - b * c;
    double disc = tr * tr - 4.0 * det;
    if (disc >= 0) {
        // Real eigenvalues: use Givens to zero out H[q][p]
        double c_rot, s_rot;
        givensRotation(H[p][p] - d, H[q][p], c_rot, s_rot);
        // Apply Givens rotation
        for (int i = 0; i < n; ++i) {
            double h1 = H[i][p], h2 = H[i][q];
            H[i][p] = c_rot * h1 - s_rot * h2;
            H[i][q] = s_rot * h1 + c_rot * h2;
        }
        for (int i = 0; i < n; ++i) {
            double h1 = H[p][i], h2 = H[q][i];
            H[p][i] = c_rot * h1 - s_rot * h2;
            H[q][i] = s_rot * h1 + c_rot * h2;
        }
        for (int i = 0; i < n; ++i) {
            double q1 = Q[i][p], q2 = Q[i][q];
            Q[i][p] = c_rot * q1 - s_rot * q2;
            Q[i][q] = s_rot * q1 + c_rot * q2;
        }
    }
    // If disc < 0, keep 2x2 block (complex conjugate pair)
}

/* ---- Francis double-shift QR step ---- */

void SchurDecomposition6::francisQRStep(QVector<QVector<double>>& H, int lo, int hi,
                                           QVector<QVector<double>>& Q)
{
    int n = H.size();
    // Compute shifts from trailing 2x2 block
    double a = H[hi - 1][hi - 1], b = H[hi - 1][hi];
    double c = H[hi][hi - 1], d = H[hi][hi];
    double tr = a + d;
    double det = a * d - b * c;

    // First column of (H - s1*I)(H - s2*I)
    double h11 = H[lo][lo], h12 = H[lo][lo + 1];
    double h21 = H[lo + 1][lo];
    double p0 = h11 * h11 + h12 * h21 - tr * h11 + det;
    double p1 = h21 * (h11 + h12 - tr);
    double p2 = h21 * H[lo + 2][lo + 1];

    for (int k = lo; k <= hi - 2; ++k) {
        // Compute Householder reflector from [p0, p1, p2]
        double norm = qSqrt(p0 * p0 + p1 * p1 + p2 * p2);
        if (norm < 1e-30) { p0 = H[k + 1][k]; p1 = H[k + 2][k];
            if (k + 3 <= hi) p2 = H[k + 3][k]; else p2 = 0.0; continue; }

        double v0 = p0, v1 = p1, v2 = p2;
        double sign = (v0 >= 0) ? 1.0 : -1.0;
        v0 += sign * norm;
        double vnorm = qSqrt(v0 * v0 + v1 * v1 + v2 * v2);
        if (vnorm < 1e-30) continue;
        v0 /= vnorm; v1 /= vnorm; v2 /= vnorm;

        int r = qMax(lo, k - 1);
        int s_end = qMin(hi, k + 3);
        // Apply from left
        for (int j = r; j < n; ++j) {
            double dot = v0 * H[k][j] + v1 * H[k + 1][j] + v2 * H[k + 2][j];
            H[k][j] -= 2.0 * v0 * dot;
            H[k + 1][j] -= 2.0 * v1 * dot;
            H[k + 2][j] -= 2.0 * v2 * dot;
        }
        // Apply from right
        for (int i = 0; i <= s_end; ++i) {
            double dot = v0 * H[i][k] + v1 * H[i][k + 1] + v2 * H[i][k + 2];
            H[i][k] -= 2.0 * v0 * dot;
            H[i][k + 1] -= 2.0 * v1 * dot;
            H[i][k + 2] -= 2.0 * v2 * dot;
        }
        // Update Q
        for (int i = 0; i < n; ++i) {
            double dot = v0 * Q[i][k] + v1 * Q[i][k + 1] + v2 * Q[i][k + 2];
            Q[i][k] -= 2.0 * v0 * dot;
            Q[i][k + 1] -= 2.0 * v1 * dot;
            Q[i][k + 2] -= 2.0 * v2 * dot;
        }
        // Update bulge chase values
        p0 = H[k + 1][k]; p1 = H[k + 2][k];
        p2 = (k + 3 <= hi) ? H[k + 3][k] : 0.0;
    }
}

/* ---- Aggressive early deflation ---- */

bool SchurDecomposition6::tryDeflation(QVector<QVector<double>>& H, int& lo, int& hi)
{
    bool deflated = false;
    // Check trailing elements for near-zero subdiagonal
    while (hi > lo) {
        double off = qFabs(H[hi][hi - 1]);
        double diagSum = qFabs(H[hi - 1][hi - 1]) + qFabs(H[hi][hi]);
        if (off <= m_tol * diagSum || off < 1e-30) {
            H[hi][hi - 1] = 0.0;
            hi--;
            deflated = true;
        } else {
            break;
        }
    }
    // Also check for 2x2 block deflation
    if (hi > lo + 1) {
        double off2 = qFabs(H[hi - 1][hi - 2]);
        double diagSum2 = qFabs(H[hi - 2][hi - 2]) + qFabs(H[hi - 1][hi - 1]);
        if (off2 <= m_tol * diagSum2 || off2 < 1e-30) {
            H[hi - 1][hi - 2] = 0.0;
            schur2x2(H, hi - 1, hi, m_Q);
            hi -= 2;
            deflated = true;
        }
    }
    if (deflated) m_stats.deflations++;
    return deflated;
}

/* ---- Main decomposition ---- */

bool SchurDecomposition6::decompose(const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();
    int n = matrix.size();
    m_T = matrix;
    m_Q.assign(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) m_Q[i][i] = 1.0;

    hessenbergReduce(m_T, m_Q);

    int lo = 0, hi = n - 1;
    int iter = 0;

    while (hi > lo && iter < m_maxIter) {
        tryDeflation(m_T, lo, hi);
        if (hi <= lo) break;
        if (hi == lo + 1) {
            schur2x2(m_T, lo, hi, m_Q);
            break;
        }
        francisQRStep(m_T, lo, hi, m_Q);
        iter++;
    }

    m_stats.totalOps++;
    m_stats.matrixSize = n;
    m_stats.iterations = iter;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit decompositionCompleted(n, iter, timer.elapsed());
    return iter < m_maxIter;
}

/* ---- Accessors ---- */

QVector<QVector<double>> SchurDecomposition6::schurForm() const { return m_T; }
QVector<QVector<double>> SchurDecomposition6::orthogonalMatrix() const { return m_Q; }

/* ---- Extract eigenvalues ---- */

QVector<QPair<double, double>> SchurDecomposition6::eigenvalues() const
{
    QVector<QPair<double, double>> eigs;
    int n = m_T.size();
    int i = 0;
    while (i < n) {
        if (i == n - 1 || qFabs(m_T[i + 1][i]) < m_tol * (qFabs(m_T[i][i]) + qFabs(m_T[i + 1][i + 1]))) {
            eigs.append({m_T[i][i], 0.0});
            i++;
        } else {
            double a = m_T[i][i], b = m_T[i][i + 1];
            double c = m_T[i + 1][i], d = m_T[i + 1][i + 1];
            double tr = a + d;
            double det = a * d - b * c;
            double disc = tr * tr - 4.0 * det;
            if (disc < 0) {
                eigs.append({tr / 2.0, qSqrt(-disc) / 2.0});
                eigs.append({tr / 2.0, -qSqrt(-disc) / 2.0});
            } else {
                eigs.append({(tr + qSqrt(disc)) / 2.0, 0.0});
                eigs.append({(tr - qSqrt(disc)) / 2.0, 0.0});
            }
            i += 2;
        }
    }
    return eigs;
}

/* ---- Reset ---- */

void SchurDecomposition6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_T.clear();
    m_Q.clear();
}
