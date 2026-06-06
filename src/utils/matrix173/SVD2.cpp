/**
 * @file SVD2.cpp
 * @brief SVD2 实现
 *
 * 实现全SVD：Householder双对角化、隐式QR零追赶、Golub-Kahan SVD步。
 */

#include "utils/matrix173/SVD2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

SVD2::SVD2(QObject *parent) : QObject(parent) {}
SVD2::~SVD2() = default;

void SVD2::setMaxIterations(int maxIter) { m_maxIter = qMax(10, maxIter); }
void SVD2::setTolerance(double tol) { m_tol = qMax(1e-15, tol); }

QVector<QVector<double>> SVD2::identity(int n)
{
    QVector<QVector<double>> I(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) I[i][i] = 1.0;
    return I;
}

void SVD2::applyGivensLeft(QVector<QVector<double>>& M, int i, int j,
                            double c, double s)
{
    int cols = M.isEmpty() ? 0 : M[0].size();
    for (int k = 0; k < cols; ++k) {
        double ti = c * M[i][k] + s * M[j][k];
        double tj = -s * M[i][k] + c * M[j][k];
        M[i][k] = ti;
        M[j][k] = tj;
    }
}

void SVD2::applyGivensRight(QVector<QVector<double>>& M, int i, int j,
                              double c, double s)
{
    int rows = M.size();
    for (int k = 0; k < rows; ++k) {
        double ti = c * M[k][i] + s * M[k][j];
        double tj = -s * M[k][i] + c * M[k][j];
        M[k][i] = ti;
        M[k][j] = tj;
    }
}

void SVD2::householderBidiagonalize(QVector<QVector<double>>& A,
                                     QVector<QVector<double>>& U,
                                     QVector<QVector<double>>& V)
{
    int m = A.size();
    int n = A.isEmpty() ? 0 : A[0].size();
    U = identity(m);
    V = identity(n);

    for (int k = 0; k < qMin(m, n); ++k) {
        /* Left Householder: zero below diagonal in column k */
        double norm = 0.0;
        for (int i = k; i < m; ++i) norm += A[i][k] * A[i][k];
        norm = qSqrt(norm);
        if (norm > 1e-30) {
            double sign = (A[k][k] >= 0) ? 1.0 : -1.0;
            double alpha = -sign * norm;
            double r = qSqrt(0.5 * (A[k][k] - alpha) * (A[k][k] - alpha + norm));
            if (r < 1e-30) continue;
            double f = (A[k][k] - alpha) / (2.0 * r * r);
            A[k][k] = alpha;

            QVector<double> v(m - k, 0.0);
            v[0] = r;
            for (int i = k + 1; i < m; ++i) {
                v[i - k] = A[i][k] / (2.0 * r);
                A[i][k] = 0.0;
            }

            /* Apply to A columns k+1..n-1 */
            for (int j = k + 1; j < n; ++j) {
                double dot = f * A[k][j] * r;
                for (int i = k + 1; i < m; ++i)
                    dot += v[i - k] * A[i][j];
                A[k][j] -= 2.0 * f * dot * r / (2.0 * r);
                for (int i = k + 1; i < m; ++i)
                    A[i][j] -= 2.0 * v[i - k] * dot;
            }

            /* Update U */
            for (int j = 0; j < m; ++j) {
                double dot = 0.0;
                for (int i = k; i < m; ++i)
                    dot += v[i - k] * U[j][i];
                for (int i = k; i < m; ++i)
                    U[j][i] -= 2.0 * v[i - k] * dot;
            }
        }

        /* Right Householder: zero right of superdiagonal in row k */
        if (k < n - 2) {
            double norm2 = 0.0;
            for (int j = k + 1; j < n; ++j) norm2 += A[k][j] * A[k][j];
            norm2 = qSqrt(norm2);
            if (norm2 > 1e-30) {
                double sign = (A[k][k + 1] >= 0) ? 1.0 : -1.0;
                double alpha = -sign * norm2;
                double r = qSqrt(0.5 * (A[k][k + 1] - alpha) * (A[k][k + 1] - alpha + norm2));
                if (r < 1e-30) continue;
                double f = (A[k][k + 1] - alpha) / (2.0 * r * r);
                A[k][k + 1] = alpha;

                QVector<double> v(n - k - 1, 0.0);
                v[0] = r;
                for (int j = k + 2; j < n; ++j) {
                    v[j - k - 1] = A[k][j] / (2.0 * r);
                    A[k][j] = 0.0;
                }

                for (int i = k + 1; i < m; ++i) {
                    double dot = f * A[i][k + 1] * r;
                    for (int j = k + 2; j < n; ++j)
                        dot += v[j - k - 1] * A[i][j];
                    A[i][k + 1] -= 2.0 * f * dot * r / (2.0 * r);
                    for (int j = k + 2; j < n; ++j)
                        A[i][j] -= 2.0 * v[j - k - 1] * dot;
                }

                /* Update V */
                for (int i = 0; i < n; ++i) {
                    double dot = 0.0;
                    for (int j = k + 1; j < n; ++j)
                        dot += v[j - k - 1] * V[i][j];
                    for (int j = k + 1; j < n; ++j)
                        V[i][j] -= 2.0 * v[j - k - 1] * dot;
                }
            }
        }
    }
}

