/**
 * @file SVD3.cpp
 * @brief SVD3 实现
 *
 * 实现SVD分解：Golub-Kahan双对角化、隐式零位移QR迭代。
 */

#include "utils/matrix182/SVD3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SVD3::SVD3(QObject *parent) : QObject(parent) {}
SVD3::~SVD3() = default;

/* ---- Householder: compute reflection vector ---- */

void SVD3::householder(QVector<double>& x, int start,
                         double& beta, QVector<double>& v) const
{
    int n = x.size() - start;
    if (n <= 0) { beta = 0.0; return; }

    double sigma = 0.0;
    for (int i = start + 1; i < x.size(); ++i)
        sigma += x[i] * x[i];

    v = QVector<double>(n, 0.0);
    v[0] = x[start];

    if (sigma < 1e-30) {
        beta = 0.0;
        return;
    }

    double mu = qSqrt(x[start] * x[start] + sigma);
    if (x[start] <= 0)
        v[0] = x[start] - mu;
    else
        v[0] = -sigma / (x[start] + mu);

    beta = 2.0 * v[0] * v[0] / (sigma + v[0] * v[0]);
    for (int i = 1; i < n; ++i)
        v[i] = x[start + i];
    double scale = v[0];
    if (qAbs(scale) > 1e-30)
        for (int i = 0; i < n; ++i) v[i] /= scale;
}

/* ---- Golub-Kahan bidiagonalization ---- */

void SVD3::bidiagonalize(QVector<QVector<double>>& B,
                           QVector<QVector<double>>& U,
                           QVector<QVector<double>>& V)
{
    int m = B.size();
    int n = B[0].size();
    int k = qMin(m, n);

    for (int i = 0; i < k; ++i) {
        /* Left Householder: zero below diagonal in column i */
        QVector<double> col(m - i);
        for (int j = i; j < m; ++j) col[j - i] = B[j][i];

        double beta;
        QVector<double> v;
        householder(col, 0, beta, v);

        if (beta > 1e-30) {
            /* Apply: B[i:m, i:n] -= beta * v * (v^T * B[i:m, i:n]) */
            for (int j = i; j < n; ++j) {
                double dot = 0.0;
                for (int r = 0; r < v.size(); ++r)
                    dot += v[r] * B[i + r][j];
                for (int r = 0; r < v.size(); ++r)
                    B[i + r][j] -= beta * v[r] * dot;
            }
            /* Accumulate U */
            for (int r = 0; r < m; ++r) {
                double dot = 0.0;
                for (int s = 0; s < v.size(); ++s)
                    dot += U[r][i + s] * v[s];
                for (int s = 0; s < v.size(); ++s)
                    U[r][i + s] -= beta * v[s] * dot;
            }
        }

        /* Right Householder: zero right of superdiagonal in row i */
        if (i < n - 2) {
            QVector<double> row(n - i - 1);
            for (int j = i + 1; j < n; ++j) row[j - i - 1] = B[i][j];

            double beta2;
            QVector<double> v2;
            householder(row, 0, beta2, v2);

            if (beta2 > 1e-30) {
                for (int j = i; j < m; ++j) {
                    double dot = 0.0;
                    for (int s = 0; s < v2.size(); ++s)
                        dot += v2[s] * B[j][i + 1 + s];
                    for (int s = 0; s < v2.size(); ++s)
                        B[j][i + 1 + s] -= beta2 * v2[s] * dot;
                }
                /* Accumulate V */
                for (int r = 0; r < n; ++r) {
                    double dot = 0.0;
                    for (int s = 0; s < v2.size(); ++s)
                        dot += V[r][i + 1 + s] * v2[s];
                    for (int s = 0; s < v2.size(); ++s)
                        V[r][i + 1 + s] -= beta2 * v2[s] * dot;
                }
            }
        }
    }
}

/* ---- Apply Givens rotation ---- */

void SVD3::applyGivens(QVector<double>& a, QVector<double>& b,
                         double c, double s, int i, int j) const
{
    double t1 = a[i], t2 = b[j];
    a[i] = c * t1 - s * t2;
    b[j] = s * t1 + c * t2;
}

/* ---- Implicit zero-shift QR step on bidiagonal ---- */

