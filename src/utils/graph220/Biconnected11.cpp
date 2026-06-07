/**
 * @file Biconnected11.cpp
 * @brief Biconnected11 实现
 *
 * 实现双连通分量分解：Hopcroft-Tarjan DFS、割点识别、桥边检测。
 */

#include "utils/graph220/Biconnected11.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Biconnected11::Biconnected11(QObject *parent) : QObject(parent) {}
Biconnected11::~Biconnected11() = default;

/* ---- Build graph ---- */

void Biconnected11::buildGraph(int n, const QVector<QPair<int, int>>& edges)
{
    m_n = n;
    m_adj.assign(n, QVector<int>());
    for (const auto& [u, v] : edges) {
        if (u >= 0 && u < n && v >= 0 && v < n) {
            m_adj[u].append(v);
            m_adj[v].append(u);
        }
    }
    m_stats.numVertices = n;
    m_stats.numEdges = edges.size();
}

/* ---- Articulation point DFS ---- */

void Biconnected11::articulationDFS(int u, int parent, QVector<int>& disc,
                                      QVector<int>& low, QVector<bool>& visited,
                                      QVector<bool>& isArt, QVector<int>& parentArr) const
{
    visited[u] = true;
    disc[u] = low[u] = m_timer++;
    parentArr[u] = parent;
    int children = 0;

    for (int v : m_adj[u]) {
        if (!visited[v]) {
            children++;
            articulationDFS(v, u, disc, low, visited, isArt, parentArr);
            low[u] = qMin(low[u], low[v]);

            // u is articulation if: root with 2+ children, or non-root with low[v] >= disc[u]
            if (parent == -1 && children > 1)
                isArt[u] = true;
            if (parent != -1 && low[v] >= disc[u])
                isArt[u] = true;
        } else if (v != parent) {
            low[u] = qMin(low[u], disc[v]);
        }
    }
}

/* ---- Find articulation points ---- */

QVector<int> Biconnected11::findArticulationPoints()
{
    QElapsedTimer timer;
    timer.start();
    QVector<int> disc(m_n, 0), low(m_n, 0), parentArr(m_n, -1);
    QVector<bool> visited(m_n, false), isArt(m_n, false);
    m_timer = 0;

    for (int i = 0; i < m_n; ++i) {
        if (!visited[i])
            articulationDFS(i, -1, disc, low, visited, isArt, parentArr);
    }

    QVector<int> result;
    for (int i = 0; i < m_n; ++i)
        if (isArt[i]) result.append(i);

    m_stats.numArticulations = result.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    return result;
}

/* ---- BCC DFS with edge stack ---- */

void Biconnected11::bccDFS(int u, int parent, QVector<int>& disc, QVector<int>& low,
                             QVector<bool>& visited, QVector<QPair<int, int>>& edgeStack,
                             QVector<QVector<int>>& components) const
{
    visited[u] = true;
    disc[u] = low[u] = m_timer++;

    for (int v : m_adj[u]) {
        if (!visited[v]) {
            edgeStack.append({u, v});
            bccDFS(v, u, disc, low, visited, edgeStack, components);
            low[u] = qMin(low[u], low[v]);

            // If u is articulation point, pop edge stack to form a BCC
            if ((parent == -1 && m_adj[u].size() > 1) ||
                (parent != -1 && low[v] >= disc[u])) {
                QVector<int> comp;
                QPair<int, int> edge;
                do {
                    edge = edgeStack.last();
                    edgeStack.removeLast();
                    if (!comp.contains(edge.first)) comp.append(edge.first);
                    if (!comp.contains(edge.second)) comp.append(edge.second);
                } while (edge != QPair<int, int>{u, v} && !edgeStack.isEmpty());
                components.append(comp);
            }
        } else if (v != parent && disc[v] < disc[u]) {
            edgeStack.append({u, v});
            low[u] = qMin(low[u], disc[v]);
        }
    }
}

/* ---- Find biconnected components ---- */

QVector<QVector<int>> Biconnected11::findBiconnectedComponents()
{
    QElapsedTimer timer;
    timer.start();
    QVector<int> disc(m_n, 0), low(m_n, 0);
    QVector<bool> visited(m_n, false);
    QVector<QPair<int, int>> edgeStack;
    QVector<QVector<int>> components;
    m_timer = 0;

    for (int i = 0; i < m_n; ++i) {
        if (!visited[i])
            bccDFS(i, -1, disc, low, visited, edgeStack, components);
    }

    m_stats.numComponents = components.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit decompositionCompleted(components.size(), m_stats.numArticulations, timer.elapsed());
    return components;
}

/* ---- Bridge DFS ---- */

void Biconnected11::bridgeDFS(int u, int parent, QVector<int>& disc, QVector<int>& low,
                                QVector<bool>& visited, QVector<QPair<int, int>>& bridges) const
{
    visited[u] = true;
    disc[u] = low[u] = m_timer++;

    for (int v : m_adj[u]) {
        if (!visited[v]) {
            bridgeDFS(v, u, disc, low, visited, bridges);
            low[u] = qMin(low[u], low[v]);
            if (low[v] > disc[u])
                bridges.append({u, v});
        } else if (v != parent) {
            low[u] = qMin(low[u], disc[v]);
        }
    }
}

/* ---- Find bridges ---- */

QVector<QPair<int, int>> Biconnected11::findBridges()
{
    QVector<int> disc(m_n, 0), low(m_n, 0);
    QVector<bool> visited(m_n, false);
    QVector<QPair<int, int>> bridges;
    m_timer = 0;

    for (int i = 0; i < m_n; ++i) {
        if (!visited[i])
            bridgeDFS(i, -1, disc, low, visited, bridges);
    }
    return bridges;
}

/* ---- Check articulation ---- */

bool Biconnected11::isArticulation(int vertex) const
{
    QVector<int> artPts = const_cast<Biconnected11*>(this)->findArticulationPoints();
    return artPts.contains(vertex);
}

/* ---- Check biconnected ---- */

bool Biconnected11::isBiconnected()
{
    QVector<int> artPts = findArticulationPoints();
    return artPts.isEmpty() && m_n >= 2;
}

/* ---- Reset ---- */

void Biconnected11::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_adj.clear();
    m_n = 0;
}