void SVD2::golubKahanStep(QVector<double>& diag, QVector<double>& superDiag,
                           QVector<QVector<double>>& U,
                           QVector<QVector<double>>& V, int lo, int hi)
{
    /* Compute Wilkinson shift */
    double d = (diag[hi - 1] * diag[hi - 1] + superDiag[hi - 1] * superDiag[hi - 1]);
    double e = diag[hi] * diag[hi] + ((hi + 1 < diag.size()) ? superDiag[hi] * superDiag[hi] : 0.0);
    double f = (diag[hi] != 0.0)
        ? superDiag[hi - 1] * (diag[hi - 1] + diag[hi])
        : superDiag[hi - 1] * diag[hi - 1];

    double dm = 0.5 * (d - e);
    double sq = qSqrt(dm * dm + f * f);
    double mu = e + dm - ((dm >= 0) ? sq : -sq);

    /* Implicit QR step with shift */
    double y = diag[lo] * diag[lo] - mu;
    double z = diag[lo] * superDiag[lo];

    for (int k = lo; k < hi; ++k) {
        /* Givens rotation to zero z */
        double r = qSqrt(y * y + z * z);
        double c = (qAbs(r) < 1e-30) ? 1.0 : y / r;
        double s = (qAbs(r) < 1e-30) ? 0.0 : z / r;

        if (k > lo) superDiag[k - 1] = r;

        double t1 = c * diag[k] + s * superDiag[k];
        double t2 = -s * diag[k] + c * superDiag[k];
        double t3 = s * diag[k + 1];
        diag[k] = c * t1 + s * t2;
        superDiag[k] = c * t2 - s * t1;
        diag[k + 1] = c * diag[k + 1];

        applyGivensRight(V, k, k + 1, c, s);
        applyGivensLeft(U, k, k + 1, c, s);

        if (k < hi - 1) {
            y = superDiag[k];
            z = t3;
            superDiag[k] = y;
        }
    }
}

