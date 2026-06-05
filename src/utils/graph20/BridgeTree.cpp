/**
 * @file BridgeTree.cpp
 * @brief 桥树实现 — Tarjan桥边检测 + 二边连通分量分解
 */

#include "utils/graph20/BridgeTree.h"

#include <QElapsedTimer>

#include <algorithm>
#include <queue>
#include <set>
#include <stack>

/** @brief 构造函数 @param numVertices 顶点数 @param parent 父对象 */
BridgeTree::BridgeTree(int numVertices, QObject* parent)
    : QObject(parent)
    , m_n(numVertices)
    , m_componentCount(0)
    , m_built(false)
{
    m_adj.resize(m_n);
}

/** @brief 添加无向边 @param u 顶点u @param v 顶点v */
void BridgeTree::addEdge(int u, int v)
{
    QElapsedTimer timer;
    timer.start();

    if (u < 0 || u >= m_n || v < 0 || v >= m_n) return;

    int edgeId = static_cast<int>(m_isBridgeEdge.size());
    m_adj[u].push_back({u, v, edgeId});
    m_adj[v].push_back({v, u, edgeId});
    m_isBridgeEdge.push_back(false);
    m_built = false;

    m_stats.totalEdgeAdditions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalQueries + m_stats.totalEdgeAdditions + m_stats.totalRebuilds > 0)
        ? m_timeSum / (m_stats.totalQueries + m_stats.totalEdgeAdditions + m_stats.totalRebuilds) : 0.0;
}

/** @brief Tarjan DFS查找桥边 @param u 当前顶点 @param parentEdgeId 父边编号 @param timer 时间戳 */
void BridgeTree::tarjanDfs(int u, int parentEdgeId, int& timer)
{
    m_disc[u] = m_low[u] = timer++;

    for (const auto& edge : m_adj[u]) {
        if (edge.edgeId == parentEdgeId) continue; /* 跳过回边 */

        int v = edge.to;
        if (m_disc[v] == -1) {
            /* 未访问过的顶点 */
            tarjanDfs(v, edge.edgeId, timer);
            m_low[u] = std::min(m_low[u], m_low[v]);

            /* 桥边判定: v的low值大于u的disc值 */
            if (m_low[v] > m_disc[u]) {
                m_isBridgeEdge[edge.edgeId] = true;
            }
        } else {
            /* 已访问过的顶点，更新low值 */
            m_low[u] = std::min(m_low[u], m_disc[v]);
        }
    }
}

/** @brief 分配分量编号 @param u 当前顶点 @param compId 分量编号 */
void BridgeTree::assignComponents(int u, int compId)
{
    m_component[u] = compId;

    for (const auto& edge : m_adj[u]) {
        int v = edge.to;
        if (m_component[v] != -1) continue;

        if (m_isBridgeEdge[edge.edgeId]) {
            /* 桥边: 对端属于新分量 */
            int newCompId = m_componentCount++;
            assignComponents(v, newCompId);
        } else {
            /* 非桥边: 同一分量 */
            assignComponents(v, compId);
        }
    }
}

/** @brief BFS预处理桥树 @param root 根节点 */
void BridgeTree::bfsBridgeTree(int root)
{
    std::queue<int> q;
    std::vector<bool> visited(m_componentCount, false);

    m_depth[root] = 0;
    m_parent[root] = -1;
    m_distFromRoot[root] = 0;
    visited[root] = true;
    q.push(root);

    while (!q.empty()) {
        int u = q.front();
        q.pop();

        for (int v : m_bridgeTree[u]) {
            if (!visited[v]) {
                visited[v] = true;
                m_depth[v] = m_depth[u] + 1;
                m_parent[v] = u;
                /* 桥树中每条边都对应原图的一条桥边 */
                m_distFromRoot[v] = m_distFromRoot[u] + 1;
                q.push(v);
            }
        }
    }
}

