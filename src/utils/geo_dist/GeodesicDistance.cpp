/**
 * @file GeodesicDistance.cpp
 * @brief 测地距离计算实现 — Dijkstra最短路径
 */

#include "utils/geo_dist/GeodesicDistance.h"

#include <QElapsedTimer>
#include <algorithm>
#include <queue>
#include <vector>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
GeodesicDistance::GeodesicDistance(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 析构函数 */
GeodesicDistance::~GeodesicDistance() = default;

/**
 * @brief 计算测地距离场
 *
 * 使用优先队列优化的Dijkstra算法。从起点出发，
 * 对8邻域进行松弛操作，直到所有可达点都被访问。
 * 对角线移动代价为sqrt(2)，正交移动代价为1.0，
 * 乘以目标格的代价值。代价<=0的格子视为不可通行障碍。
 */
QVector<QVector<double>> GeodesicDistance::compute(
    const QVector<QVector<double>>& costMap,
    int startX, int startY)
{
    QElapsedTimer timer;
    timer.start();

    int rows = costMap.size();
    int cols = (rows > 0) ? costMap[0].size() : 0;

    /* 初始化距离场为-1(不可达) */
    QVector<QVector<double>> distMap(rows, QVector<double>(cols, -1.0));

    /* 边界检查 */
    if (rows == 0 || cols == 0) {
        double elapsed = static_cast<double>(timer.elapsed());
        m_timeSum += elapsed;
        ++m_stats.totalComputations;
        m_stats.avgProcessingTimeMs = (m_stats.totalComputations > 0)
            ? m_timeSum / static_cast<double>(m_stats.totalComputations) : 0.0;
        emit computationCompleted(cols, rows);
        return distMap;
    }

    if (startX < 0 || startX >= cols || startY < 0 || startY >= rows) {
        double elapsed = static_cast<double>(timer.elapsed());
        m_timeSum += elapsed;
        ++m_stats.totalComputations;
        m_stats.avgProcessingTimeMs = (m_stats.totalComputations > 0)
            ? m_timeSum / static_cast<double>(m_stats.totalComputations) : 0.0;
        emit computationCompleted(cols, rows);
        return distMap;
    }

    /* 起点不可通行 */
    if (costMap[startY][startX] <= 0.0) {
        double elapsed = static_cast<double>(timer.elapsed());
        m_timeSum += elapsed;
        ++m_stats.totalComputations;
        m_stats.avgProcessingTimeMs = (m_stats.totalComputations > 0)
            ? m_timeSum / static_cast<double>(m_stats.totalComputations) : 0.0;
        emit computationCompleted(cols, rows);
        return distMap;
    }

    /* 优先队列: (距离, x, y) */
    using PQEntry = std::tuple<double, int, int>;
    std::priority_queue<PQEntry, std::vector<PQEntry>, std::greater<PQEntry>> pq;

    distMap[startY][startX] = 0.0;
    pq.push({0.0, startX, startY});

    while (!pq.empty()) {
        auto [dist, cx, cy] = pq.top();
        pq.pop();

        /* 跳过过时的条目 */
        if (dist > distMap[cy][cx] + 1e-12) {
            continue;
        }

        /* 遍历8邻域 */
        for (int d = 0; d < 8; ++d) {
            int nx = cx + s_dx[d];
            int ny = cy + s_dy[d];

            /* 边界检查 */
            if (nx < 0 || nx >= cols || ny < 0 || ny >= rows) {
                continue;
            }

            /* 障碍检查 */
            if (costMap[ny][nx] <= 0.0) {
                continue;
            }

            /* 计算移动代价: 基础距离 × 目标格代价 */
            double baseDist = (d < 2 || d > 5) ? s_diagCost : 1.0;
            double moveCost = baseDist * costMap[ny][nx];
            double newDist = dist + moveCost;

            /* 松弛操作 */
            if (distMap[ny][nx] < -0.5 || newDist < distMap[ny][nx] - 1e-12) {
                distMap[ny][nx] = newDist;
                pq.push({newDist, nx, ny});
            }
        }
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalComputations;
    m_stats.avgProcessingTimeMs = (m_stats.totalComputations > 0)
        ? m_timeSum / static_cast<double>(m_stats.totalComputations) : 0.0;

    emit computationCompleted(cols, rows);
    return distMap;
}

/** @brief 重置统计信息 */
void GeodesicDistance::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