SVD2::Result SVD2::decompose(const QVector<QVector<double>>& A)
{
    QElapsedTimer timer;
    timer.start();

    int m = A.size();
    int n = A.isEmpty() ? 0 : A[0].size();
    if (m == 0 || n == 0) return Result{};

    /* Copy A for bidiagonalization */
    QVector<QVector<double>> B = A;
    QVector<QVector<double>> U, V;
    householderBidiagonalize(B, U, V);

    int p = qMin(m, n);
    QVector<double> diag(p), superDiag(p - 1, 0.0);
    for (int i = 0; i < p; ++i) diag[i] = B[i][i];
    for (int i = 0; i < p - 1; ++i) superDiag[i] = B[i][i + 1];

    /* QR iteration */
    int iterations = 0;
    for (int iter = 0; iter < m_maxIter; ++iter) {
        iterations++;
        /* Find active block [lo, hi] */
        int hi = p - 1;
        while (hi > 0 && qAbs(superDiag[hi - 1]) <= m_tol * (qAbs(diag[hi - 1]) + qAbs(diag[hi])))
            hi--;
        if (hi == 0) break;

        int lo = hi - 1;
        while (lo > 0 && qAbs(superDiag[lo - 1]) > m_tol * (qAbs(diag[lo - 1]) + qAbs(diag[lo])))
            lo--;

        golubKahanStep(diag, superDiag, U, V, lo, hi);
    }

    /* Ensure non-negative singular values */
    for (int i = 0; i < p; ++i) {
        if (diag[i] < 0) {
            diag[i] = -diag[i];
            for (int j = 0; j < n; ++j) V[j][i] = -V[j][i];
        }
    }

    /* Sort by descending singular value */
    QVector<int> idx(p);
    for (int i = 0; i < p; ++i) idx[i] = i;
    std::sort(idx.begin(), idx.end(), [&](int a, int b) { return diag[a] > diag[b]; });

    Result result;
    result.singularValues.resize(p);
    result.U = QVector<QVector<double>>(m, QVector<double>(p, 0.0));
    result.V = QVector<QVector<double>>(n, QVector<double>(p, 0.0));

    for (int i = 0; i < p; ++i) {
        result.singularValues[i] = diag[idx[i]];
        for (int j = 0; j < m; ++j) result.U[j][i] = U[j][idx[i]];
        for (int j = 0; j < n; ++j) result.V[j][i] = V[j][idx[i]];
    }

    m_stats.totalDecompositions++;
    m_stats.lastRows = m;
    m_stats.lastCols = n;
    m_stats.lastIterations = iterations;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalDecompositions > 0)
        ? m_timeSum / m_stats.totalDecompositions : 0.0;

    emit decompositionCompleted(m, n);
    return result;
}

double SVD2::conditionNumber(const QVector<QVector<double>>& A)
{
    Result r = decompose(A);
    if (r.singularValues.isEmpty()) return 0.0;
    double maxSv = r.singularValues.first();
    double minSv = r.singularValues.last();
    return (minSv > 1e-30) ? maxSv / minSv : 1e30;
}

int SVD2::rank(const QVector<QVector<double>>& A, double threshold)
{
    Result r = decompose(A);
    int rk = 0;
    for (double sv : r.singularValues)
        if (sv > threshold) rk++;
    return rk;
}

QVector<QVector<double>> SVD2::lowRankApprox(const Result& svd, int r) const
{
    int m = svd.U.size();
    int n = svd.V.size();
    int p = qMin(r, svd.singularValues.size());
    QVector<QVector<double>> result(m, QVector<double>(n, 0.0));

    for (int k = 0; k < p; ++k) {
        double sv = svd.singularValues[k];
        for (int i = 0; i < m; ++i)
            for (int j = 0; j < n; ++j)
                result[i][j] += sv * svd.U[i][k] * svd.V[j][k];
    }
    return result;
}

QVector<QVector<double>> SVD2::pseudoInverse(const Result& svd) const
{
    int m = svd.U.size();
    int n = svd.V.size();
    int p = svd.singularValues.size();

    /* Sigma^+: invert non-zero singular values */
    QVector<double> invSv(p);
    for (int i = 0; i < p; ++i)
        invSv[i] = (qAbs(svd.singularValues[i]) > 1e-12)
            ? 1.0 / svd.singularValues[i] : 0.0;

    /* A^+ = V * Sigma^+ * U^T */
    QVector<QVector<double>> result(n, QVector<double>(m, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j)
            for (int k = 0; k < p; ++k)
                result[i][j] += svd.V[i][k] * invSv[k] * svd.U[j][k];
    return result;
}

void SVD2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
