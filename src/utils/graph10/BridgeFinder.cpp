/**
 * @file BridgeFinder.cpp
 * @brief 无向图桥与割点检测器实现 — Tarjan DFS算法
 * @author Serial Tool Team
 * @date 2026-06-05
 */

#include "utils/graph10/BridgeFinder.h"

#include <QElapsedTimer>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
BridgeFinder::BridgeFinder(QObject *parent)
    : QObject(parent)
{
}

/** @brief 设置无向图邻接表
 *  @param adj 邻接表，adj[i]包含节点i的所有邻居
 *  @param numNodes 节点总数 */
void BridgeFinder::setGraph(const QVector<QVector<int>> &adj, int numNodes)
{
    m_adj = adj;
    m_numNodes = numNodes;
    /* 确保邻接表大小至少为numNodes */
    if (m_adj.size() < numNodes) {
        m_adj.resize(numNodes);
    }
}

/** @brief 查找所有桥(割边)，使用Tarjan算法在O(V+E)内完成
 *  @return 桥列表，每对(u,v)满足u<v */
QVector<QPair<int, int>> BridgeFinder::findBridges()
{
    QElapsedTimer timer;
    timer.start();

    if (m_numNodes <= 0 || m_adj.isEmpty()) {
        return {};
    }

    QVector<int> disc(m_numNodes, 0);       ///< 发现时间戳
    QVector<int> low(m_numNodes, 0);        ///< Low值
    QVector<bool> visited(m_numNodes, false);
    QVector<QPair<int, int>> bridges;
    QVector<int> articulations; /* 不存储，仅为接口兼容 */
    int timerCounter = 0;

    /* 对每个未访问节点执行DFS */
    for (int i = 0; i < m_numNodes; ++i) {
        if (!visited[i]) {
            tarjanDfs(i, -1, timerCounter, disc, low,
                      visited, bridges, articulations);
        }
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalSearches;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalSearches);

    emit searchCompleted(bridges.size(), articulations.size());
    return bridges;
}

/** @brief 查找所有关节点(割点)
 *  @return 关节点编号列表(已去重排序) */
QVector<int> BridgeFinder::findArticulationPoints()
{
    QElapsedTimer timer;
    timer.start();

    if (m_numNodes <= 0 || m_adj.isEmpty()) {
        return {};
    }

    QVector<int> disc(m_numNodes, 0);
    QVector<int> low(m_numNodes, 0);
    QVector<bool> visited(m_numNodes, false);
    QVector<QPair<int, int>> bridges; /* 不存储 */
    QVector<int> articulations;
    int timerCounter = 0;

    for (int i = 0; i < m_numNodes; ++i) {
        if (!visited[i]) {
            tarjanDfs(i, -1, timerCounter, disc, low,
                      visited, bridges, articulations);
        }
    }

    /* 去重 */
    std::sort(articulations.begin(), articulations.end());
    articulations.erase(
        std::unique(articulations.begin(), articulations.end()),
        articulations.end());

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalSearches;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalSearches);

    emit searchCompleted(bridges.size(), articulations.size());
    return articulations;
}

/** @brief 判断图是否双连通(无关节点且连通)
 *  @return true表示双连通 */
bool BridgeFinder::isBiconnected()
{
    if (m_numNodes <= 2) {
        /* 0/1/2个节点的图，需特殊处理 */
        return m_numNodes >= 2;
    }

    QVector<int> arts = findArticulationPoints();
    if (!arts.isEmpty()) {
        return false;
    }

    /* 还需检查连通性: 从节点0出发能访问所有节点 */
    QVector<bool> visited(m_numNodes, false);
    QVector<int> stack;
    stack.push_back(0);
    visited[0] = true;
    int visitCount = 1;

    while (!stack.isEmpty()) {
        int u = stack.back();
        stack.pop_back();
        for (int v : m_adj.value(u)) {
            if (!visited[v]) {
                visited[v] = true;
                ++visitCount;
                stack.push_back(v);
            }
        }
    }

    return visitCount == m_numNodes;
}

/** @brief 重置统计计数器 */
void BridgeFinder::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief Tarjan DFS递归遍历，同时检测桥和割点
 *  @param u 当前节点
 *  @param parent 父节点(-1表示根)
 *  @param timer 全局时间戳引用
 *  @param disc 发现时间数组
 *  @param low Low值数组
 *  @param visited 访问标记数组
 *  @param bridges 桥结果输出
 *  @param articulations 割点结果输出 */
void BridgeFinder::tarjanDfs(int u, int parent, int &timer,
                             QVector<int> &disc, QVector<int> &low,
                             QVector<bool> &visited,
                             QVector<QPair<int, int>> &bridges,
                             QVector<int> &articulations)
{
    visited[u] = true;
    disc[u] = low[u] = ++timer;
    int children = 0;

    const QVector<int> &neighbors = m_adj.value(u);
    for (int v : neighbors) {
        if (!visited[v]) {
            ++children;
            tarjanDfs(v, u, timer, disc, low, visited,
                      bridges, articulations);

            /* 更新当前节点的Low值 */
            low[u] = qMin(low[u], low[v]);

            /* 桥检测: 如果 low[v] > disc[u]，则边(u,v)是桥 */
            if (low[v] > disc[u]) {
                bridges.append({qMin(u, v), qMax(u, v)});
            }

            /* 割点检测(非根节点): 如果 low[v] >= disc[u] */
            if (parent != -1 && low[v] >= disc[u]) {
                articulations.append(u);
            }
        } else if (v != parent) {
            /* 回边: 更新Low值 */
            low[u] = qMin(low[u], disc[v]);
        }
    }

    /* 割点检测(根节点): 如果有2个以上子树 */
    if (parent == -1 && children > 1) {
        articulations.append(u);
    }
}
