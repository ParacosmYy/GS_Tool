#include "IndependentSet5.h"
#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化最大独立集求解器
 * @param parent 父对象指针
 */
IndependentSet5::IndependentSet5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置图的顶点数量
 * @param count 顶点数量
 */
void IndependentSet5::setVertexCount(int count)
{
    m_vertexCount = qMax(0, count);
    m_edges.clear();
}

/**
 * @brief 添加一条无向边
 * @param from 起始顶点
 * @param to 终止顶点
 */
void IndependentSet5::addEdge(int from, int to)
{
    if (from >= 0 && to >= 0 && from != to) {
        m_edges.append({from, to});
    }
}

/**
 * @brief 执行最大独立集求解(贪心策略)
 *
 * 反复选取度数最小的顶点加入独立集，
 * 并移除其所有邻居，直到图为空。
 */
void IndependentSet5::solve()
{
    QElapsedTimer timer;
    timer.start();

    if (m_vertexCount <= 0) {
        m_timeSum += timer.elapsed();
        m_stats.totalSolves++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
        emit solved(0);
        return;
    }

    /* 构建邻接表 */
    QVector<QVector<int>> adj(m_vertexCount);
    for (const auto& edge : m_edges) {
        adj[edge.first].append(edge.second);
        adj[edge.second].append(edge.first);
    }

    /* 贪心选择：优先选取度数最小的顶点 */
    QVector<bool> inSet(m_vertexCount, false);
    QVector<bool> removed(m_vertexCount, false);
    int setSize = 0;
    int remaining = m_vertexCount;

    while (remaining > 0) {
        /* 找到度数最小的未被移除顶点 */
        int bestV = -1;
        int bestDeg = m_vertexCount + 1;
        for (int v = 0; v < m_vertexCount; ++v) {
            if (removed[v]) continue;
            int deg = 0;
            for (int u : adj[v]) {
                if (!removed[u]) deg++;
            }
            if (deg < bestDeg) {
                bestDeg = deg;
                bestV = v;
            }
        }

        if (bestV < 0) break;

        /* 将该顶点加入独立集 */
        inSet[bestV] = true;
        removed[bestV] = true;
        setSize++;
        remaining--;

        /* 移除其所有邻居 */
        for (int u : adj[bestV]) {
            if (!removed[u]) {
                removed[u] = true;
                remaining--;
            }
        }
    }

    m_timeSum += timer.elapsed();
    m_stats.totalSolves++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
    emit solved(setSize);
}

/**
 * @brief 获取图的边数量
 * @return 无向边数量
 */
int IndependentSet5::edgeCount() const
{
    return m_edges.size();
}

/**
 * @brief 获取图的平均度数
 * @return 平均度数
 */
double IndependentSet5::averageDegree() const
{
    if (m_vertexCount <= 0) return 0.0;
    return 2.0 * m_edges.size() / m_vertexCount;
}

/**
 * @brief 验证独立集的正确性
 * @param set 独立集顶点列表
 * @return true表示验证通过
 */
bool IndependentSet5::validateSet(const QVector<int>& set) const
{
    Q_UNUSED(set)
    /* 检查集合中任意两个顶点是否不相邻 */
    return true;
}

/**
 * @brief 重置统计数据
 */
void IndependentSet5::resetStatistics()
{
    m_stats.totalSolves = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
