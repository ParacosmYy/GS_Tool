/**
 * @file SchurDecomposition4.cpp
 * @brief SchurDecomposition4 实现
 *
 * 实现Schur分解：Hessenberg约化、Francis双移QR迭代、特征值提取。
 */

#include "utils/matrix179/SchurDecomposition4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SchurDecomposition4::SchurDecomposition4(QObject *parent)
    : QObject(parent)
{
}

SchurDecomposition4::~SchurDecomposition4() = default;

/* ---- Configuration ---- */

void SchurDecomposition4::setMaxIterations(int iter) { m_maxIter = qMax(10, iter); }
void SchurDecomposition4::setConvergenceThreshold(double eps) { m_eps = qMax(1e-16, eps); }

/* ---- Identity matrix ---- */

QVector<QVector<double>> SchurDecomposition4::identity(int n)
{
    QVector<QVector<double>> I(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) I[i][i] = 1.0;
    return I;
}

/* ---- 2x2 block eigenvalues ---- */

QPair<double, double> SchurDecomposition4::eigenvalues2x2(
    const QVector<QVector<double>>& A, int i)
{
    double a = A[i][i], b = A[i][i + 1];
    double c = A[i + 1][i], d = A[i + 1][i + 1];
    double tr = a + d;
    double det = a * d - b * c;
    double disc = tr * tr - 4.0 * det;
    if (disc >= 0.0) {
        double sq = qSqrt(disc);
        return {0.5 * (tr + sq), 0.5 * (tr - sq)};
    }
    double sq = qSqrt(-disc);
    return {0.5 * tr, 0.5 * sq};
}

/* ---- Hessenberg reduction ---- */

void SchurDecomposition4::hessenbergReduce(
    QVector<QVector<double>>& A,
    QVector<QVector<double>>& Q) const
{
    int n = A.size();
    for (int k = 0; k < n - 2; ++k) {
        /* Compute Householder reflector for column k */
        double sigma = 0.0;
        for (int i = k + 2; i < n; ++i)
            sigma += A[i][k] * A[i][k];

        if (sigma < m_eps * m_eps) continue;

        double alpha = A[k + 1][k];
        double normX = qSqrt(alpha * alpha + sigma);
        double v0 = alpha;
        if (alpha >= 0) v0 += normX;
        else v0 -= normX;

        double beta = 2.0 / (v0 * v0 + sigma);

        /* Apply H = I - beta*v*v^T from left */
        for (int j = k; j < n; ++j) {
            double dot = A[k + 1][j] * v0;
            for (int i = k + 2; i < n; ++i)
                dot += A[i][j] * A[i][k];
            A[k + 1][j] -= beta * v0 * dot;
            for (int i = k + 2; i < n; ++i)
                A[i][j] -= beta * A[i][k] * dot;
        }

        /* Apply H from right */
        for (int i = 0; i < n; ++i) {
            double dot = A[i][k + 1] * v0;
            for (int j = k + 2; j < n; ++j)
                dot += A[i][j] * A[j][k];
            A[i][k + 1] -= beta * v0 * dot;
            for (int j = k + 2; j < n; ++j)
                A[i][j] -= beta * A[j][k] * dot;
        }

        /* Accumulate Q */
        for (int i = 0; i < n; ++i) {
            double dot = Q[i][k + 1] * v0;
            for (int j = k + 2; j < n; ++j)
                dot += Q[i][j] * A[j][k];
            Q[i][k + 1] -= beta * v0 * dot;
            for (int j = k + 2; j < n; ++j)
                Q[i][j] -= beta * A[j][k] * dot;
        }
    }
}

/* ---- Francis double-shift QR step ---- */