void SVD3::implicitQRStep(QVector<double>& diag,
                            QVector<double>& superdiag,
                            QVector<QVector<double>>& U,
                            QVector<QVector<double>>& V,
                            int lo, int hi)
{
    int n = diag.size();
    /* Compute Wilkinson shift */
    double d = (diag[hi - 1] * diag[hi - 1] - diag[lo] * diag[lo]
                + superdiag[lo] * superdiag[lo]) / (2.0 * superdiag[lo]);
    double mu = diag[hi - 1] * diag[hi - 1]
              + superdiag[hi - 1] * superdiag[hi - 1]
              - d * d / (d < 0 ? -qSqrt(d * d + 1.0) : qSqrt(d * d + 1.0));

    double x = diag[lo] * diag[lo] - mu;
    double z = diag[lo] * superdiag[lo];

    for (int k = lo; k < hi - 1; ++k) {
        double r = qSqrt(x * x + z * z);
        double c = (qAbs(r) > 1e-30) ? x / r : 1.0;
        double s = (qAbs(r) > 1e-30) ? z / r : 0.0;

        if (k > lo) superdiag[k] = r;

        /* Givens from right (affects columns k, k+1) */
        double t1 = diag[k], t2 = superdiag[k];
        diag[k] = c * t1 + s * t2;
        superdiag[k] = -s * t1 + c * t2;
        x = superdiag[k];
        if (k + 1 < n) {
            z = s * diag[k + 1];
            diag[k + 1] = c * diag[k + 1];
        }

        /* Update V */
        for (int i = 0; i < n; ++i) {
            double v1 = V[i][k], v2 = V[i][k + 1];
            V[i][k] = c * v1 + s * v2;
            V[i][k + 1] = -s * v1 + c * v2;
        }

        /* Givens from left */
        r = qSqrt(x * x + z * z);
        c = (qAbs(r) > 1e-30) ? x / r : 1.0;
        s = (qAbs(r) > 1e-30) ? z / r : 0.0;

        t1 = diag[k]; t2 = diag[k + 1];
        diag[k] = c * t1 + s * t2;
        diag[k + 1] = -s * t1 + c * t2;
        superdiag[k] = c * superdiag[k];

        x = superdiag[k];
        if (k + 2 < n) {
            z = s * superdiag[k + 1];
            superdiag[k + 1] = c * superdiag[k + 1];
        }

        /* Update U */
        for (int i = 0; i < m_rows; ++i) {
            double u1 = U[i][k], u2 = U[i][k + 1];
            U[i][k] = c * u1 + s * u2;
            U[i][k + 1] = -s * u1 + c * u2;
        }
    }
}

/* ---- Main decompose ---- */

void SVD3::decompose(const QVector<QVector<double>>& A)
{
    QElapsedTimer timer;
    timer.start();

    m_rows = A.size();
    if (m_rows == 0) return;
    m_cols = A[0].size();
    int m = m_rows, n = m_cols;
    int k = qMin(m, n);

    /* Initialize U = I_m, V = I_n */
    m_U.resize(m);
    for (int i = 0; i < m; ++i) {
        m_U[i].resize(m, 0.0);
        m_U[i][i] = 1.0;
    }
    m_V.resize(n);
    for (int i = 0; i < n; ++i) {
        m_V[i].resize(n, 0.0);
        m_V[i][i] = 1.0;
    }

    /* Copy A to B */
    QVector<QVector<double>> B = A;
    /* Pad B to m x m if needed for bidiagonalization */
    for (int i = 0; i < m; ++i)
        B[i].resize(m, 0.0);

    bidiagonalize(B, m_U, m_V);

    /* Extract bidiagonal */
    QVector<double> diag(k), superdiag(k - 1, 0.0);
    for (int i = 0; i < k; ++i) diag[i] = B[i][i];
    for (int i = 0; i < k - 1; ++i) superdiag[i] = B[i][i + 1];

    /* QR iteration to diagonalize */
    for (int iter = 0; iter < 100 * k; ++iter) {
        /* Find active block */
        int lo = 0, hi = k;
        for (int i = k - 1; i > 0; --i) {
            if (qAbs(superdiag[i - 1]) > 1e-12 *
                (qAbs(diag[i - 1]) + qAbs(diag[i]))) {
                lo = i;
                break;
            }
        }
        if (hi == 0) break;
        implicitQRStep(diag, superdiag, m_U, m_V, lo, hi);
    }

    /* Ensure non-negative singular values */
    m_S = diag;
    for (int i = 0; i < k; ++i) {
        if (m_S[i] < 0) {
            m_S[i] = -m_S[i];
            for (int r = 0; r < m; ++r) m_U[r][i] = -m_U[r][i];
        }
    }

    /* Sort descending */
    for (int i = 0; i < k - 1; ++i)
        for (int j = i + 1; j < k; ++j)
            if (m_S[j] > m_S[i]) {
                std::swap(m_S[i], m_S[j]);
                for (int r = 0; r < m; ++r) std::swap(m_U[r][i], m_U[r][j]);
                for (int r = 0; r < n; ++r) std::swap(m_V[r][i], m_V[r][j]);
            }

    int rank = numericalRank();
    double cond = (m_S[0] > 1e-30 && m_S[k - 1] > 1e-30)
                  ? m_S[0] / m_S[k - 1] : 1e18;

    m_stats.totalDecompositions++;
    m_stats.rows = m;
    m_stats.cols = n;
    m_stats.rank = rank;
    m_stats.conditionNumber = cond;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    emit decompositionCompleted(rank, cond);
}

/* ---- Accessors ---- */

QVector<QVector<double>> SVD3::matrixU() const { return m_U; }
QVector<double> SVD3::singularValues() const { return m_S; }
QVector<QVector<double>> SVD3::matrixV() const { return m_V; }

double SVD3::conditionNumber() const
{
    if (m_S.isEmpty()) return 0.0;
    return (m_S.last() > 1e-30) ? m_S.first() / m_S.last() : 1e18;
}

int SVD3::numericalRank(double threshold) const
{
    int r = 0;
    for (double s : m_S)
        if (s > threshold) r++;
    return r;
}

/* ---- Reset ---- */

void SVD3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
