/**
 * @file SchurDecomposition7.cpp
 * @brief SchurDecomposition7 实现
 *
 * 实现Schur分解：Hessenberg归约、Francis双隐式位移QR、AED加速收缩。
 */

#include "utils/matrix224/SchurDecomposition7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SchurDecomposition7::SchurDecomposition7(QObject *parent) : QObject(parent) {}
SchurDecomposition7::~SchurDecomposition7() = default;

/* ---- Configuration ---- */

void SchurDecomposition7::setParameters(int maxIterations, double tolerance)
{
    m_maxIterations = qMax(10, maxIterations);
    m_tolerance = qMax(1e-16, tolerance);
}

/* ---- Identity matrix ---- */

QVector<QVector<double>> SchurDecomposition7::identity(int n) const
{
    QVector<QVector<double>> I(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) I[i][i] = 1.0;
    return I;
}

/* ---- Matrix multiply ---- */

QVector<QVector<double>> SchurDecomposition7::matMul(
    const QVector<QVector<double>>& A,
    const QVector<QVector<double>>& B) const
{
    int n = A.size();
    QVector<QVector<double>> C(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        for (int k = 0; k < n; ++k)
            for (int j = 0; j < n; ++j)
                C[i][j] += A[i][k] * B[k][j];
    return C;
}

/* ---- 2x2 eigenvalues ---- */

QVector<double> SchurDecomposition7::eigenvalues2x2(double a, double b,
                                                       double c, double d) const
{
    double trace = a + d;
    double det = a * d - b * c;
    double disc = trace * trace - 4.0 * det;
    QVector<double> eigvals(2);
    if (disc >= 0.0) {
        double sq = qSqrt(disc);
        eigvals[0] = (trace + sq) / 2.0;
        eigvals[1] = (trace - sq) / 2.0;
    } else {
        eigvals[0] = trace / 2.0; // Real part
        eigvals[1] = qSqrt(-disc) / 2.0; // Imaginary part
    }
    return eigvals;
}

/* ---- Givens rotations ---- */

void SchurDecomposition7::applyGivensLeft(QVector<QVector<double>>& A,
                                            int i, int j, double c, double s)
{
    int n = A.size();
    for (int k = 0; k < n; ++k) {
        double ti = c * A[i][k] + s * A[j][k];
        double tj = -s * A[i][k] + c * A[j][k];
        A[i][k] = ti;
        A[j][k] = tj;
    }
}

void SchurDecomposition7::applyGivensRight(QVector<QVector<double>>& A,
                                             int i, int j, double c, double s)
{
    int n = A.size();
    for (int k = 0; k < n; ++k) {
        double ti = c * A[k][i] + s * A[k][j];
        double tj = -s * A[k][i] + c * A[k][j];
        A[k][i] = ti;
        A[k][j] = tj;
    }
}

/* ---- Hessenberg reduction ---- */

void SchurDecomposition7::hessenbergReduction(QVector<QVector<double>>& A,
                                                 QVector<QVector<double>>& Q)
{
    int n = A.size();
    Q = identity(n);

    for (int k = 0; k < n - 2; ++k) {
        // Compute Householder vector for column k below diagonal
        double sigma = 0.0;
        for (int i = k + 2; i < n; ++i) sigma += A[i][k] * A[i][k];

        if (sigma < m_tolerance * qAbs(A[k + 1][k])) continue;

        double alpha = qSqrt(A[k + 1][k] * A[k + 1][k] + sigma);
        if (A[k + 1][k] >= 0) alpha = -alpha;

        double r = qSqrt(0.5 * alpha * (alpha - A[k + 1][k]));
        if (r < m_tolerance) continue;

        QVector<double> v(n, 0.0);
        v[k + 1] = (A[k + 1][k] - alpha) / (2.0 * r);
        for (int i = k + 2; i < n; ++i)
            v[i] = A[i][k] / (2.0 * r);

        // Apply: A = (I - 2vv^T) A (I - 2vv^T)
        for (int j = k; j < n; ++j) {
            double dot = 0.0;
            for (int i = k + 1; i < n; ++i) dot += v[i] * A[i][j];
            for (int i = k + 1; i < n; ++i) A[i][j] -= 2.0 * v[i] * dot;
        }
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = k + 1; j < n; ++j) dot += v[j] * A[i][j];
            for (int j = k + 1; j < n; ++j) A[i][j] -= 2.0 * v[j] * dot;
        }
        // Update Q
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = k + 1; j < n; ++j) dot += v[j] * Q[i][j];
            for (int j = k + 1; j < n; ++j) Q[i][j] -= 2.0 * v[j] * dot;
        }
    }
}

/* ---- Francis QR step ---- */

