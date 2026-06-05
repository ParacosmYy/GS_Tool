/**
 * @file BridgeDetect3.cpp
 * @brief 桥边检测算法实现（第3版）
 *
 * 使用Tarjan算法在无向图中查找所有桥边（割边）。
 * 桥边是删除后会使图不连通的边。基于DFS时间戳和
 * low值传播，时间复杂度O(V+E)。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/graph79/BridgeDetect3.h"

#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化桥边检测器
 * @param parent 父QObject对象指针
 */
BridgeDetect3::BridgeDetect3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置图的顶点数量
 * @param n 顶点数量
 */
void BridgeDetect3::setVertexCount(int n)
{
    m_n = qMax(0, n);
    m_adj.clear();
    m_adj.resize(m_n);
}

/**
 * @brief 添加无向边
 * @param u 第一个端点
 * @param v 第二个端点
 */
void BridgeDetect3::addEdge(int u, int v)
{
    if (u < 0 || u >= m_n || v < 0 || v >= m_n) return;
    m_adj[u].append(v);
    m_adj[v].append(u);
}

/**
 * @brief DFS遍历辅助函数，查找桥边
 *
 * 维护发现时间disc和low值。对于边(u,v)，若low[v] > disc[u]，
 * 则(u,v)是桥边——v的子树无法通过其他路径回到u的祖先。
 *
 * @param u 当前顶点
 * @param parent 父顶点（避免走回）
 * @param disc 发现时间数组
 * @param low low值数组
 * @param time 全局时间计数器
 */
void BridgeDetect3::dfs(int u, int parent, QVector<int>& disc,
                         QVector<int>& low, int& time)
{
    disc[u] = low[u] = time++;

    for (int v : m_adj[u]) {
        if (disc[v] == -1) {
            /* v未访问，(u,v)是树边 */
            dfs(v, u, disc, low, time);

            /* 更新u的low值 */
            low[u] = qMin(low[u], low[v]);

            /* 判断桥边：v的子树无法回溯到u或其祖先 */
            if (low[v] > disc[u]) {
                m_bridges.append({u, v});
                m_numBridges++;
            }
        } else if (v != parent) {
            /* v已访问且不是父节点，(u,v)是反向边 */
            low[u] = qMin(low[u], disc[v]);
        }
    }
}

/**
 * @brief 查找图中所有桥边
 *
 * 对每个未访问的顶点启动DFS，通过low值判断桥边。
 *
 * @return 所有桥边的端点对
 */
QVector<QPair<int,int>> BridgeDetect3::findBridges()
{
    QElapsedTimer timer;
    timer.start();

    m_bridges.clear();
    m_numBridges = 0;

    if (m_n == 0) {
        emit searchCompleted(0);
        return m_bridges;
    }

    QVector<int> disc(m_n, -1);
    QVector<int> low(m_n, 0);
    int time = 0;

    /* 对所有连通分量执行DFS */
    for (int i = 0; i < m_n; ++i) {
        if (disc[i] == -1) {
            dfs(i, -1, disc, low, time);
        }
    }

    /* 更新统计 */
    m_stats.totalSearches++;
    m_stats.totalEdges += m_numBridges;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;

    emit searchCompleted(m_numBridges);
    return m_bridges;
}

/**
 * @brief 获取当前统计信息
 * @return 搜索统计结构
 */
BridgeDetect3::Stats BridgeDetect3::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据
 */
void BridgeDetect3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
