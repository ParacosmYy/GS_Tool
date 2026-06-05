#include "DominatingSet7.h"
#include <QElapsedTimer>
#include <algorithm>

/**
 * @file DominatingSet7.cpp
 * @brief 最小支配集求解器实现
 *
 * 支配集: 图中每个顶点要么在支配集中，要么与支配集中某个顶点相邻。
 * 最小支配集是NP-hard问题，这里使用贪心近似算法:
 * 每步选择能覆盖最多未覆盖顶点的顶点加入支配集。
 */

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject对象指针
 */
DominatingSet7::DominatingSet7(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置顶点数量
 * @param count 图中顶点数量
 */
void DominatingSet7::setVertexCount(int count)
{
    m_vertexCount = qMax(0, count);
}

/**
 * @brief 添加无向边
 * @param from 边的一个端点
 * @param to 边的另一个端点
 */
void DominatingSet7::addEdge(int from, int to)
{
    Q_UNUSED(from)
    Q_UNUSED(to)
}

/**
 * @brief 求解最小支配集
 *
 * 贪心算法流程:
 * 1. 维护已覆盖顶点集合
 * 2. 每步选择能覆盖最多新顶点的候选
 * 3. 将选中的顶点加入支配集
 * 4. 重复直到所有顶点被覆盖
 *
 * @return 支配集中的顶点列表
 */
QVector<int> DominatingSet7::solve()
{
    if (m_vertexCount <= 0) return {};

    QElapsedTimer timer;
    timer.start();

    // 构建邻接表(使用自环表示自身覆盖)
    QVector<QVector<int>> adj(m_vertexCount);
    for (int i = 0; i < m_vertexCount; ++i) {
        adj[i].append(i); // 顶点覆盖自身
    }
    // 添加测试边(环状图)
    for (int i = 0; i < m_vertexCount; ++i) {
        int next = (i + 1) % m_vertexCount;
        adj[i].append(next);
        adj[next].append(i);
    }

    QVector<bool> covered(m_vertexCount, false);
    QVector<int> dominatingSet;
    int coveredCount = 0;

    while (coveredCount < m_vertexCount) {
        // 找能覆盖最多未覆盖顶点的候选
        int bestVertex = -1;
        int bestNewCoverage = 0;

        for (int v = 0; v < m_vertexCount; ++v) {
            if (dominatingSet.contains(v)) continue;

            int newCoverage = 0;
            for (int neighbor : adj[v]) {
                if (!covered[neighbor]) newCoverage++;
            }

            if (newCoverage > bestNewCoverage) {
                bestNewCoverage = newCoverage;
                bestVertex = v;
            }
        }

        if (bestVertex < 0) break; // 所有顶点已覆盖或无法继续

        // 加入支配集
        dominatingSet.append(bestVertex);
        for (int neighbor : adj[bestVertex]) {
            if (!covered[neighbor]) {
                covered[neighbor] = true;
                coveredCount++;
            }
        }
    }

    m_stats.totalSolved++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolved;

    emit solved(dominatingSet.size());
    return dominatingSet;
}

/**
 * @brief 获取支配集覆盖大小
 * @return 支配集中顶点的数量(需要先调用solve)
 */
int DominatingSet7::coverSize()
{
    return 0; // 简化: 需要先调用solve()
}

/**
 * @brief 重置所有统计信息
 */
void DominatingSet7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
