/**
 * @file SvdSolver.cpp
 * @brief SVD求解器实现 — Golub-Kahan双对角化 + 隐式QR位移
 */

#include "utils/svd2/SvdSolver.h"

#include <QElapsedTimer>
#include <cmath>

SvdSolver::SvdSolver(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 向量二范数 */
static double vecNorm(const QVector<double>& v)
{
    double s = 0.0;
    for (double x : v) s += x * x;
    return std::sqrt(s);
}

/**
 * @brief 左Householder: 消去A[k+1:m, k]
 */
static void leftHouseholder(QVector<QVector<double>>& A,
                            QVector<QVector<double>>& U,
                            int k, int m, int n)
{
    int len = m - k;
    if (len <= 1) return;

    QVector<double> x(len);
    for (int i = 0; i < len; ++i) x[i] = A[k + i][k];

    double normX = vecNorm(x);
    if (normX < 1e-15) return;

    double sign = (x[0] >= 0) ? 1.0 : -1.0;
    x[0] += sign * normX;
    double normV = vecNorm(x);
    if (normV < 1e-15) return;
    for (double& v : x) v /= normV;

    for (int j = k; j < n; ++j) {
        double dot = 0.0;
        for (int i = 0; i < len; ++i) dot += x[i] * A[k + i][j];
        for (int i = 0; i < len; ++i) A[k + i][j] -= 2.0 * dot * x[i];
    }
    for (int i = 0; i < m; ++i) {
        double dot = 0.0;
        for (int j = 0; j < len; ++j) dot += U[i][k + j] * x[j];
        for (int j = 0; j < len; ++j) U[i][k + j] -= 2.0 * dot * x[j];
    }
}

/**
 * @brief 右Householder: 消去A[k, k+2:n]
 */
static void rightHouseholder(QVector<QVector<double>>& A,
                             QVector<QVector<double>>& V,
                             int k, int m, int n)
{
    if (k >= n - 2) return;
    int lenR = n - k - 1;

    QVector<double> x(lenR);
    for (int i = 0; i < lenR; ++i) x[i] = A[k][k + 1 + i];

    double normX = vecNorm(x);
    if (normX < 1e-15) return;

    double sign = (x[0] >= 0) ? 1.0 : -1.0;
    x[0] += sign * normX;
    double normV = vecNorm(x);
    if (normV < 1e-15) return;
    for (double& v : x) v /= normV;

    for (int i = k; i < m; ++i) {
        double dot = 0.0;
        for (int j = 0; j < lenR; ++j) dot += A[i][k + 1 + j] * x[j];
        for (int j = 0; j < lenR; ++j) A[i][k + 1 + j] -= 2.0 * dot * x[j];
    }
    for (int i = 0; i < n; ++i) {
        double dot = 0.0;
        for (int j = 0; j < lenR; ++j) dot += V[i][k + 1 + j] * x[j];
        for (int j = 0; j < lenR; ++j) V[i][k + 1 + j] -= 2.0 * dot * x[j];
    }
}

/**
 * @brief Householder双对角化: A = U * B * V^T
 */
static void bidiagonalize(QVector<QVector<double>>& A,
                          QVector<QVector<double>>& U,
                          QVector<QVector<double>>& V)
{
    int m = A.size();
    int n = A[0].size();
    int steps = qMin(m, n);

    U = QVector<QVector<double>>(m, QVector<double>(m, 0.0));
    V = QVector<QVector<double>>(n, QVector<double>(n, 0.0));
    for (int i = 0; i < m; ++i) U[i][i] = 1.0;
    for (int i = 0; i < n; ++i) V[i][i] = 1.0;

    for (int k = 0; k < steps; ++k) {
        leftHouseholder(A, U, k, m, n);
        rightHouseholder(A, V, k, m, n);
    }
}

/**
 * @brief 隐式QR迭代: 对双对角矩阵(d, e)求奇异值
 */
static void qrIteration(QVector<double>& d, QVector<double>& e, int p)
{
    int maxQrIter = 300;
    int qIdx = p - 1;

    while (qIdx > 0) {
        /* 收缩 */
        while (qIdx > 0) {
            double thresh = 1e-14 * (std::abs(d[qIdx - 1]) + std::abs(d[qIdx]));
            if (std::abs(e[qIdx - 1]) <= thresh) { e[qIdx - 1] = 0.0; --qIdx; }
            else break;
        }
        if (qIdx == 0) break;

        /* 找起始点 */
        int lIdx = qIdx - 1;
        while (lIdx > 0) {
            double thresh = 1e-14 * (std::abs(d[lIdx - 1]) + std::abs(d[lIdx]));
            if (std::abs(e[lIdx - 1]) <= thresh) { e[lIdx - 1] = 0.0; break; }
            --lIdx;
        }

        /* Wilkinson位移 */
        double dd = (d[qIdx - 1] - d[qIdx]) * 0.5;
        double ee = e[qIdx - 1];
        double mu = d[qIdx];
        if (std::abs(dd) > 1e-30) {
            double signDd = (dd >= 0) ? 1.0 : -1.0;
            mu -= ee * ee / (dd + signDd * std::sqrt(dd * dd + ee * ee));
        }

        double x = d[lIdx] - mu;
        double z = e[lIdx];
        for (int k = lIdx; k < qIdx; ++k) {
            double r = std::sqrt(x * x + z * z);
            double c = 1.0, s = 0.0;
            if (r > 1e-30) { c = x / r; s = z / r; }
            if (k > lIdx) e[k - 1] = r;

            double dk = d[k], ek = e[k], dk1 = d[k + 1];
            d[k]     = c * c * dk + 2.0 * c * s * ek + s * s * dk1;
            d[k + 1] = s * s * dk - 2.0 * c * s * ek + c * c * dk1;
            e[k]     = c * s * (dk1 - dk) + (c * c - s * s) * ek;

            if (k + 1 < qIdx) {
                x = e[k]; z = s * e[k + 1]; e[k + 1] = c * e[k + 1];
            }
        }
        if (--maxQrIter <= 0) break;
    }
}

QPair<QVector<double>, QVector<QVector<double>>>
SvdSolver::decompose(const QVector<QVector<double>>& matA)
{
    QElapsedTimer timer;
    timer.start();

    int m = matA.size();
    if (m == 0) { emit decompositionCompleted(0); return {{}, {}}; }
    int n = matA[0].size();
    if (n == 0) { emit decompositionCompleted(0); return {{}, {}}; }

    int p = qMin(m, n);

    /* 双对角化 */
    QVector<QVector<double>> B = matA;
    QVector<QVector<double>> U, V;
    bidiagonalize(B, U, V);

    /* 提取双对角元素 */
    QVector<double> d(p, 0.0), e(p, 0.0);
    for (int i = 0; i < p; ++i) {
        d[i] = B[i][i];
        if (i < p - 1 && i + 1 < n) e[i] = B[i][i + 1];
    }

    /* 隐式QR迭代 */
    qrIteration(d, e, p);

    /* 取绝对值并降序排列 */
    for (int i = 0; i < p; ++i) d[i] = std::abs(d[i]);
    for (int i = 0; i < p - 1; ++i) {
        for (int j = i + 1; j < p; ++j) {
            if (d[j] > d[i]) {
                std::swap(d[i], d[j]);
                for (int r = 0; r < n; ++r) std::swap(V[r][i], V[r][j]);
            }
        }
    }

    ++m_stats.totalDecompositions;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    emit decompositionCompleted(p);
    return {d, V};
}

void SvdSolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
