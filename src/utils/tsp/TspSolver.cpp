/**
 * @file TspSolver.cpp
 * @brief TSP 求解器实现 (Nearest Neighbor + 2-opt)
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/tsp/TspSolver.h"

#include <QtGlobal>
#include <algorithm>

/**
 * @brief 构造函数
 * @param parent 父 QObject
 */
TspSolver::TspSolver(QObject *parent)
    : QObject(parent)
{
}

/**
 * @brief 最近邻启发式构造初始路径
 *
 * 从 startCity 出发，每次访问最近的未访问城市。
 * 贪心策略，时间复杂度 O(n^2)。
 *
 * @param distMatrix 距离矩阵
 * @param startCity 起始城市索引
 * @return 路径序列(包含所有城市)
 */
QVector<int> TspSolver::nearestNeighbor(const QVector<QVector<double>> &distMatrix,
                                         int startCity)
{
    const int n = distMatrix.size();
    QVector<int> tour;
    tour.reserve(n);
    QVector<bool> visited(n, false);

    int current = startCity;
    visited[current] = true;
    tour.push_back(current);

    for (int step = 1; step < n; ++step) {
        double bestDist = std::numeric_limits<double>::max();
        int bestNext = -1;

        for (int j = 0; j < n; ++j) {
            if (!visited[j]) {
                double d = distMatrix[current][j];
                if (d < bestDist) {
                    bestDist = d;
                    bestNext = j;
                }
            }
        }

        if (bestNext < 0) break; // 所有城市已访问
        visited[bestNext] = true;
        tour.push_back(bestNext);
        current = bestNext;
    }

    return tour;
}

/**
 * @brief 计算闭合路径总距离
 * @param distMatrix 距离矩阵
 * @param tour 路径序列
 * @return 闭合回路总距离(tour[last] -> tour[0] 包含在内)
 */
double TspSolver::tourDistance(const QVector<QVector<double>> &distMatrix,
                                const QVector<int> &tour)
{
    const int n = tour.size();
    if (n < 2) return 0.0;
    double total = 0.0;
    for (int i = 0; i < n; ++i) {
        int from = tour[i];
        int to = tour[(i + 1) % n];
        total += distMatrix[from][to];
    }
    return total;
}

/**
 * @brief 2-opt 局部搜索改进
 *
 * 反复尝试: 对路径中每对边 (i,i+1) 和 (j,j+1)，
 * 如果反转 [i+1, j] 段能缩短总距离则执行。
 * 直到无法继续改进为止。
 *
 * @param distMatrix 距离矩阵
 * @param tour 初始路径(会被就地修改)
 * @return 改进后的总距离
 */
double TspSolver::twoOptImprove(const QVector<QVector<double>> &distMatrix,
                                 QVector<int> &tour)
{
    const int n = tour.size();
    if (n < 4) return tourDistance(distMatrix, tour);

    bool improved = true;
    while (improved) {
        improved = false;
        for (int i = 0; i < n - 1; ++i) {
            for (int j = i + 2; j < n; ++j) {
                // 当前两段: (i, i+1) 和 (j, j+1%n)
                int ci = tour[i];
                int ci1 = tour[i + 1];
                int cj = tour[j];
                int cj1 = tour[(j + 1) % n];

                // 2-opt 增量: 新距离 - 旧距离
                double delta = (distMatrix[ci][cj] + distMatrix[ci1][cj1])
                             - (distMatrix[ci][ci1] + distMatrix[cj][cj1]);

                if (delta < -1e-10) {
                    // 反转 [i+1, j] 段
                    int left = i + 1;
                    int right = j;
                    while (left < right) {
                        std::swap(tour[left], tour[right]);
                        ++left;
                        --right;
                    }
                    improved = true;
                }
            }
        }
    }

    return tourDistance(distMatrix, tour);
}

/**
 * @brief 求解 TSP
 *
 * 流程:
 * 1. 输入校验(方阵、对角线为 0)
 * 2. 尝试多个起始城市的最近邻构造
 * 3. 对每个初始解执行 2-opt 改进
 * 4. 选择最优结果
 * 5. 更新统计并发射信号
 *
 * @param distMatrix n x n 距离矩阵
 * @return <总距离, 路径序列>
 */
QPair<double, QVector<int>> TspSolver::solve(
    const QVector<QVector<double>> &distMatrix)
{
    m_timer.start();

    const int n = distMatrix.size();
    // 输入校验
    if (n < 2) {
        emit solveCompleted(0, 0.0);
        return {0.0, {}};
    }
    for (int i = 0; i < n; ++i) {
        if (distMatrix[i].size() != n) {
            emit solveCompleted(0, 0.0);
            return {0.0, {}};
        }
    }

    // 多起始点: 选取 min(n, 5) 个起始城市
    int numStarts = std::min(n, 5);
    QVector<int> startPoints;
    if (n <= 5) {
        for (int i = 0; i < n; ++i) startPoints.push_back(i);
    } else {
        // 均匀采样
        for (int i = 0; i < numStarts; ++i) {
            startPoints.push_back(i * n / numStarts);
        }
    }

    double bestDist = std::numeric_limits<double>::max();
    QVector<int> bestTour;

    for (int start : startPoints) {
        // 最近邻构造
        QVector<int> tour = nearestNeighbor(distMatrix, start);
        if (tour.size() != n) continue;

        // 2-opt 改进
        double dist = twoOptImprove(distMatrix, tour);

        if (dist < bestDist) {
            bestDist = dist;
            bestTour = tour;
        }
    }

    // 更新统计
    double elapsed = static_cast<double>(m_timer.elapsed());
    m_stats.totalSolves++;
    m_timeAccum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeAccum / static_cast<double>(m_stats.totalSolves);

    emit solveCompleted(n, bestDist);
    return {bestDist, bestTour};
}

/**
 * @brief 重置统计计数器
 */
void TspSolver::resetStatistics()
{
    m_stats.totalSolves = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeAccum = 0.0;
}
