#include "GraphColoring7.h"
#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化图着色求解器
 * @param parent 父对象指针
 */
GraphColoring7::GraphColoring7(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置图的顶点数量
 * @param count 顶点数量，必须大于0
 */
void GraphColoring7::setVertexCount(int count)
{
    m_vertexCount = qMax(0, count);
    m_edges.clear();
}

/**
 * @brief 添加一条无向边
 * @param from 起始顶点编号
 * @param to 终止顶点编号
 */
void GraphColoring7::addEdge(int from, int to)
{
    if (from >= 0 && to >= 0 && from != to) {
        m_edges.append({from, to});
    }
}

/**
 * @brief 执行图着色求解(贪心策略)
 *
 * 按顶点度数降序排列，依次为每个顶点分配
 * 最小的可用颜色，最终得到色数和着色方案。
 */
void GraphColoring7::solve()
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

    /* 按度数降序排列顶点(Welsh-Powell策略) */
    QVector<int> order(m_vertexCount);
    for (int i = 0; i < m_vertexCount; ++i) order[i] = i;
    std::sort(order.begin(), order.end(), [&](int a, int b) {
        return adj[a].size() > adj[b].size();
    });

    /* 贪心着色 */
    QVector<int> color(m_vertexCount, -1);
    int maxColor = 0;

    for (int v : order) {
        /* 收集相邻顶点已使用的颜色 */
        QVector<bool> used(m_vertexCount + 1, false);
        for (int u : adj[v]) {
            if (color[u] >= 0 && color[u] <= m_vertexCount) {
                used[color[u]] = true;
            }
        }
        /* 分配最小可用颜色 */
        int c = 0;
        while (c < m_vertexCount && used[c]) c++;
        color[v] = c;
        maxColor = qMax(maxColor, c);
    }

    m_chromaticNumber = maxColor + 1;

    m_timeSum += timer.elapsed();
    m_stats.totalSolves++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
    emit solved(m_chromaticNumber);
}

/**
 * @brief 获取色数(最小着色颜色数)
 * @return 色数值
 */
int GraphColoring7::chromaticNumber() const
{
    return m_chromaticNumber;
}

/**
 * @brief 重置统计数据
 */
void GraphColoring7::resetStatistics()
{
    m_stats.totalSolves = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
