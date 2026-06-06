/**
 * @file GraphMatching2.cpp
 * @brief GraphMatching2 实现
 *
 * 实现Edmonds花算法：BFS增广路搜索、花检测与收缩、匹配翻转。
 */

#include "utils/graph187/GraphMatching2.h"

#include <QElapsedTimer>
#include <QQueue>
#include <algorithm>

/* ---- Construction / Destruction ---- */

GraphMatching2::GraphMatching2(QObject *parent)
    : QObject(parent)
{
}

GraphMatching2::~GraphMatching2() = default;

/* ---- Find LCA of u and v in alternating tree ---- */

int GraphMatching2::findLCA(int u, int v, const QVector<QVector<int>>& adj)
{
    Q_UNUSED(adj);
    int n = m_base.size();
    QVector<bool> inPath(n, false);

    /* Trace path from u to root */
    int cur = u;
    while (true) {
        cur = m_base[cur];
        inPath[cur] = true;
        if (m_parent[cur] == -1) break;
        cur = m_match[cur];
        if (cur == -1) break;
    }

    /* Trace path from v to find intersection */
    cur = v;
    while (true) {
        cur = m_base[cur];
        if (inPath[cur]) return cur;
        if (m_parent[cur] == -1) break;
        cur = m_match[cur];
        if (cur == -1) break;
    }
    return -1;
}

/* ---- Mark blossom vertices along path ---- */

void GraphMatching2::markBlossomPath(int u, int ancestor, const QVector<QVector<int>>& adj)
{
    Q_UNUSED(adj);
    while (m_base[u] != ancestor) {
        m_inBlossom[m_base[u]] = true;
        m_inBlossom[m_base[m_match[u]]] = true;
        m_parent[u] = m_match[u]; /* Ensure proper parent link */
        u = m_parent[m_match[u]];
    }
}

/* ---- Augment along found path ---- */

void GraphMatching2::augmentPath(int end)
{
    int cur = end;
    while (cur != -1) {
        int p = m_parent[cur];
        if (p == -1) break;
        int pp = m_parent[p];
        m_match[cur] = p;
        m_match[p] = cur;
        cur = pp;
    }
}

/* ---- BFS to find augmenting path from start ---- */

bool GraphMatching2::findAugmentingPath(int start, const QVector<QVector<int>>& adj)
{
    int n = adj.size();
    m_parent.fill(-1);
    m_base.resize(n);
    for (int i = 0; i < n; ++i) m_base[i] = i;
    m_inBlossom.fill(false);
    m_visited.fill(false);

    QQueue<int> queue;
    queue.enqueue(start);
    m_visited[start] = true;

    while (!queue.isEmpty()) {
        int u = queue.dequeue();

        for (int v : adj[u]) {
            if (m_base[u] == m_base[v]) continue;
            if (m_match[u] == v) continue;

            if (!m_visited[v]) {
                m_parent[v] = u;
                m_visited[v] = true;

                if (m_match[v] == -1) {
                    /* Found augmenting path */
                    augmentPath(v);
                    return true;
                }

                /* v is matched, extend alternating tree */
                int w = m_match[v];
                m_parent[w] = v;
                m_visited[w] = true;
                queue.enqueue(w);
            } else {
                /* v already visited -> blossom detected */
                int lca = findLCA(u, v, adj);
                if (lca == -1) continue;

                m_inBlossom.fill(false);
                markBlossomPath(u, lca, adj);
                markBlossomPath(v, lca, adj);

                /* Contract blossom */
                for (int i = 0; i < n; ++i) {
                    if (m_inBlossom[m_base[i]]) {
                        m_base[i] = lca;
                        if (!m_visited[i]) {
                            m_visited[i] = true;
                            queue.enqueue(i);
                        }
                    }
                }
            }
        }
    }
    return false;
}

/* ---- Main maximum matching ---- */

QVector<QPair<int, int>> GraphMatching2::maxMatching(const QVector<QVector<int>>& adjList)
{
    QElapsedTimer timer;
    timer.start();

    int n = adjList.size();
    if (n == 0) return {};

    m_match.fill(-1, n);
    int matchSize = 0;

    /* Greedy initial matching */
    for (int u = 0; u < n; ++u) {
        if (m_match[u] != -1) continue;
        for (int v : adjList[u]) {
            if (m_match[v] == -1 && v > u) {
                m_match[u] = v;
                m_match[v] = u;
                matchSize++;
                break;
            }
        }
    }

    /* Edmonds: try to augment from each free vertex */
    for (int u = 0; u < n; ++u) {
        if (m_match[u] != -1) continue;
        m_parent.resize(n);
        if (findAugmentingPath(u, adjList))
            matchSize++;
    }

    /* Build result */
    QVector<QPair<int, int>> result;
    for (int u = 0; u < n; ++u) {
        int v = m_match[u];
        if (v > u) result.append({u, v});
    }

    int edgeCount = 0;
    for (int u = 0; u < n; ++u) edgeCount += adjList[u].size();
    edgeCount /= 2;

    m_stats.totalMatches++;
    m_stats.lastMatchSize = matchSize;
    m_stats.lastVertices = n;
    m_stats.lastEdges = edgeCount;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalMatches;

    emit matchingCompleted(matchSize, n);
    return result;
}

/* ---- Get match vector ---- */

QVector<int> GraphMatching2::matchVector() const
{
    return m_match;
}

/* ---- Statistics ---- */

void GraphMatching2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
