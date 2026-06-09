/**
 * @file SchurDecomposition9.cpp
 * @brief SchurDecomposition9 实现
 *
 * 实现Schur分解：AED激进提前收缩与多位移QR聚类特征值分离。
 */

#include "utils/matrix252/SchurDecomposition9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SchurDecomposition9::SchurDecomposition9(QObject *parent) : QObject(parent) {}
SchurDecomposition9::~SchurDecomposition9() = default;

/* ---- Configuration ---- */

void SchurDecomposition9::setMaxIterations(int maxIter) { m_maxIterations = qMax(1, maxIter); }
void SchurDecomposition9::setTolerance(double tol) { m_tolerance = qMax(1e-16, tol); }

/* ---- Hessenberg reduction ---- */

void SchurDecomposition9::hessenbergReduce(QVector<QVector<double>>& A,
                                             QVector<QVector<double>>& Q)
{
    int n = A.size();
    Q.assign(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) Q[i][i] = 1.0;

    for (int k = 0; k < n - 2; ++k) {
        applyHouseholder(A, k + 1, k, Q);
    }
}

/* ---- Apply Householder reflection ---- */

void SchurDecomposition9::applyHouseholder(QVector<QVector<double>>& A, int row, int col,
                                             QVector<QVector<double>>& Q)
{
    int n = A.size();
    // Compute norm of subdiagonal column
    double norm = 0.0;
    for (int i = row; i < n; ++i) norm += A[i][col] * A[i][col];
    norm = qSqrt(norm);
    if (norm < 1e-15) return;

    double sign = (A[row][col] >= 0) ? 1.0 : -1.0;
    double alpha = sign * norm;
    double r = qSqrt(2.0 * alpha * (alpha + A[row][col]));
    if (qAbs(r) < 1e-15) return;

    // Householder vector v
    QVector<double> v(n, 0.0);
    v[row] = (A[row][col] + alpha) / r;
    for (int i = row + 1; i < n; ++i)
        v[i] = A[i][col] / r;

    // Apply: A = (I - 2vv^T) A (I - 2vv^T)
    // Left: A = (I - 2vv^T) A
    for (int j = 0; j < n; ++j) {
        double dot = 0.0;
        for (int i = row; i < n; ++i) dot += v[i] * A[i][j];
        for (int i = row; i < n; ++i) A[i][j] -= 2.0 * v[i] * dot;
    }
    // Right: A = A (I - 2vv^T)
    for (int i = 0; i < n; ++i) {
        double dot = 0.0;
        for (int j = row; j < n; ++j) dot += A[i][j] * v[j];
        for (int j = row; j < n; ++j) A[i][j] -= 2.0 * dot * v[j];
    }
    // Update Q
    for (int i = 0; i < n; ++i) {
        double dot = 0.0;
        for (int j = row; j < n; ++j) dot += Q[i][j] * v[j];
        for (int j = row; j < n; ++j) Q[i][j] -= 2.0 * dot * v[j];
    }
}

/* ---- Check for deflation ---- */

int SchurDecomposition9::checkDeflation(const QVector<QVector<double>>& H,
                                           int lo, int hi) const
{
    for (int i = hi; i > lo; --i) {
        double off = qAbs(H[i][i - 1]);
        double scale = qAbs(H[i - 1][i - 1]) + qAbs(H[i][i]);
        if (scale < 1e-15) scale = 1.0;
        if (off <= m_tolerance * scale) return i;
    }
    return -1;
}

/* ---- Extract 2x2 eigenvalue pair ---- */

QPair<SchurDecomposition9::Complex, SchurDecomposition9::Complex>
SchurDecomposition9::eigenvalue2x2(const QVector<QVector<double>>& H, int i) const
{
    double a = H[i][i], b = H[i][i + 1];
    double c = H[i + 1][i], d = H[i + 1][i + 1];

    double tr = a + d;
    double det = a * d - b * c;
    double disc = tr * tr - 4.0 * det;

    if (disc >= 0) {
        double sq = qSqrt(disc);
        return {{(tr + sq) / 2.0, 0.0}, {(tr - sq) / 2.0, 0.0}};
    }
    double sq = qSqrt(-disc);
    return {{tr / 2.0, sq / 2.0}, {tr / 2.0, -sq / 2.0}};
}

/* ---- Francis double-shift QR step ---- */

