/**
 * @file SVD10.cpp
 * @brief SVD10 实现
 *
 * 实现奇异值分解：单侧Jacobi旋转与扫描收敛的高瘦矩阵奇异三元组计算。
 */

#include "utils/matrix290/SVD10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SVD10::SVD10(QObject *parent)
    : QObject(parent) {}

SVD10::~SVD10() = default;

/* ---- Configuration ---- */

void SVD10::setMaxSweeps(int sweeps) { m_maxSweeps = qBound(1, sweeps, 10000); }
void SVD10::setTolerance(double tol) { m_tol = qBound(1e-15, tol, 1.0); }

/* ---- Column dot product ---- */

double SVD10::colDot(const QVector<QVector<double>>& A, int c1, int c2) const
{
    double dot = 0.0;
    for (int i = 0; i < A.size(); ++i)
        dot += A[i][c1] * A[i][c2];
    return dot;
}

/* ---- Column norm ---- */

double SVD10::colNorm(const QVector<QVector<double>>& A, int c) const
{
    return qSqrt(colDot(A, c, c));
}

/* ---- Apply Jacobi rotation to columns p, q ---- */

void SVD10::applyJacobi(QVector<QVector<double>>& A,
                          QVector<QVector<double>>& V,
                          int p, int q)
{
    double app = colDot(A, p, p);
    double aqq = colDot(A, q, q);
    double apq = colDot(A, p, q);

    if (qAbs(apq) < m_tol * qSqrt(app * aqq)) return;

    double tau = (aqq - app) / (2.0 * apq);
    double t = (tau >= 0) ?  1.0 / (tau + qSqrt(1.0 + tau * tau))
                          : -1.0 / (-tau + qSqrt(1.0 + tau * tau));
    double c = 1.0 / qSqrt(1.0 + t * t);
    double s = t * c;

    int m = A.size();
    int n = V.size();

    // Rotate columns p and q of A
    for (int i = 0; i < m; ++i) {
        double aip = A[i][p];
        double aiq = A[i][q];
        A[i][p] = c * aip - s * aiq;
        A[i][q] = s * aip + c * aiq;
    }

    // Rotate columns p and q of V
    for (int i = 0; i < n; ++i) {
        double vip = V[i][p];
        double viq = V[i][q];
        V[i][p] = c * vip - s * viq;
        V[i][q] = s * vip + c * viq;
    }
}

/* ---- One Jacobi sweep ---- */

bool SVD10::jacobiSweep(QVector<QVector<double>>& A,
                          QVector<QVector<double>>& V)
{
    int n = V.size();
    double maxOff = 0.0;

    for (int p = 0; p < n; ++p) {
        for (int q = p + 1; q < n; ++q) {
            double apq = colDot(A, p, q);
            maxOff = qMax(maxOff, qAbs(apq));
            applyJacobi(A, V, p, q);
        }
    }

    double diagSum = 0.0;
    for (int p = 0; p < n; ++p)
        diagSum += colDot(A, p, p);

    return (maxOff < m_tol * qMax(diagSum, 1e-30));
}

/* ---- Extract U, sigma from post-Jacobi A ---- */

void SVD10::extractU(QVector<QVector<double>>& A,
                      QVector<double>& sigma,
                      QVector<QVector<double>>& U)
{
    int m = A.size();
    int n = A[0].size();
    int k = qMin(m, n);

    sigma.resize(k);
    U.resize(m, QVector<double>(k, 0.0));

    for (int j = 0; j < k; ++j) {
        double norm = colNorm(A, j);
        sigma[j] = norm;

        if (norm > 1e-30) {
            for (int i = 0; i < m; ++i)
                U[i][j] = A[i][j] / norm;
        }
    }
}

/* ---- Identity matrix ---- */

QVector<QVector<double>> SVD10::identityMatrix(int n) const
{
    QVector<QVector<double>> I(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) I[i][i] = 1.0;
    return I;
}

/* ---- Transpose ---- */

