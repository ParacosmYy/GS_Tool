#include "StronglyConnected3.h"
#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化强连通分量求解器
 * @param parent 父对象指针
 */
StronglyConnected3::StronglyConnected3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置图的顶点数量
 * @param count 顶点数量
 */
void StronglyConnected3::setVertexCount(int count)
{
    m_vertexCount = qMax(0, count);
    m_edges.clear();
}

/**
 * @brief 添加一条有向边
 * @param from 起始顶点
 * @param to 终止顶点
 */
void StronglyConnected3::addEdge(int from, int to)
{
    if (from >= 0 && to >= 0 && from != to) {
        m_edges.append({from, to});
    }
}

/**
 * @brief 执行Kosaraju算法的第一遍DFS，记录完成顺序
 * @param v 当前顶点
 * @param visited 已访问标记数组
 * @param adj 邻接表
 * @param order 完成顺序栈
 */
static void dfsFirst(int v, QVector<bool>& visited,
                      const QVector<QVector<int>>& adj, QVector<int>& order)
{
    visited[v] = true;
    for (int u : adj[v]) {
        if (!visited[u]) dfsFirst(u, visited, adj, order);
    }
    order.append(v);
}

/**
 * @brief 执行Kosaraju算法的第二遍DFS，标记分量
 * @param v 当前顶点
 * @param visited 已访问标记数组
 * @param revAdj 反向图邻接表
 * @param componentId 当前分量编号
 * @param labels 分量标记数组
 */
static void dfsSecond(int v, QVector<bool>& visited,
                       const QVector<QVector<int>>& revAdj,
                       int componentId, QVector<int>& labels)
{
    visited[v] = true;
    labels[v] = componentId;
    for (int u : revAdj[v]) {
        if (!visited[u]) dfsSecond(u, visited, revAdj, componentId, labels);
    }
}

/**
 * @brief 执行强连通分量求解(Kosaraju算法)
 *
 * 1. 对原图执行DFS，按完成时间逆序排列顶点
 * 2. 构建反向图
 * 3. 按逆序对反向图执行DFS，每次遍历发现一个SCC
 */
void StronglyConnected3::solve()
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

    /* 构建正向和反向邻接表 */
    QVector<QVector<int>> adj(m_vertexCount);
    QVector<QVector<int>> revAdj(m_vertexCount);
    for (const auto& edge : m_edges) {
        adj[edge.first].append(edge.second);
        revAdj[edge.second].append(edge.first);
    }

    /* 第一遍DFS：记录完成顺序 */
    QVector<bool> visited(m_vertexCount, false);
    QVector<int> order;
    for (int v = 0; v < m_vertexCount; ++v) {
        if (!visited[v]) dfsFirst(v, visited, adj, order);
    }

    /* 第二遍DFS：按逆序在反向图上遍历 */
    std::fill(visited.begin(), visited.end(), false);
    QVector<int> labels(m_vertexCount, -1);
    int numSCC = 0;

    for (int i = order.size() - 1; i >= 0; --i) {
        int v = order[i];
        if (!visited[v]) {
            dfsSecond(v, visited, revAdj, numSCC, labels);
            numSCC++;
        }
    }

    m_componentCount = numSCC;

    m_timeSum += timer.elapsed();
    m_stats.totalSolves++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
    emit solved(m_componentCount);
}

/**
 * @brief 获取强连通分量数
 * @return 分量数量
 */
int StronglyConnected3::componentCount() const
{
    return m_componentCount;
}

/**
 * @brief 重置统计数据
 */
void StronglyConnected3::resetStatistics()
{
    m_stats.totalSolves = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
