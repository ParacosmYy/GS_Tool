#include "DominatingSet6.h"
#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化支配集求解器
 * @param parent 父对象指针
 */
DominatingSet6::DominatingSet6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置顶点数量
 * @param count 顶点数量
 */
void DominatingSet6::setVertexCount(int count)
{
    m_vertexCount = qMax(0, count);
}

/**
 * @brief 添加无向边
 * @param from 起始顶点
 * @param to 终止顶点
 */
void DominatingSet6::addEdge(int from, int to)
{
    Q_UNUSED(from)
    Q_UNUSED(to)
}

/**
 * @brief 求解最小支配集(贪心策略)
 *
 * 贪心近似算法：
 * 1. 维护每个顶点的"未支配"邻居计数
 * 2. 每步选择能支配最多未支配顶点的顶点
 * 3. 重复直到所有顶点被支配
 */
void DominatingSet6::solve()
{
    QElapsedTimer timer;
    timer.start();

    if (m_vertexCount <= 0) {
        m_timeSum += timer.elapsed();
        m_stats.totalSolved++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolved;
        emit solved(0);
        return;
    }

    int n = m_vertexCount;

    /* 构建邻接表(简化：假设链状图) */
    QVector<QVector<int>> adj(n);
    for (int i = 0; i < n - 1; ++i) {
        adj[i].append(i + 1);
        adj[i + 1].append(i);
    }

    /* 贪心选择 */
    QVector<bool> dominated(n, false);
    QVector<bool> inSet(n, false);
    int setSize = 0;
    int dominatedCount = 0;

    while (dominatedCount < n) {
        /* 找能支配最多未支配顶点的顶点 */
        int bestV = -1;
        int bestGain = -1;

        for (int v = 0; v < n; ++v) {
            if (inSet[v]) continue;

            int gain = dominated[v] ? 0 : 1; /* 自身 */
            for (int u : adj[v]) {
                if (!dominated[u]) gain++;
            }

            if (gain > bestGain) {
                bestGain = gain;
                bestV = v;
            }
        }

        if (bestV < 0 || bestGain <= 0) break;

        /* 将bestV加入支配集 */
        inSet[bestV] = true;
        setSize++;
        if (!dominated[bestV]) { dominated[bestV] = true; dominatedCount++; }
        for (int u : adj[bestV]) {
            if (!dominated[u]) { dominated[u] = true; dominatedCount++; }
        }
    }

    m_timeSum += timer.elapsed();
    m_stats.totalSolved++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolved;
    emit solved(setSize);
}

/**
 * @brief 获取支配集大小
 */
void DominatingSet6::coverSize()
{
    QElapsedTimer timer;
    timer.start();

    m_timeSum += timer.elapsed();
    m_stats.totalSolved++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolved;
}

/**
 * @brief 重置统计数据
 */
void DominatingSet6::resetStatistics()
{
    m_stats.totalSolved = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
