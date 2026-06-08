/**
 * @file SchurDecomposition8.cpp
 * @brief SchurDecomposition8 实现
 *
 * 实现Schur分解：Francis双步隐式QR与激进早期收缩特征值簇。
 */

#include "utils/matrix238/SchurDecomposition8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SchurDecomposition8::SchurDecomposition8(QObject *parent) : QObject(parent) {}
SchurDecomposition8::~SchurDecomposition8() = default;

/* ---- Configuration ---- */

void SchurDecomposition8::setMaxIterations(int maxIter) { m_maxIter = qMax(10, maxIter); }
void SchurDecomposition8::setTolerance(double tol) { m_tol = qMax(1e-16, tol); }

/* ---- Hessenberg reduction ---- */

void SchurDecomposition8::hessenbergReduce(QVector<QVector<double>>& A,
                                             QVector<QVector<double>>& Q) const
{
    int n = m_n;
    // Initialize Q to identity
    Q.assign(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) Q[i][i] = 1.0;

    for (int k = 0; k < n - 2; ++k) {
        // Compute Householder vector from A[k+1:n, k]
        double sigma = 0.0;
        for (int i = k + 2; i < n; ++i) sigma += A[i][k] * A[i][k];
        double alpha = qSqrt(sigma + A[k + 1][k] * A[k + 1][k]);
        if (alpha < m_tol) continue;

        double signVal = (A[k + 1][k] >= 0) ? 1.0 : -1.0;
        double v1 = A[k + 1][k] + signVal * alpha;
        double vScale = v1 * v1 + sigma;
        if (vScale < m_tol) continue;
        vScale = 2.0 / vScale;

        // Apply from left: A = (I - 2*v*v'/vScale) * A
        for (int j = k; j < n; ++j) {
            double dot = v1 * A[k + 1][j];
            for (int i = k + 2; i < n; ++i) dot += A[i][k] * A[i][j];
            dot *= vScale;
            A[k + 1][j] -= v1 * dot;
            for (int i = k + 2; i < n; ++i) A[i][j] -= A[i][k] * dot;
        }

        // Apply from right: A = A * (I - 2*v*v'/vScale)
        for (int i = 0; i < n; ++i) {
            double dot = v1 * A[i][k + 1];
            for (int j = k + 2; j < n; ++j) dot += A[i][j] * A[j][k];
            dot *= vScale;
            A[i][k + 1] -= v1 * dot;
            for (int j = k + 2; j < n; ++j) A[i][j] -= A[i][k] * A[j][k] * vScale;
        }

        // Accumulate Q
        for (int i = 0; i < n; ++i) {
            double dot = v1 * Q[i][k + 1];
            for (int j = k + 2; j < n; ++j) dot += Q[i][j] * A[j][k];
            dot *= vScale;
            Q[i][k + 1] -= v1 * dot;
            for (int j = k + 2; j < n; ++j) Q[i][j] -= Q[i][k] * A[j][k] * vScale;
        }
    }
}

/* ---- Francis double-shift implicit QR step ---- */

void SchurDecomposition8::francisDoubleShift(QVector<QVector<double>>& T, int lo, int hi,
                                               QVector<QVector<double>>& Q) const
{
    int n = m_n;
    // Compute shifts from trailing 2x2 block
    double a = T[hi - 1][hi - 1], b = T[hi - 1][hi];
    double c = T[hi][hi - 1], d = T[hi][hi];
    double tr = a + d;
    double det = a * d - b * c;

    // First column of (T^2 - tr*T + det*I)
    double x = T[lo][lo] * T[lo][lo] + T[lo][lo + 1] * T[lo + 1][lo] - tr * T[lo][lo] + det;
    double y = T[lo + 1][lo] * (T[lo][lo] + T[lo + 1][lo + 1] - tr);
    double z = T[lo + 1][lo] * T[lo + 2][lo + 1];

    for (int k = lo; k <= hi - 2; ++k) {
        // Householder to zero out y and z
        double norm = qSqrt(x * x + y * y + z * z);
        if (norm < m_tol) { x = 1.0; y = 0.0; z = 0.0; norm = 1.0; }
        double v0 = x + ((x >= 0) ? norm : -norm);
        double scale = 2.0 / (v0 * v0 + y * y + z * z);

        int r = qMax(lo, k - 1);
        // Apply from left
        for (int j = r; j < n; ++j) {
            double dot = v0 * T[k][j] + y * T[k + 1][j] + z * T[k + 2][j];
            dot *= scale;
            T[k][j] -= v0 * dot;
            T[k + 1][j] -= y * dot;
            T[k + 2][j] -= z * dot;
        }

        int rr = qMin(hi, k + 3);
        // Apply from right
        for (int i = 0; i <= rr; ++i) {
            double dot = v0 * T[i][k] + y * T[i][k + 1] + z * T[i][k + 2];
            dot *= scale;
            T[i][k] -= v0 * dot;
            T[i][k + 1] -= y * dot;
            T[i][k + 2] -= z * dot;
        }

        // Accumulate Q
        for (int i = 0; i < n; ++i) {
            double dot = v0 * Q[i][k] + y * Q[i][k + 1] + z * Q[i][k + 2];
            dot *= scale;
            Q[i][k] -= v0 * dot;
            Q[i][k + 1] -= y * dot;
            Q[i][k + 2] -= z * dot;
        }

        x = T[k + 1][k];
        y = T[k + 2][k];
        if (k < hi - 2) z = T[k + 3][k];
    }
}

