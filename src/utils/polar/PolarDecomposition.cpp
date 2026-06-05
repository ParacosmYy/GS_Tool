/**
 * @file PolarDecomposition.cpp
 * @brief 极分解实现 — 基于双边Jacobi SVD
 *
 * 流程: SVD(A) = U_svd * Sigma * V_svd^T
 *       => U = U_svd * V_svd^T  (正交部分)
 *       => P = V_svd * Sigma * V_svd^T  (正定对称部分)
 */

#include "utils/polar/PolarDecomposition.h"

#include <QElapsedTimer>

#include <algorithm>
#include <cmath>

PolarDecomposition::PolarDecomposition(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

QPair<QVector<QVector<double>>, QVector<QVector<double>>>
PolarDecomposition::decompose(const QVector<QVector<double>>& A)
{
    QElapsedTimer timer;
    timer.start();

    int n = A.size();
    int m = n > 0 ? A[0].size() : 0;

    /* 构造 B = A^T * A 用于Jacobi旋转求V */
    int p = qMin(m, n);
    QVector<QVector<double>> B(p, QVector<double>(p, 0.0));
    for (int i = 0; i < p; ++i)
        for (int j = 0; j < p; ++j)
            for (int k = 0; k < m; ++k)
                B[i][j] += A[k][i] * A[k][j];

    /* V初始化为单位矩阵 */
    QVector<QVector<double>> V(p, QVector<double>(p, 0.0));
    for (int i = 0; i < p; ++i) V[i][i] = 1.0;

    /* 双边Jacobi旋转对角化 B = V * D * V^T */
    const int maxIter = 100;
    for (int iter = 0; iter < maxIter; ++iter) {
        double offDiag = 0.0;
        for (int i = 0; i < p; ++i)
            for (int j = i + 1; j < p; ++j)
                offDiag += B[i][j] * B[i][j];

        if (std::sqrt(offDiag) < 1e-12) break;

        for (int i = 0; i < p - 1; ++i) {
            for (int j = i + 1; j < p; ++j) {
                if (std::abs(B[i][j]) < 1e-15) continue;

                double aii = B[i][i], ajj = B[j][j], aij = B[i][j];
                double tau = (ajj - aii) / (2.0 * aij);
                double t = (tau >= 0)
                    ? 1.0 / (tau + std::sqrt(1.0 + tau * tau))
                    : -1.0 / (-tau + std::sqrt(1.0 + tau * tau));
                double c = 1.0 / std::sqrt(1.0 + t * t);
                double s = t * c;

                /* 更新B的对角与非对角 */
                B[i][i] = c * c * aii - 2.0 * s * c * aij + s * s * ajj;
                B[j][j] = s * s * aii + 2.0 * s * c * aij + c * c * ajj;
                B[i][j] = B[j][i] = 0.0;

                for (int r = 0; r < p; ++r) {
                    if (r == i || r == j) continue;
                    double bri = B[r][i], brj = B[r][j];
                    B[r][i] = B[i][r] = c * bri - s * brj;
                    B[r][j] = B[j][r] = s * bri + c * brj;
                }

                /* 累积旋转到V */
                for (int r = 0; r < p; ++r) {
                    double vri = V[r][i], vrj = V[r][j];
                    V[r][i] = c * vri - s * vrj;
                    V[r][j] = s * vri + c * vrj;
                }
            }
        }
    }

    /* 提取奇异值 */
    QVector<double> sigma(p);
    for (int i = 0; i < p; ++i)
        sigma[i] = std::sqrt(std::max(0.0, B[i][i]));

    /* 按降序排列 */
    QVector<int> order(p);
    for (int i = 0; i < p; ++i) order[i] = i;
    std::sort(order.begin(), order.end(),
              [&sigma](int a, int b) { return sigma[a] > sigma[b]; });

    QVector<double> sortedSigma(p);
    QVector<QVector<double>> sortedV(p, QVector<double>(p));
    for (int i = 0; i < p; ++i) {
        sortedSigma[i] = sigma[order[i]];
        for (int j = 0; j < p; ++j)
            sortedV[j][i] = V[j][order[i]];
    }

    /* U_svd = A * V * Sigma^{-1} */
    QVector<QVector<double>> Usvd(m, QVector<double>(p, 0.0));
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < p; ++j) {
            double dot = 0.0;
            for (int k = 0; k < n; ++k) dot += A[i][k] * sortedV[k][j];
            Usvd[i][j] = (sortedSigma[j] > 1e-15) ? dot / sortedSigma[j] : 0.0;
        }
    }

    /* 极分解: U = U_svd * V^T, P = V * Sigma * V^T */
    QVector<QVector<double>> U(m, QVector<double>(p, 0.0));
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < p; ++j)
            for (int k = 0; k < p; ++k)
                U[i][j] += Usvd[i][k] * sortedV[j][k];

    QVector<QVector<double>> P(p, QVector<double>(p, 0.0));
    for (int i = 0; i < p; ++i)
        for (int j = 0; j < p; ++j)
            for (int k = 0; k < p; ++k)
                P[i][j] += sortedV[i][k] * sortedSigma[k] * sortedV[j][k];

    m_stats.totalDecompositions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    emit decompositionCompleted(n);
    return {U, P};
}

void PolarDecomposition::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