QVector<QVector<double>> SVD10::transpose(const QVector<QVector<double>>& M) const
{
    if (M.isEmpty()) return {};
    int r = M.size(), c = M[0].size();
    QVector<QVector<double>> T(c, QVector<double>(r));
    for (int i = 0; i < r; ++i)
        for (int j = 0; j < c; ++j)
            T[j][i] = M[i][j];
    return T;
}

/* ---- Main compute ---- */

SVD10::SVDResult SVD10::compute(const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    SVDResult result;
    if (matrix.isEmpty() || matrix[0].isEmpty()) return result;

    int m = matrix.size();
    int n = matrix[0].size();

    // Work on A = copy of matrix
    QVector<QVector<double>> A(m, QVector<double>(n));
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < n; ++j)
            A[i][j] = matrix[i][j];

    QVector<QVector<double>> V = identityMatrix(n);

    // One-sided Jacobi iterations
    int sweeps = 0;
    for (sweeps = 0; sweeps < m_maxSweeps; ++sweeps) {
        if (jacobiSweep(A, V)) {
            result.converged = true;
            break;
        }
    }
    result.sweeps = sweeps + 1;

    // Extract singular triplets
    QVector<double> sigma;
    QVector<QVector<double>> U;
    extractU(A, sigma, U);

    // Sort singular values in descending order
    int k = sigma.size();
    QVector<int> order(k);
    for (int i = 0; i < k; ++i) order[i] = i;
    std::sort(order.begin(), order.end(),
              [&sigma](int a, int b) { return sigma[a] > sigma[b]; });

    QVector<double> sortedSigma(k);
    QVector<QVector<double>> sortedU(m, QVector<double>(k));
    QVector<QVector<double>> sortedV(n, QVector<double>(k));

    for (int i = 0; i < k; ++i) {
        int idx = order[i];
        sortedSigma[i] = sigma[idx];
        for (int r = 0; r < m; ++r) sortedU[r][i] = U[r][idx];
        for (int r = 0; r < n; ++r) sortedV[r][i] = V[r][idx];
    }

    result.singularValues = sortedSigma;
    result.U = sortedU;
    result.V = sortedV;
    result.rank = estimateRank(sortedSigma);
    result.conditionNumber = (sortedSigma[0] > 0 && sortedSigma[k - 1] > 0)
                             ? sortedSigma[0] / sortedSigma[k - 1] : 1e30;

    double elapsed = timer.elapsed();
    m_stats.rows = m;
    m_stats.cols = n;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit svdDone(m, n, result.rank, elapsed);

    return result;
}

/* ---- Truncated SVD ---- */

SVD10::SVDResult SVD10::computeTruncated(const QVector<QVector<double>>& matrix, int k)
{
    SVDResult full = compute(matrix);
    if (k >= full.singularValues.size()) return full;

    int m = matrix.size();
    int n = matrix[0].size();

    SVDResult trunc;
    trunc.singularValues = full.singularValues.mid(0, k);
    trunc.U.resize(m, QVector<double>(k));
    trunc.V.resize(n, QVector<double>(k));

    for (int i = 0; i < m; ++i)
        for (int j = 0; j < k; ++j)
            trunc.U[i][j] = full.U[i][j];
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < k; ++j)
            trunc.V[i][j] = full.V[i][j];

    trunc.rank = estimateRank(trunc.singularValues);
    trunc.sweeps = full.sweeps;
    trunc.converged = full.converged;
    trunc.conditionNumber = (trunc.singularValues[0] > 0 && trunc.singularValues[k - 1] > 0)
                             ? trunc.singularValues[0] / trunc.singularValues[k - 1] : 1e30;
    return trunc;
}

/* ---- Estimate rank ---- */

int SVD10::estimateRank(const QVector<double>& sv, double threshold) const
{
    if (sv.isEmpty()) return 0;
    double maxSv = sv[0];
    if (maxSv <= 0) return 0;
    int rank = 0;
    for (double s : sv) {
        if (s > threshold * maxSv) rank++;
    }
    return rank;
}

/* ---- Reset ---- */

void SVD10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