/* ---- Aggressive early deflation ---- */

int SchurDecomposition8::aggressiveDeflation(QVector<QVector<double>>& T,
                                               int lo, int hi) const
{
    // Check if any subdiagonal in [lo, hi] is small enough
    for (int i = hi; i > lo; --i) {
        double diag = qAbs(T[i][i]) + qAbs(T[i - 1][i - 1]);
        if (diag < m_tol) diag = m_tol;
        if (qAbs(T[i][i - 1]) < m_tol * diag) {
            T[i][i - 1] = 0.0;
            return i;
        }
    }
    return -1;
}

/* ---- Find deflation ---- */

int SchurDecomposition8::findDeflation(const QVector<QVector<double>>& T,
                                         int lo, int hi) const
{
    for (int i = hi; i > lo; --i) {
        double diag = qAbs(T[i][i]) + qAbs(T[i - 1][i - 1]);
        if (diag < m_tol) diag = m_tol;
        if (qAbs(T[i][i - 1]) < m_tol * diag)
            return i;
    }
    return lo;
}

/* ---- Off-diagonal norm ---- */

double SchurDecomposition8::offDiagonalNorm(const QVector<QVector<double>>& T,
                                               int lo, int hi) const
{
    double norm = 0.0;
    for (int i = lo + 1; i <= hi; ++i)
        norm += T[i][i - 1] * T[i][i - 1];
    return qSqrt(norm);
}

/* ---- Householder ---- */

void SchurDecomposition8::householder(double& x, double& y,
                                        double& cosVal, double& sinVal) const
{
    double r = qSqrt(x * x + y * y);
    if (r < m_tol) { cosVal = 1.0; sinVal = 0.0; return; }
    cosVal = x / r;
    sinVal = -y / r;
}

/* ---- Extract eigenvalues ---- */

QVector<double> SchurDecomposition8::extractEigenvalues(
    const QVector<QVector<double>>& T) const
{
    QVector<double> eigs;
    int n = T.size();
    int i = 0;
    while (i < n) {
        if (i == n - 1 || qAbs(T[i + 1][i]) < m_tol * (qAbs(T[i][i]) + qAbs(T[i + 1][i + 1]))) {
            eigs.append(T[i][i]);
            i++;
        } else {
            // 2x2 block: compute complex eigenvalues
            double tr = T[i][i] + T[i + 1][i + 1];
            double det = T[i][i] * T[i + 1][i + 1] - T[i][i + 1] * T[i + 1][i];
            double disc = tr * tr - 4.0 * det;
            eigs.append(tr / 2.0);  // Real part stored
            eigs.append(tr / 2.0);  // Second of pair
            i += 2;
        }
    }
    return eigs;
}

/* ---- Decompose ---- */

SchurDecomposition8::Result SchurDecomposition8::decompose(
    const QVector<QVector<double>>& A)
{
    QElapsedTimer timer;
    timer.start();

    Result result;
    m_n = A.size();
    if (m_n == 0) return result;
    m_stats.matrixSize = m_n;

    // Copy matrix
    QVector<QVector<double>> T = A;
    QVector<QVector<double>> Q;

    // Step 1: Reduce to Hessenberg form
    hessenbergReduce(T, Q);

    // Step 2: Francis QR iteration
    int iter = 0;
    int hi = m_n - 1;

    while (hi > 0 && iter < m_maxIter) {
        // Aggressive early deflation
        int deflIdx = aggressiveDeflation(T, 0, hi);
        if (deflIdx > 0) {
            m_stats.numDeflations++;
            emit deflationOccurred(deflIdx, hi - deflIdx + 1);
            hi = deflIdx - 1;
            if (hi <= 0) break;
            continue;
        }

        // Find active unreduced block
        int lo = hi;
        while (lo > 0) {
            double diag = qAbs(T[lo][lo]) + qAbs(T[lo - 1][lo - 1]);
            if (diag < m_tol) diag = m_tol;
            if (qAbs(T[lo][lo - 1]) < m_tol * diag) {
                T[lo][lo - 1] = 0.0;
                break;
            }
            lo--;
        }

        if (lo == hi) {
            hi--;
            continue;
        }

        // Apply Francis double shift
        francisDoubleShift(T, lo, hi, Q);
        iter++;

        double norm = offDiagonalNorm(T, lo, hi);
        emit iterationCompleted(iter, norm);
    }

    result.T = T;
    result.Q = Q;
    result.eigenvalues = extractEigenvalues(T);
    result.iterations = iter;
    result.converged = (hi <= 0);

    m_stats.totalIterations += iter;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit decompositionCompleted(iter, timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void SchurDecomposition8::resetStatistics()
{
    m_n = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