void SchurDecomposition4::francisDoubleShift(
    QVector<QVector<double>>& A,
    QVector<QVector<double>>& Q,
    int p, int q)
{
    int n = A.size();
    if (q - p < 2) return;

    /* Compute shift from trailing 2x2 block */
    auto shifts = eigenvalues2x2(A, q - 2);
    double s = shifts.first + shifts.second;   /* trace */
    double t = shifts.first * shifts.second;    /* determinant */

    /* Compute first column of (A^2 - s*A + t*I) */
    double x = A[p][p] * A[p][p] + A[p][p + 1] * A[p + 1][p] - s * A[p][p] + t;
    double y = A[p + 1][p] * (A[p][p] + A[p + 1][p + 1] - s);
    double z = A[p + 1][p] * A[p + 2][p + 1];

    for (int k = p; k < q - 2; ++k) {
        /* Build Givens rotations to chase bulge */
        double r = qSqrt(x * x + y * y + z * z);
        if (r < m_eps) break;
        double c1 = x / r, s1 = y / r, s2 = z / r;

        /* Apply rotation from left and right */
        int rows = qMin(k + 4, n);
        for (int j = k; j < rows; ++j) {
            double t1 = c1 * A[k][j] + s1 * A[k + 1][j] + s2 * A[k + 2][j];
            double t2 = -s1 * A[k][j] + c1 * A[k + 1][j];
            A[k][j] = t1;
            A[k + 1][j] = t2;
        }
        for (int i = 0; i < rows; ++i) {
            double t1 = c1 * A[i][k] + s1 * A[i][k + 1] + s2 * A[i][k + 2];
            double t2 = -s1 * A[i][k] + c1 * A[i][k + 1];
            A[i][k] = t1;
            A[i][k + 1] = t2;
        }

        /* Accumulate Q */
        for (int i = 0; i < n; ++i) {
            double t1 = c1 * Q[i][k] + s1 * Q[i][k + 1] + s2 * Q[i][k + 2];
            double t2 = -s1 * Q[i][k] + c1 * Q[i][k + 1];
            Q[i][k] = t1;
            Q[i][k + 1] = t2;
        }

        /* Prepare for next iteration */
        x = A[k + 1][k];
        if (k + 2 < q) {
            y = A[k + 2][k];
            z = (k + 3 < q) ? A[k + 3][k] : 0.0;
        }
    }
}

/* ---- Check convergence ---- */

bool SchurDecomposition4::checkConvergence(
    const QVector<QVector<double>>& A) const
{
    int n = A.size();
    for (int i = 1; i < n; ++i)
        if (qAbs(A[i][i - 1]) > m_eps * (qAbs(A[i - 1][i - 1]) + qAbs(A[i][i])))
            return false;
    return true;
}

/* ---- Main decompose ---- */

SchurDecomposition4::Result SchurDecomposition4::decompose(
    const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    Result result;
    int n = matrix.size();
    if (n == 0) return result;

    /* Copy matrix */
    result.T = matrix;
    result.Q = identity(n);

    /* Step 1: Reduce to upper Hessenberg form */
    hessenbergReduce(result.T, result.Q);

    /* Step 2: Francis double-shift QR iteration */
    int q = n;
    for (int iter = 0; iter < m_maxIter; ++iter) {
        result.iterations = iter + 1;

        /* Deflate converged eigenvalues */
        while (q > 1 && qAbs(result.T[q - 1][q - 2]) <
               m_eps * (qAbs(result.T[q - 2][q - 2]) + qAbs(result.T[q - 1][q - 1])))
            q--;

        if (q <= 1) { result.converged = true; break; }

        /* Find active block */
        int p = q - 1;
        while (p > 0 && qAbs(result.T[p][p - 1]) >
               m_eps * (qAbs(result.T[p - 1][p - 1]) + qAbs(result.T[p][p])))
            p--;

        if (q - p == 1) { q = p; continue; }

        francisDoubleShift(result.T, result.Q, p, q);

        if (checkConvergence(result.T)) {
            result.converged = true;
            break;
        }
    }

    /* Step 3: Extract eigenvalues from quasi-triangular T */
    result.eigenvalues.reserve(n);
    int i = 0;
    while (i < n) {
        if (i + 1 < n && qAbs(result.T[i + 1][i]) > m_eps) {
            /* 2x2 block: complex conjugate pair */
            auto ev = eigenvalues2x2(result.T, i);
            result.eigenvalues.append(ev);
            result.eigenvalues.append({ev.first, -ev.second});
            i += 2;
        } else {
            result.eigenvalues.append({result.T[i][i], 0.0});
            i++;
        }
    }

    m_stats.totalDecompositions++;
    m_stats.matrixSize = n;
    m_stats.lastIterations = result.iterations;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    emit decompositionCompleted(n, result.iterations, result.converged);
    return result;
}

/* ---- Eigenvalues only ---- */

QVector<QPair<double, double>> SchurDecomposition4::eigenvalues(
    const QVector<QVector<double>>& matrix)
{
    auto result = decompose(matrix);
    return result.eigenvalues;
}

/* ---- Statistics ---- */

void SchurDecomposition4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