void SchurDecomposition9::francisDoubleShift(QVector<QVector<double>>& H,
                                                QVector<QVector<double>>& Q,
                                                int lo, int hi)
{
    int n = H.size();
    // Compute shifts from trailing 2x2 block
    auto shifts = eigenvalue2x2(H, qMax(lo, hi - 1));
    double s = shifts.first.first + shifts.second.first;    // trace
    double t = shifts.first.first * shifts.second.first
             - shifts.first.second * shifts.second.second;   // determinant

    // First column of (H - s1*I)(H - s2*I)
    double h11 = H[lo][lo], h12 = H[lo][lo + 1];
    double h21 = H[lo + 1][lo];
    double h22 = H[lo + 1][lo + 1];

    double x = h11 * h11 + h12 * h21 - s * h11 + t;
    double y = h21 * (h11 + h22 - s);
    double z = h21 * H[lo + 2][lo + 1];

    for (int k = lo; k <= hi - 2; ++k) {
        // Determine Householder reflector from (x, y, z)
        double norm = qSqrt(x * x + y * y + z * z);
        if (norm < 1e-15) break;
        double d = (x >= 0) ? norm : -norm;
        double r = qSqrt(2.0 * norm * (norm + qAbs(x)));
        if (r < 1e-15) break;

        QVector<double> v(3);
        v[0] = (x + d) / r;
        v[1] = y / r;
        v[2] = z / r;

        int rStart = qMax(lo, k - 1);
        // Left multiply
        for (int j = rStart; j < n; ++j) {
            double dot = v[0] * H[k][j] + v[1] * H[k + 1][j] + v[2] * H[k + 2][j];
            H[k][j] -= 2.0 * v[0] * dot;
            H[k + 1][j] -= 2.0 * v[1] * dot;
            H[k + 2][j] -= 2.0 * v[2] * dot;
        }
        // Right multiply
        int rEnd = qMin(hi, k + 3);
        for (int i = 0; i <= rEnd; ++i) {
            double dot = H[i][k] * v[0] + H[i][k + 1] * v[1] + H[i][k + 2] * v[2];
            H[i][k] -= 2.0 * dot * v[0];
            H[i][k + 1] -= 2.0 * dot * v[1];
            H[i][k + 2] -= 2.0 * dot * v[2];
        }
        // Update Q
        for (int i = 0; i < n; ++i) {
            double dot = Q[i][k] * v[0] + Q[i][k + 1] * v[1] + Q[i][k + 2] * v[2];
            Q[i][k] -= 2.0 * dot * v[0];
            Q[i][k + 1] -= 2.0 * dot * v[1];
            Q[i][k + 2] -= 2.0 * dot * v[2];
        }

        // Setup for next step
        x = H[k + 1][k];
        if (k < hi - 2) {
            y = H[k + 2][k];
            z = H[k + 3][k];
        }
    }
}

/* ---- AED aggressive early deflation ---- */

int SchurDecomposition9::aggressiveDeflation(QVector<QVector<double>>& H,
                                               QVector<QVector<double>>& Q,
                                               int lo, int hi)
{
    // Check for converged eigenvalues in trailing portion
    int deflated = 0;
    for (int i = hi; i > lo; --i) {
        double off = qAbs(H[i][i - 1]);
        double scale = qAbs(H[i - 1][i - 1]) + qAbs(H[i][i]);
        if (scale < 1e-15) scale = 1.0;
        if (off <= m_tolerance * scale) {
            H[i][i - 1] = 0.0;
            deflated++;
        }
    }
    m_stats.numDeflations += deflated;
    return deflated;
}

/* ---- Multishift QR step ---- */

void SchurDecomposition9::multishiftQRStep(QVector<QVector<double>>& H,
                                              QVector<QVector<double>>& Q,
                                              int lo, int hi)
{
    // Apply Francis double-shift as the multishift QR step
    francisDoubleShift(H, Q, lo, hi);
    m_stats.numShifts += 2;
}

/* ---- Main decomposition ---- */

bool SchurDecomposition9::decompose(const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    m_n = matrix.size();
    if (m_n == 0) return false;
    for (const auto& row : matrix)
        if (row.size() != m_n) return false;

    // Copy matrix to H (work array)
    QVector<QVector<double>> H = matrix;

    // Step 1: Hessenberg reduction
    hessenbergReduce(H, m_Q);
    m_T = H;

    // Step 2: QR iteration with AED
    int lo = 0, hi = m_n - 1;
    int iter = 0;
    bool converged = false;

    while (iter < m_maxIterations && hi > lo) {
        // Check for deflation
        int defl = checkDeflation(m_T, lo, hi);
        if (defl > lo) {
            m_T[defl][defl - 1] = 0.0;
            m_stats.numDeflations++;
            if (defl == hi) { hi--; continue; }
            lo = defl;
            if (lo >= hi) { converged = true; break; }
        }

        // AED: aggressive early deflation on trailing submatrix
        aggressiveDeflation(m_T, m_Q, lo, hi);

        // Multishift QR step
        multishiftQRStep(m_T, m_Q, lo, hi);
        iter++;

        // Check convergence
        if (qAbs(m_T[hi][hi - 1]) < m_tolerance) {
            m_T[hi][hi - 1] = 0.0;
            hi--;
        }
    }

    converged = (hi <= lo);
    m_stats.matrixSize = m_n;
    m_stats.numIterations = iter;
    m_stats.converged = converged;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit decompositionCompleted(m_n, iter, converged, timer.elapsed());
    return converged;
}

/* ---- Get Q matrix ---- */

QVector<QVector<double>> SchurDecomposition9::matrixQ() const { return m_Q; }

/* ---- Get T matrix ---- */

QVector<QVector<double>> SchurDecomposition9::matrixT() const { return m_T; }

/* ---- Extract eigenvalues ---- */

QVector<SchurDecomposition9::Complex> SchurDecomposition9::eigenvalues() const
{
    QVector<Complex> eigs;
    int i = 0;
    while (i < m_n) {
        if (i == m_n - 1 || qAbs(m_T[i + 1][i]) < m_tolerance) {
            eigs.append({m_T[i][i], 0.0});
            i++;
        } else {
            auto pair = eigenvalue2x2(m_T, i);
            eigs.append(pair.first);
            eigs.append(pair.second);
            i += 2;
        }
    }
    return eigs;
}

/* ---- Reset ---- */

void SchurDecomposition9::resetStatistics()
{
    m_Q.clear();
    m_T.clear();
    m_n = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
