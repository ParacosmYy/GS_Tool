/**
 * @file Matching2.cpp
 * @brief 一般图匹配引擎实现 — Edmonds花算法/交错路径/花收缩
 */

#include "utils/graph34/Matching2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <queue>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
Matching2::Matching2(QObject* parent)
    : QObject(parent)
    , m_numVertices(0)
    , m_blossomCount(0)
    , m_timeSum(0.0)
{
}

/** @brief 从邻接表构建图 @param adjList 邻接表 @param numVertices 顶点数 */
void Matching2::buildGraph(const QVector<QVector<int>>& adjList, int numVertices)
{
    m_numVertices = numVertices;
    m_adj = adjList;
    m_match.resize(numVertices);
    m_parent.resize(numVertices);
    m_base.resize(numVertices);
    m_inBlossom.resize(numVertices);
    m_visited.resize(numVertices);
    std::fill(m_match.begin(), m_match.end(), -1);
}

/** @brief 从边列表构建图 @param edges 边列表 @param numVertices 顶点数 */
void Matching2::buildFromEdges(const QVector<QPair<int, int>>& edges,
                                int numVertices)
{
    m_numVertices = numVertices;
    m_adj.resize(numVertices);
    for (auto& list : m_adj) list.clear();

    for (const auto& edge : edges) {
        int u = edge.first, v = edge.second;
        if (u >= 0 && u < numVertices && v >= 0 && v < numVertices && u != v) {
            m_adj[u].append(v);
            m_adj[v].append(u);
        }
    }

    m_match.resize(numVertices);
    m_parent.resize(numVertices);
    m_base.resize(numVertices);
    m_inBlossom.resize(numVertices);
    m_visited.resize(numVertices);
    std::fill(m_match.begin(), m_match.end(), -1);
}

/** @brief 计算最大基数匹配 @return 匹配结果 */
Matching2::MatchResult Matching2::maximumMatching()
{
    QElapsedTimer timer;
    timer.start();

    MatchResult result;
    if (m_numVertices == 0) return result;

    m_blossomCount = 0;

    /* 对每个未匹配的顶点尝试增广 */
    for (int v = 0; v < m_numVertices; ++v) {
        if (m_match[v] == -1) {
            if (augment()) {
                /* 增广成功，从头重新扫描未匹配顶点 */
                v = -1;
            }
        }
    }

    /* 收集匹配边 */
    for (int u = 0; u < m_numVertices; ++u) {
        int v = m_match[u];
        if (v != -1 && u < v) {
            result.edges.append({u, v});
        }
    }

    result.cardinality = result.edges.size();
    result.isPerfect = (result.cardinality * 2 == m_numVertices);
    result.blossomCount = m_blossomCount;

    /* 更新统计 */
    double elapsed = timer.elapsed();
    ++m_stats.totalMatches;
    m_stats.totalVerticesProcessed += static_cast<quint64>(m_numVertices);
    int edgeCount = 0;
    for (const auto& list : m_adj) edgeCount += list.size();
    m_stats.totalEdgesProcessed += static_cast<quint64>(edgeCount / 2);
    m_stats.totalBlossomsContracted += static_cast<quint64>(m_blossomCount);
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalMatches);

    emit matchingComplete(result.cardinality, result.blossomCount);
    return result;
}

/** @brief 检查边是否在匹配中 @param u 顶点u @param v 顶点v @return 是否匹配边 */
bool Matching2::isMatched(int u, int v) const
{
    if (u < 0 || u >= m_numVertices || v < 0 || v >= m_numVertices) return false;
    return m_match[u] == v && m_match[v] == u;
}

/** @brief 获取匹配伙伴 @param v 顶点 @return 匹配伙伴(-1=未匹配) */
int Matching2::matchOf(int v) const
{
    if (v < 0 || v >= m_numVertices) return -1;
    return m_match[v];
}

/** @brief 重置统计 */
void Matching2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 最近公共祖先(在交替树中) @param a 顶点a @param b 顶点b @return LCA索引 */
int Matching2::lca(int a, int b)
{
    QVector<bool> used(m_numVertices, false);
    while (true) {
        a = m_base[a];
        used[a] = true;
        if (m_match[a] == -1) break;
        a = m_parent[m_match[a]];
    }
    while (true) {
        b = m_base[b];
        if (used[b]) return b;
        b = m_parent[m_match[b]];
    }
}

