/**
 * @file ArnoldiProcess.cpp
 * @brief Arnoldi迭代实现 — 修正Gram-Schmidt正交化
 *
 * 构建Krylov子空间 K_m(A, q_1) = span{q_1, A*q_1, ..., A^{m-1}*q_1} 的
 * 正交基Q和上Hessenberg矩阵H。使用修正Gram-Schmidt(MGS)保证正交性。
 */

#include "utils/arnoldi/ArnoldiProcess.h"

#include <QElapsedTimer>

#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>

ArnoldiProcess::ArnoldiProcess(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

QPair<QVector<QVector<double>>, QVector<QVector<double>>>
ArnoldiProcess::build(std::function<QVector<double>(const QVector<double>&)> matvec,
                      int n, int numVectors, int maxIter)
{
    QElapsedTimer timer;
    timer.start();

    int k = (maxIter > 0) ? qMin(maxIter, numVectors) : numVectors;
    if (n <= 0 || k <= 0) return {{}, {}};

    /* 初始向量q1: 使用随机初始向量并归一化 */
    QVector<double> q1(n);
    std::mt19937 gen(42);
    std::uniform_real_distribution<double> dist(-1.0, 1.0);
    for (int i = 0; i < n; ++i) q1[i] = dist(gen);

    double nrm = 0.0;
    for (double v : q1) nrm += v * v;
    nrm = std::sqrt(nrm);
    for (int i = 0; i < n; ++i) q1[i] /= nrm;

    /* Q: n x k, H: (k+1) x k */
    QVector<QVector<double>> Q(n, QVector<double>(k, 0.0));
    QVector<QVector<double>> H(k + 1, QVector<double>(k, 0.0));

    /* q1放入Q的第0列 */
    for (int i = 0; i < n; ++i) Q[i][0] = q1[i];

    int actualK = k;
    for (int j = 0; j < k; ++j) {
        /* w = A * q_j */
        QVector<double> qj(n);
        for (int i = 0; i < n; ++i) qj[i] = Q[i][j];
        QVector<double> w = matvec(qj);

        /* 修正Gram-Schmidt正交化 */
        for (int i = 0; i <= j; ++i) {
            double dot = 0.0;
            for (int r = 0; r < n; ++r)
                dot += w[r] * Q[r][i];
            H[i][j] = dot;
            for (int r = 0; r < n; ++r)
                w[r] -= dot * Q[r][i];
        }

        /* H[j+1][j] = ||w|| */
        double wNorm = 0.0;
        for (double v : w) wNorm += v * v;
        wNorm = std::sqrt(wNorm);
        H[j + 1][j] = wNorm;

        /* 如果wNorm接近零，Krylov子空间已不变 */
        if (wNorm < 1e-12) {
            actualK = j + 1;
            break;
        }

        /* 归一化w放入Q的下一列 */
        if (j + 1 < k) {
            for (int i = 0; i < n; ++i)
                Q[i][j + 1] = w[i] / wNorm;
        }
    }

    /* 截断Q和H到实际维度 */
    if (actualK < k) {
        QVector<QVector<double>> Qtrim(n, QVector<double>(actualK));
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < actualK; ++j)
                Qtrim[i][j] = Q[i][j];

        QVector<QVector<double>> Htrim(actualK + 1, QVector<double>(actualK));
        for (int i = 0; i <= actualK; ++i)
            for (int j = 0; j < actualK; ++j)
                Htrim[i][j] = H[i][j];

        m_stats.totalBuilds++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalBuilds;

        emit buildCompleted(actualK);
        return {Qtrim, Htrim};
    }

    m_stats.totalBuilds++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalBuilds;

    emit buildCompleted(actualK);
    return {Q, H};
}

void ArnoldiProcess::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
