/**
 * @file SylvesterSolver.cpp
 * @brief Sylvester方程求解实现 — Bartels-Stewart
 */

#include "utils/sylvester/SylvesterSolver.h"

#include <QElapsedTimer>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
SylvesterSolver::SylvesterSolver(QObject* parent)
    : QObject(parent)
{
}

/** @brief 求解Sylvester方程 AX + XB = C */
QVector<QVector<double>> SylvesterSolver::solve(
    const QVector<QVector<double>>& A,
    const QVector<QVector<double>>& B,
    const QVector<QVector<double>>& C)
{
    QElapsedTimer timer;
    timer.start();

    int m = A.size();
    int n = B.size();
    if (m == 0 || n == 0 || C.size() != m) return {};

    /* 将A和B化为上三角(简化Bartels-Stewart) */
    QVector<QVector<double>> U = A;
    QVector<QVector<double>> V = B;
    QVector<QVector<double>> Y = C;

    /* 对A做简化: 逐列消元(上三角化) */
    for (int k = 0; k < m - 1; ++k) {
        for (int i = m - 1; i > k; --i) {
            if (std::abs(U[i][k]) < 1e-15) continue;

            double a = U[i - 1][k];
            double b = U[i][k];
            double r = std::sqrt(a * a + b * b);
            if (r < 1e-300) continue;

            double c = a / r, s = -b / r;

            /* 旋转U的行 */
            for (int j = k; j < m; ++j) {
                double t1 = c * U[i - 1][j] - s * U[i][j];
                double t2 = s * U[i - 1][j] + c * U[i][j];
                U[i - 1][j] = t1;
                U[i][j] = t2;
            }
            /* 旋转Y的行 */
            for (int j = 0; j < n; ++j) {
                double t1 = c * Y[i - 1][j] - s * Y[i][j];
                double t2 = s * Y[i - 1][j] + c * Y[i][j];
                Y[i - 1][j] = t1;
                Y[i][j] = t2;
            }
        }
    }

    /* 对B做简化: 上三角化 */
    for (int k = 0; k < n - 1; ++k) {
        for (int i = n - 1; i > k; --i) {
            if (std::abs(V[i][k]) < 1e-15) continue;

            double a = V[i - 1][k];
            double b = V[i][k];
            double r = std::sqrt(a * a + b * b);
            if (r < 1e-300) continue;

            double c = a / r, s = -b / r;

            for (int j = k; j < n; ++j) {
                double t1 = c * V[i - 1][j] - s * V[i][j];
                double t2 = s * V[i - 1][j] + c * V[i][j];
                V[i - 1][j] = t1;
                V[i][j] = t2;
            }
            /* 旋转Y的列 */
            for (int j = 0; j < m; ++j) {
                double t1 = c * Y[j][i - 1] - s * Y[j][i];
                double t2 = s * Y[j][i - 1] + c * Y[j][i];
                Y[j][i - 1] = t1;
                Y[j][i] = t2;
            }
        }
    }

    /* 回代求解上三角Sylvester方程 */
    QVector<QVector<double>> X(m, QVector<double>(n, 0.0));

    for (int j = n - 1; j >= 0; --j) {
        for (int i = m - 1; i >= 0; --i) {
            double sum = Y[i][j];
            for (int k = i + 1; k < m; ++k)
                sum -= U[i][k] * X[k][j];
            for (int l = j + 1; l < n; ++l)
                sum -= X[i][l] * V[l][j];

            double denom = U[i][i] + V[j][j];
            if (std::abs(denom) > 1e-300)
                X[i][j] = sum / denom;
        }
    }

    m_stats.totalSolves++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(m);
    return X;
}

/** @brief 重置统计 */
void SylvesterSolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