void SchurDecomposition7::francisQRStep(QVector<QVector<double>>& H,
                                           int lo, int hi,
                                           QVector<QVector<double>>& Q)
{
    int n = H.size();
    // Compute shift from trailing 2x2 block
    double a = H[hi - 1][hi - 1], b = H[hi - 1][hi];
    double c = H[hi][hi - 1], d = H[hi][hi];
    QVector<double> eig = eigenvalues2x2(a, b, c, d);

    // Double implicit shift
    double s1 = eig[0], s2 = (eig.size() > 1) ? eig[1] : s1;
    double p = s1 + s2;
    double q = s1 * s2;

    // First column of M = H^2 - p*H + q*I
    double x = H[lo][lo] * H[lo][lo] + H[lo][lo + 1] * H[lo + 1][lo] - p * H[lo][lo] + q;
    double y = H[lo + 1][lo] * (H[lo][lo] + H[lo + 1][lo + 1] - p);
    double z = H[lo + 1][lo] * H[lo + 2][lo + 1];

    for (int k = lo; k <= hi - 2; ++k) {
        // Bulge chase via Givens rotations
        double r = qSqrt(x * x + y * y + z * z);
        if (r < m_tolerance) { x = y = z = 0.0; continue; }
        double c1 = x / r, s1g = y / r, s2g = z / r;

        int row1 = k, row2 = k + 1, row3 = k + 2;
        if (row3 > hi) break;

        // Apply Givens from left
        for (int j = qMax(0, k - 1); j < n; ++j) {
            double t1 = c1 * H[row1][j] + s1g * H[row2][j];
            double t2 = -s1g * H[row1][j] + c1 * H[row2][j] + s2g * H[row3][j];
            double t3 = -s2g * H[row2][j] + c1 * H[row3][j]; // Approximate
            H[row1][j] = t1;
            H[row2][j] = t2;
            H[row3][j] = t3;
        }
        // Apply from right
        for (int i = qMax(0, k - 1); i <= qMin(hi, k + 3); ++i) {
            double t1 = c1 * H[i][row1] + s1g * H[i][row2];
            double t2 = -s1g * H[i][row1] + c1 * H[i][row2];
            H[i][row1] = t1;
            H[i][row2] = t2;
        }
        // Update Q
        for (int i = 0; i < n; ++i) {
            double t1 = c1 * Q[i][row1] + s1g * Q[i][row2];
            double t2 = -s1g * Q[i][row1] + c1 * Q[i][row2];
            Q[i][row1] = t1;
            Q[i][row2] = t2;
        }

        x = H[k + 1][k];
        y = H[k + 2][k];
        z = (k + 3 <= hi) ? H[k + 3][k] : 0.0;
    }
}

/* ---- Deflation check ---- */

int SchurDecomposition7::checkDeflation(const QVector<QVector<double>>& H,
                                           int lo, int hi) const
{
    for (int i = lo; i < hi; ++i) {
        if (qAbs(H[i + 1][i]) <= m_tolerance *
            (qAbs(H[i][i]) + qAbs(H[i + 1][i + 1]))) {
            return i;
        }
    }
    return -1;
}

/* ---- AED ---- */

int SchurDecomposition7::aggressiveEarlyDeflation(QVector<QVector<double>>& H,
                                                     int lo, int hi,
                                                     QVector<QVector<double>>& Q)
{
    int window = qMin(32, hi - lo + 1);
    int deflated = 0;

    for (int i = hi; i > hi - window && i > lo; --i) {
        if (qAbs(H[i][i - 1]) <= m_tolerance *
            (qAbs(H[i - 1][i - 1]) + qAbs(H[i][i]))) {
            H[i][i - 1] = 0.0;
            deflated++;
        }
    }
    return deflated;
}

/* ---- Decompose ---- */

void SchurDecomposition7::decompose(const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    m_n = matrix.size();
    if (m_n == 0) return;
    m_stats.matrixSize = m_n;

    m_T = matrix;
    m_Q = identity(m_n);

    // Step 1: Reduce to upper Hessenberg form
    hessenbergReduction(m_T, m_Q);

    // Step 2: QR iteration with Francis shifts and AED
    int lo = 0, hi = m_n - 1;
    int iter = 0;
    int totalDeflations = 0;
    int aedDeflations = 0;

    while (hi > lo && iter < m_maxIterations) {
        // Check deflation
        int deflPos = checkDeflation(m_T, lo, hi);
        if (deflPos >= 0) {
            m_T[deflPos + 1][deflPos] = 0.0;
            totalDeflations++;
            if (deflPos == hi - 1) hi -= 2;
            else hi = deflPos;
            continue;
        }

        // Aggressive early deflation
        int aed = aggressiveEarlyDeflation(m_T, lo, hi, m_Q);
        aedDeflations += aed;
        if (aed > 0) { hi -= aed; continue; }

        // Francis double-shift QR step
        francisQRStep(m_T, lo, hi, m_Q);
        iter++;
    }

    m_stats.qrIterations = iter;
    m_stats.deflations = totalDeflations;
    m_stats.aedDeflations = aedDeflations;
    m_stats.converged = (hi <= lo);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit decompositionCompleted(m_n, iter, m_stats.converged, timer.elapsed());
}

/* ---- Accessors ---- */

QVector<QVector<double>> SchurDecomposition7::schurForm() const { return m_T; }
QVector<QVector<double>> SchurDecomposition7::transformationMatrix() const { return m_Q; }

QVector<double> SchurDecomposition7::realEigenvalues() const
{
    QVector<double> reals;
    int i = 0;
    while (i < m_n) {
        if (i + 1 < m_n && qAbs(m_T[i + 1][i]) > m_tolerance) {
            // 2x2 block -> complex pair, skip
            i += 2;
        } else {
            reals.append(m_T[i][i]);
            i++;
        }
    }
    return reals;
}

QVector<QVector<double>> SchurDecomposition7::complexEigenvaluePairs() const
{
    QVector<QVector<double>> pairs;
    int i = 0;
    while (i < m_n) {
        if (i + 1 < m_n && qAbs(m_T[i + 1][i]) > m_tolerance) {
            QVector<double> eig = eigenvalues2x2(m_T[i][i], m_T[i][i + 1],
                                                   m_T[i + 1][i], m_T[i + 1][i + 1]);
            if (eig.size() >= 2 && eig[1] != 0.0) {
                QVector<double> pair(2);
                pair[0] = eig[0]; // Real part
                pair[1] = eig[1]; // Imaginary part
                pairs.append(pair);
            }
            i += 2;
        } else {
            i++;
        }
    }
    return pairs;
}

/* ---- Reset ---- */

void SchurDecomposition7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_T.clear();
    m_Q.clear();
    m_n = 0;
}
