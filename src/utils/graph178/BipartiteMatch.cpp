/**
 * @file BipartiteMatch.cpp
 * @brief BipartiteMatch 实现
 *
 * 实现Hopcroft-Karp算法：BFS分层构建距离标签，
 * DFS沿分层图寻找增广路，O(E*sqrt(V))复杂度。
 */

#include "utils/graph178/BipartiteMatch.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

BipartiteMatch::BipartiteMatch(QObject* parent)
    : QObject(parent)
{
}

BipartiteMatch::~BipartiteMatch() = default;

void BipartiteMatch::setGraph(int leftSize, int rightSize,
                              const QVector<QPair<int, int>>& edges)
{
    clearGraph();
    m_leftSize = leftSize;
    m_rightSize = rightSize;
    m_adj.resize(leftSize);
    for (const auto& edge : edges) {
        if (edge.first >= 0 && edge.first < leftSize &&
            edge.second >= 0 && edge.second < rightSize) {
            m_adj[edge.first].append(edge.second);
        }
    }
}

void BipartiteMatch::addEdge(int left, int right)
{
    if (left >= 0 && left < m_leftSize && right >= 0 && right < m_rightSize) {
        if (!m_adj[left].contains(right)) {
            m_adj[left].append(right);
        }
    }
}

void BipartiteMatch::clearGraph()
{
    m_leftSize = 0;
    m_rightSize = 0;
    m_adj.clear();
    m_leftMatch.clear();
    m_rightMatch.clear();
}

bool BipartiteMatch::bfsLayering(QVector<int>& dist)
{
    dist.fill(-1);
    QVector<int> queue;
    queue.reserve(m_leftSize);

    /* Initialize: all free left vertices at distance 0 */
    for (int u = 0; u < m_leftSize; ++u) {
        if (m_leftMatch[u] == -1) {
            dist[u] = 0;
            queue.append(u);
        }
    }

    bool found = false;
    int head = 0;
    while (head < queue.size()) {
        int u = queue[head++];
        for (int v : m_adj[u]) {
            int u2 = m_rightMatch[v];
            if (u2 == -1) {
                /* Free right vertex: we can reach the end */
                found = true;
            } else if (dist[u2] == -1) {
                /* Left vertex u2 is matched to v, extend path */
                dist[u2] = dist[u] + 1;
                queue.append(u2);
            }
        }
    }

    return found;
}

bool BipartiteMatch::dfsAugment(int u, const QVector<int>& dist,
                                QVector<bool>& visited)
{
    for (int v : m_adj[u]) {
        if (visited[v]) continue;
        visited[v] = true;

        if (m_rightMatch[v] == -1) {
            /* Free right vertex: augment */
            m_leftMatch[u] = v;
            m_rightMatch[v] = u;
            return true;
        }

        int u2 = m_rightMatch[v];
        if (dist[u2] == dist[u] + 1) {
            if (dfsAugment(u2, dist, visited)) {
                m_leftMatch[u] = v;
                m_rightMatch[v] = u;
                return true;
            }
        }
    }
    return false;
}

BipartiteMatch::MatchResult BipartiteMatch::findMaxMatching()
{
    QElapsedTimer timer;
    timer.start();

    MatchResult result;
    if (m_leftSize == 0 || m_rightSize == 0) {
        result.matchCount = 0;
        return result;
    }

    m_leftMatch.fill(-1, m_leftSize);
    m_rightMatch.fill(-1, m_rightSize);

    QVector<int> dist(m_leftSize);
    int totalPaths = 0;
    int bfsPhases = 0;

    while (true) {
        if (!bfsLayering(dist)) break;
        bfsPhases++;

        QVector<bool> visited(m_rightSize, false);
        for (int u = 0; u < m_leftSize; ++u) {
            if (m_leftMatch[u] == -1 && dist[u] == 0) {
                visited.fill(false);
                if (dfsAugment(u, dist, visited)) {
                    totalPaths++;
                }
            }
        }
    }

    /* Build result */
    result.leftMatch = m_leftMatch;
    result.rightMatch = m_rightMatch;
    result.matchCount = totalPaths;

    for (int u = 0; u < m_leftSize; ++u) {
        if (m_leftMatch[u] != -1) {
            result.matches.append({u, m_leftMatch[u]});
        }
    }

    m_stats.totalMatches++;
    m_stats.totalBfsPhases += bfsPhases;
    m_stats.totalDfsPaths += totalPaths;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalMatches > 0)
        ? m_timeSum / m_stats.totalMatches : 0.0;

    emit matchingCompleted(totalPaths);
    return result;
}

QVector<int> BipartiteMatch::minVertexCover(const MatchResult& result) const
{
    /* Konig's theorem: min vertex cover from max matching */
    /* Find unmatched left vertices reachable via alternating paths (BFS) */
    QVector<bool> leftVisited(m_leftSize, false);
    QVector<bool> rightVisited(m_rightSize, false);
    QVector<int> queue;

    /* Start from unmatched left vertices */
    for (int u = 0; u < m_leftSize; ++u) {
        if (result.leftMatch[u] == -1) {
            leftVisited[u] = true;
            queue.append(u);
        }
    }

    /* BFS alternating paths: free-left -> right(matched) -> left(matched) */
    int head = 0;
    while (head < queue.size()) {
        int u = queue[head++];
        /* Non-matching edges: u -> v where match[u] != v */
        for (int v : m_adj[u]) {
            if (result.leftMatch[u] == v) continue;
            if (rightVisited[v]) continue;
            rightVisited[v] = true;
            /* Follow matching edge back */
            int u2 = result.rightMatch[v];
            if (u2 != -1 && !leftVisited[u2]) {
                leftVisited[u2] = true;
                queue.append(u2);
            }
        }
    }

    /* Min vertex cover = (L \ Z_L) U (R ∩ Z_R) */
    QVector<int> cover;
    for (int u = 0; u < m_leftSize; ++u) {
        if (!leftVisited[u]) cover.append(u); /* Left not in Z */
    }
    for (int v = 0; v < m_rightSize; ++v) {
        if (rightVisited[v]) cover.append(m_leftSize + v); /* Right in Z */
    }

    return cover;
}

void BipartiteMatch::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
