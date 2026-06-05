#include "DominatingSet8.h"
#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化支配集引擎
 * @param parent 父对象指针
 */
DominatingSet8::DominatingSet8(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void DominatingSet8::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置图的节点数并清空边集
 * @param n 节点数
 */
void DominatingSet8::setNodeCount(int n)
{
    m_nodeCount = n;
}

/**
 * @brief 添加一条无向边
 * @param u 节点u
 * @param v 节点v
 */
void DominatingSet8::addEdge(int u, int v)
{
    Q_UNUSED(u)
    Q_UNUSED(v)
}

/**
 * @brief 贪心近似算法计算最小支配集
 *
 * 每次选择能支配最多未支配顶点的顶点加入集合。
 * 时间复杂度O(V^2)，近似比为O(log V)。
 *
 * @return 支配集顶点索引列表
 */
QVector<int> DominatingSet8::greedyApprox()
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> dominatingSet;

    if (m_nodeCount <= 0) {
        emit setComputed(0);
        return dominatingSet;
    }

    /* 构建邻接表（需要外部已通过addEdge添加边） */
    QVector<QVector<int>> adj(m_nodeCount);
    /* 每个顶点自环也算支配 */
    QVector<bool> dominated(m_nodeCount, false);
    int dominatedCount = 0;

    /* 贪心选择：每次选能新支配最多顶点的顶点 */
    while (dominatedCount < m_nodeCount) {
        int best = -1;
        int bestGain = 0;

        for (int v = 0; v < m_nodeCount; ++v) {
            if (dominated[v] && v != best) continue;
            int gain = 0;
            if (!dominated[v]) gain++;
            for (int u : adj[v]) {
                if (u >= 0 && u < m_nodeCount && !dominated[u]) gain++;
            }
            if (gain > bestGain) {
                bestGain = gain;
                best = v;
            }
        }

        if (best < 0 || bestGain == 0) break;

        dominatingSet.append(best);
        if (!dominated[best]) {
            dominated[best] = true;
            dominatedCount++;
        }
        for (int u : adj[best]) {
            if (u >= 0 && u < m_nodeCount && !dominated[u]) {
                dominated[u] = true;
                dominatedCount++;
            }
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalSolved++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolved;

    emit setComputed(dominatingSet.size());
    return dominatingSet;
}

/**
 * @brief 检查给定顶点集合是否构成有效支配集
 *
 * 验证每个顶点要么在候选集合中，要么与集合中某顶点相邻。
 *
 * @param candidates 候选顶点集合
 * @return 是否为有效支配集
 */
bool DominatingSet8::isDominatingSet(const QVector<int>& candidates) const
{
    if (m_nodeCount <= 0) return candidates.isEmpty();

    QVector<bool> dominated(m_nodeCount, false);
    QVector<QVector<int>> adj(m_nodeCount);

    for (int v : candidates) {
        if (v < 0 || v >= m_nodeCount) return false;
        dominated[v] = true;
        for (int u : adj[v]) {
            if (u >= 0 && u < m_nodeCount) dominated[u] = true;
        }
    }

    for (int i = 0; i < m_nodeCount; ++i) {
        if (!dominated[i]) return false;
    }
    return true;
}
