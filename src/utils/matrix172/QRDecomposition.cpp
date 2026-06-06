/**
 * @file QRDecomposition.cpp
 * @brief QRDecomposition 实现
 *
 * 实现QR分解：Householder反射、列主元选取、秩估计、最小二乘求解。
 */

#include "utils/matrix172/QRDecomposition.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

QRDecomposition::QRDecomposition(QObject *parent)
    : QObject(parent)
{
}

QRDecomposition::~QRDecomposition() = default;

void QRDecomposition::setTolerance(double tol) { m_tolerance = qMax(1e-15, tol); }

QVector<double> QRDecomposition::householderVector(const QVector<double>& x)
{
    int n = x.size();
    if (n == 0) return {};

    double normX = 0.0;
    for (int i = 0; i < n; ++i) normX += x[i] * x[i];
    normX = qSqrt(normX);

    QVector<double> v = x;
    v[0] += (x[0] >= 0 ? normX : -normX);

    double normV = 0.0;
    for (double val : v) normV += val * val;
    if (normV > 1e-30) {
        double inv = 1.0 / qSqrt(normV);
        for (double& val : v) val *= inv;
    }
    return v;
}

void QRDecomposition::applyHouseholder(QVector<QVector<double>>& M, int col,
                                         const QVector<double>& v, int startRow)
{
    int rows = M.size();
    int cols = (rows > 0) ? M[0].size() : 0;

    for (int j = col; j < cols; ++j) {
        double dot = 0.0;
        for (int i = 0; i < v.size(); ++i)
            dot += v[i] * M[startRow + i][j];
        for (int i = 0; i < v.size(); ++i)
            M[startRow + i][j] -= 2.0 * v[i] * dot;
    }
}

QVector<QVector<double>> QRDecomposition::transpose(
    const QVector<QVector<double>>& M)
{
    if (M.isEmpty()) return {};
    int rows = M.size(), cols = M[0].size();
    QVector<QVector<double>> T(cols, QVector<double>(rows));
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            T[j][i] = M[i][j];
    return T;
}

QVector<double> QRDecomposition::matVec(const QVector<QVector<double>>& M,
                                          const QVector<double>& v)
{
    int rows = M.size();
    QVector<double> result(rows, 0.0);
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < v.size(); ++j)
            result[i] += M[i][j] * v[j];
    return result;
}

QRDecomposition::QRResult QRDecomposition::decompose(
    const QVector<QVector<double>>& A)
{
    QElapsedTimer timer;
    timer.start();

    int m = A.size();
    int n = (m > 0) ? A[0].size() : 0;
    QRResult result;

    if (m == 0 || n == 0) return result;

    /* Working copy R */
    QVector<QVector<double>> R = A;
    result.permutation.resize(n);
    for (int i = 0; i < n; ++i) result.permutation[i] = i;

    /* Column norms for pivoting */
    QVector<double> colNorms(n, 0.0);
    for (int j = 0; j < n; ++j)
        for (int i = 0; i < m; ++i)
            colNorms[j] += R[i][j] * R[i][j];

    /* Collect Householder reflectors for Q */
    QVector<QVector<double>> reflectors;

    int rank = 0;
    for (int k = 0; k < qMin(m, n); ++k) {
        /* Pivot: find column with largest remaining norm */
        int maxCol = k;
        double maxNorm = colNorms[k];
        for (int j = k + 1; j < n; ++j) {
            if (colNorms[j] > maxNorm) {
                maxNorm = colNorms[j];
                maxCol = j;
            }
        }

        if (maxCol != k) {
            /* Swap columns in R and permutation */
            for (int i = 0; i < m; ++i) std::swap(R[i][k], R[i][maxCol]);
            std::swap(colNorms[k], colNorms[maxCol]);
            std::swap(result.permutation[k], result.permutation[maxCol]);
        }

        /* Skip if column is numerically zero */
        if (qSqrt(qMax(maxNorm, 0.0)) < m_tolerance) break;
        rank++;

        /* Extract Householder vector */
        QVector<double> x(m - k);
        for (int i = k; i < m; ++i) x[i - k] = R[i][k];
        QVector<double> v = householderVector(x);
        reflectors.append(v);

        /* Apply H to R */
        applyHouseholder(R, k, v, k);

        /* Update column norms */
        for (int j = k + 1; j < n; ++j) {
            colNorms[j] -= R[k][j] * R[k][j];
            colNorms[j] = qMax(colNorms[j], 0.0);
        }
    }

    result.rank = rank;

    /* Extract R (upper triangular part) */
    result.R.assign(m, QVector<double>(n, 0.0));
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < n; ++j)
            result.R[i][j] = (j >= i) ? R[i][j] : 0.0;

    /* Build Q from Householder reflectors: Q = H1*H2*...*Hk */
    result.Q.assign(m, QVector<double>(m, 0.0));
    for (int i = 0; i < m; ++i) result.Q[i][i] = 1.0;

    for (int k = reflectors.size() - 1; k >= 0; --k) {
        const QVector<double>& v = reflectors[k];
        for (int j = 0; j < m; ++j) {
            double dot = 0.0;
            for (int i = 0; i < v.size(); ++i)
                dot += v[i] * result.Q[k + i][j];
            for (int i = 0; i < v.size(); ++i)
                result.Q[k + i][j] -= 2.0 * v[i] * dot;
        }
    }

    m_stats.totalDecompositions++;
    m_stats.lastRows = m;
    m_stats.lastCols = n;
    m_stats.lastRank = rank;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalDecompositions > 0)
        ? m_timeSum / m_stats.totalDecompositions : 0.0;

    emit decompositionCompleted(m, n, rank);
    return result;
}

QVector<double> QRDecomposition::solve(const QVector<QVector<double>>& A,
                                         const QVector<double>& b)
{
    QRResult qr = decompose(A);
    int m = A.size();
    int n = (m > 0) ? A[0].size() : 0;
    if (m == 0 || n == 0 || qr.rank == 0) return {};

    /* Q^T * b */
    QVector<double> Qtb = matVec(transpose(qr.Q), b);

    /* Back-substitution on R*x_perm = Qtb */
    QVector<double> xPerm(n, 0.0);
    for (int i = qMin(n, qr.rank) - 1; i >= 0; --i) {
        double sum = Qtb[i];
        for (int j = i + 1; j < n; ++j)
            sum -= qr.R[i][j] * xPerm[j];
        if (qAbs(qr.R[i][i]) > 1e-15)
            xPerm[i] = sum / qr.R[i][i];
    }

    /* Undo column permutation */
    QVector<double> x(n, 0.0);
    for (int j = 0; j < n; ++j)
        x[qr.permutation[j]] = xPerm[j];

    return x;
}

int QRDecomposition::computeRank(const QVector<QVector<double>>& A, double tol)
{
    QRDecomposition qr;
    qr.setTolerance(tol);
    QRResult result = qr.decompose(A);
    return result.rank;
}

void QRDecomposition::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