/** @brief 花收缩: 沿路径标记花内顶点 @param v 当前顶点 @param ancestor 花的基 */
void Matching2::tracePath(QVector<int>& path, int v, int ancestor)
{
    while (m_base[v] != ancestor) {
        path.append(v);
        int u = m_match[v];
        if (u == -1) break;
        path.append(u);
        v = m_parent[u];
    }
    path.append(ancestor);
}

/** @brief 花收缩 @param u 花的一侧 @param v 花的另一侧 @param lcaVertex 花的基 */
void Matching2::blossomContract(int u, int v, int lcaVertex)
{
    ++m_blossomCount;

    /* 标记花内所有顶点 */
    std::fill(m_inBlossom.begin(), m_inBlossom.end(), false);
    int cur = u;
    while (cur != lcaVertex) {
        m_inBlossom[m_base[cur]] = true;
        if (m_match[cur] != -1) {
            cur = m_parent[m_match[cur]];
        } else {
            break;
        }
    }
    cur = v;
    while (cur != lcaVertex) {
        m_inBlossom[m_base[cur]] = true;
        if (m_match[cur] != -1) {
            cur = m_parent[m_match[cur]];
        } else {
            break;
        }
    }
    m_inBlossom[lcaVertex] = true;

    /* 将花内所有顶点的base统一为lca */
    for (int i = 0; i < m_numVertices; ++i) {
        if (m_inBlossom[m_base[i]]) {
            m_base[i] = lcaVertex;
        }
    }
}

/** @brief BFS寻找增广路径 @param root 根顶点 */
void Matching2::bfs(int root)
{
    std::fill(m_parent.begin(), m_parent.end(), -1);
    for (int i = 0; i < m_numVertices; ++i) m_base[i] = i;
    std::fill(m_inBlossom.begin(), m_inBlossom.end(), false);
    std::fill(m_visited.begin(), m_visited.end(), false);

    std::queue<int> q;
    q.push(root);
    m_visited[root] = true;

    while (!q.empty()) {
        int u = q.front();
        q.pop();

        for (int v : m_adj[u]) {
            if (m_base[u] == m_base[v]) continue; /* 同一花内 */
            if (m_match[u] == v) continue;        /* 匹配边,已在树中 */

            int bu = m_base[u], bv = m_base[v];

            if (m_match[v] == -1 && v != root) {
                /* 找到增广路径: v是未匹配顶点 */
                m_parent[v] = u;
                return;
            }

            if (m_visited[bv]) {
                /* 发现花: u和v都在交替树中,形成奇环 */
                int curLca = lca(u, v);
                blossomContract(u, v, curLca);

                /* 将花内新发现的顶点加入队列 */
                for (int i = 0; i < m_numVertices; ++i) {
                    if (m_inBlossom[m_base[i]] && !m_visited[i]) {
                        m_visited[i] = true;
                        q.push(i);
                    }
                }
            } else if (!m_visited[v]) {
                /* 扩展交替树 */
                m_visited[v] = true;
                m_parent[v] = u;

                if (m_match[v] != -1) {
                    /* v已匹配,将匹配伙伴加入队列 */
                    int w = m_match[v];
                    m_parent[w] = v;
                    m_visited[w] = true;
                    q.push(w);
                }
            }
        }
    }
}

/** @brief 增广操作 @return 是否找到增广路径 */
bool Matching2::augment()
{
    for (int root = 0; root < m_numVertices; ++root) {
        if (m_match[root] != -1) continue;

        bfs(root);

        /* 沿parent回溯执行增广 */
        for (int v = 0; v < m_numVertices; ++v) {
            if (m_match[v] == -1 && m_parent[v] != -1) {
                /* 回溯增广路径 */
                int cur = v;
                while (cur != -1) {
                    int p = m_parent[cur];
                    if (p == -1) break;
                    int pp = m_parent[p];
                    m_match[p] = cur;
                    m_match[cur] = p;
                    cur = pp;
                }
                return true;
            }
        }
    }
    return false;
}
