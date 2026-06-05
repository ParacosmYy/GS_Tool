/**
 * @file VandermondeSolver.cpp
 * @brief Vandermonde矩阵求解实现 — Björck-Pereyra算法
 */

#include "utils/vandermonde/VandermondeSolver.h"

#include <QElapsedTimer>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
VandermondeSolver::VandermondeSolver(QObject* parent)
    : QObject(parent)
{
}

/** @brief 求解Vandermonde系统 */
QVector<double> VandermondeSolver::solve(const QVector<double>& nodes,
                                          const QVector<double>& values)
{
    QElapsedTimer timer;
    timer.start();

    int n = nodes.size();
    if (n == 0 || values.size() != n) return {};

    QVector<double> c = values;

    /* Björck-Pereyra: 逐步消元 */
    for (int k = 0; k < n - 1; ++k) {
        for (int i = n - 1; i > k; --i) {
            double denom = nodes[i] - nodes[i - 1];
            if (std::abs(denom) < 1e-300) return {};
            c[i] = (c[i] - c[i - 1]) / denom;
        }
    }

    /* 回代 */
    for (int k = n - 2; k >= 0; --k) {
        for (int i = k; i < n - 1; ++i) {
            c[i] -= c[i + 1] * nodes[k];
        }
    }

    m_stats.totalSolves++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(n);
    return c;
}

/** @brief 重置统计 */
void VandermondeSolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