/** @brief 执行桥分解 */
void BridgeTree::build()
{
    QElapsedTimer timer;
    timer.start();

    /* 初始化Tarjan数组 */
    m_disc.assign(m_n, -1);
    m_low.assign(m_n, -1);
    m_component.assign(m_n, -1);

    std::fill(m_isBridgeEdge.begin(), m_isBridgeEdge.end(), false);

    /* Step 1: Tarjan DFS查找所有桥边 */
    int dfsTimer = 0;
    for (int i = 0; i < m_n; ++i) {
        if (m_disc[i] == -1) {
            tarjanDfs(i, -1, dfsTimer);
        }
    }

    /* Step 2: 根据桥边分配分量编号 */
    m_componentCount = 0;
    for (int i = 0; i < m_n; ++i) {
        if (m_component[i] == -1) {
            assignComponents(i, m_componentCount++);
        }
    }

    /* Step 3: 构建桥树 */
    m_bridgeTree.assign(m_componentCount, std::vector<int>());
    int bridgeCount = 0;

    for (size_t i = 0; i < m_isBridgeEdge.size(); ++i) {
        if (m_isBridgeEdge[i]) {
            /* 找到桥边的两个端点 */
            for (int u = 0; u < m_n; ++u) {
                for (const auto& edge : m_adj[u]) {
                    if (edge.edgeId == static_cast<int>(i)) {
                        int compU = m_component[u];
                        int compV = m_component[edge.to];
                        if (compU != compV) {
                            /* 在桥树中添加边 */
                            m_bridgeTree[compU].push_back(compV);
                        }
                    }
                }
                break; /* 每条边只处理一次 */
            }
            ++bridgeCount;
        }
    }

    /* 去重桥树邻接表 */
    for (auto& adj : m_bridgeTree) {
        std::sort(adj.begin(), adj.end());
        adj.erase(std::unique(adj.begin(), adj.end()), adj.end());
    }

    /* Step 4: BFS预处理深度和距离 */
    m_depth.assign(m_componentCount, 0);
    m_parent.assign(m_componentCount, -1);
    m_distFromRoot.assign(m_componentCount, 0);

    std::vector<bool> rootVisited(m_componentCount, false);
    for (int i = 0; i < m_componentCount; ++i) {
        if (!rootVisited[i]) {
            bfsBridgeTree(i);
            rootVisited[i] = true;
        }
    }

    m_built = true;
    m_stats.totalComponents = m_componentCount;
    m_stats.totalBridges = bridgeCount;
    m_stats.totalRebuilds++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalQueries + m_stats.totalEdgeAdditions + m_stats.totalRebuilds > 0)
        ? m_timeSum / (m_stats.totalQueries + m_stats.totalEdgeAdditions + m_stats.totalRebuilds) : 0.0;
}

/** @brief 查询边是否为桥 @param u 顶点u @param v 顶点v @return 是否为桥 */
bool BridgeTree::isBridge(int u, int v) const
{
    if (!m_built) return false;
    if (u < 0 || u >= m_n || v < 0 || v >= m_n) return false;

    /* 检查所有连接u-v的边 */
    for (const auto& edge : m_adj[u]) {
        if (edge.to == v && m_isBridgeEdge[edge.edgeId]) return true;
    }
    return false;
}

/** @brief 查询顶点分量 @param v 顶点 @return 分量编号 */
int BridgeTree::componentOf(int v) const
{
    if (!m_built || v < 0 || v >= m_n) return -1;
    return m_component[v];
}

/** @brief 获取所有桥边 @return 桥边列表 */
QVector<std::pair<int, int>> BridgeTree::bridges() const
{
    QVector<std::pair<int, int>> result;
    if (!m_built) return result;

    std::set<std::pair<int, int>> seen;
    for (int u = 0; u < m_n; ++u) {
        for (const auto& edge : m_adj[u]) {
            if (m_isBridgeEdge[edge.edgeId]) {
                int lo = std::min(u, edge.to);
                int hi = std::max(u, edge.to);
                if (seen.insert({lo, hi}).second) {
                    result.append({lo, hi});
                }
            }
        }
    }
    return result;
}

/** @brief 获取分量顶点 @param compId 分量编号 @return 顶点列表 */
QVector<int> BridgeTree::componentVertices(int compId) const
{
    QVector<int> result;
    if (!m_built || compId < 0 || compId >= m_componentCount) return result;

    for (int i = 0; i < m_n; ++i) {
        if (m_component[i] == compId) result.append(i);
    }
    return result;
}

/** @brief 计算路径上的桥数 @param u 起点 @param v 终点 @return 桥的数量 */
int BridgeTree::bridgesOnPath(int u, int v) const
{
    if (!m_built) return -1;
    if (u < 0 || u >= m_n || v < 0 || v >= m_n) return -1;

    int compU = m_component[u];
    int compV = m_component[v];
    if (compU == compV) return 0;
    if (compU < 0 || compU >= m_componentCount || compV < 0 || compV >= m_componentCount) return -1;

    /* 在桥树上通过LCA计算距离 */
    /* 简单实现: 上升较深的节点到同一深度 */
    int dist = 0;
    int a = compU, b = compV;

    while (m_depth[a] > m_depth[b]) {
        a = m_parent[a];
        ++dist;
    }
    while (m_depth[b] > m_depth[a]) {
        b = m_parent[b];
        ++dist;
    }

    /* 同时上升到LCA */
    while (a != b) {
        if (m_parent[a] == -1 || m_parent[b] == -1) return -1;
        a = m_parent[a];
        b = m_parent[b];
        dist += 2;
    }

    return dist;
}

/** @brief 检查是否同一分量 @param u 顶点u @param v 顶点v @return 是否同分量 */
bool BridgeTree::sameComponent(int u, int v) const
{
    if (!m_built) return false;
    if (u < 0 || u >= m_n || v < 0 || v >= m_n) return false;
    return m_component[u] == m_component[v];
}

/** @brief 重置图结构 @param numVertices 新顶点数 */
void BridgeTree::reset(int numVertices)
{
    m_n = numVertices;
    m_adj.assign(m_n, std::vector<Edge>());
    m_disc.clear();
    m_low.clear();
    m_component.assign(m_n, -1);
    m_isBridgeEdge.clear();
    m_bridgeTree.clear();
    m_depth.clear();
    m_parent.clear();
    m_distFromRoot.clear();
    m_componentCount = 0;
    m_built = false;
}

/** @brief 重置统计 */
void BridgeTree::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
