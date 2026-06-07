/**
 * @file Biconnected10.cpp
 * @brief Biconnected10 实现
 *
 * 实现双连通分量：桥检测、割点识别、2-边连通分解、桥块树构建。
 */

#include "utils/graph207/Biconnected10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <stack>

/* ---- Construction / Destruction ---- */

Biconnected10::Biconnected10(QObject *parent) : QObject(parent) {}
Biconnected10::~Biconnected10() = default;

/* ---- Graph initialization ---- */

void Biconnected10::initGraph(int n)
{
    m_n = qMax(0, n);
    m_adj.resize(m_n);
    for (auto& list : m_adj) list.clear();

    m_disc.resize(m_n, 0);
    m_low.resize(m_n, 0);
    m_parent.resize(m_n, -1);
    m_visited.resize(m_n, false);

    m_bridges.clear();
    m_articulations.clear();
    m_components.clear();
    m_vertComp.clear();

    m_stats.numVertices = m_n;
}

/* ---- Add undirected edge ---- */

void Biconnected10::addEdge(int u, int v)
{
    if (u < 0 || u >= m_n || v < 0 || v >= m_n || u == v) return;
    m_adj[u].append(v);
    m_adj[v].append(u);
    m_stats.numEdges++;
}

/* ---- DFS for bridges and articulation points ---- */

void Biconnected10::dfs(int u)
{
    m_visited[u] = true;
    m_disc[u] = m_low[u] = ++m_timer;
    int children = 0;

    for (int v : m_adj[u]) {
        if (!m_visited[v]) {
            children++;
            m_parent[v] = u;
            dfs(v);

            m_low[u] = qMin(m_low[u], m_low[v]);

            // Bridge: (u, v) is a bridge if low[v] > disc[u]
            if (m_low[v] > m_disc[u])
                m_bridges.append({u, v});

            // Articulation point
            if ((m_parent[u] == -1 && children > 1) ||
                (m_parent[u] != -1 && m_low[v] >= m_disc[u])) {
                if (!m_articulations.contains(u))
                    m_articulations.append(u);
            }
        } else if (v != m_parent[u]) {
            m_low[u] = qMin(m_low[u], m_disc[v]);
        }
    }
}

/* ---- DFS for BCC extraction using edge stack ---- */

void Biconnected10::dfsBcc(int u, QVector<QPair<int, int>>& edgeStack)
{
    m_visited[u] = true;
    m_disc[u] = m_low[u] = ++m_timer;

    for (int v : m_adj[u]) {
        if (!m_visited[v]) {
            edgeStack.append({u, v});
            m_parent[v] = u;
            dfsBcc(v, edgeStack);

            m_low[u] = qMin(m_low[u], m_low[v]);

            // If u is an articulation point, pop edges to form a BCC
            if ((m_parent[u] == -1 && m_low[v] >= m_disc[u]) ||
                (m_parent[u] != -1 && m_low[v] >= m_disc[u])) {
                QVector<int> comp;
                QSet<int> vertices;
                while (!edgeStack.isEmpty()) {
                    auto edge = edgeStack.takeLast();
                    vertices.insert(edge.first);
                    vertices.insert(edge.second);
                    if ((edge.first == u && edge.second == v) ||
                        (edge.first == v && edge.second == u))
                        break;
                }
                for (int vtx : vertices)
                    comp.append(vtx);
                std::sort(comp.begin(), comp.end());
                m_components.append(comp);
            }
        } else if (v != m_parent[u] && m_disc[v] < m_disc[u]) {
            edgeStack.append({u, v});
            m_low[u] = qMin(m_low[u], m_disc[v]);
        }
    }
}

/* ---- Compute ---- */

void Biconnected10::compute()
{
    QElapsedTimer timer;
    timer.start();

    // Reset DFS state
    m_timer = 0;
    m_disc.fill(0);
    m_low.fill(0);
    m_parent.fill(-1);
    m_visited.fill(false);
    m_bridges.clear();
    m_articulations.clear();
    m_components.clear();

    // Phase 1: Find bridges and articulation points
    for (int i = 0; i < m_n; ++i)
        if (!m_visited[i]) {
            m_parent[i] = -1;
            dfs(i);
        }

    // Phase 2: Extract biconnected components
    m_timer = 0;
    m_disc.fill(0);
    m_low.fill(0);
    m_parent.fill(-1);
    m_visited.fill(false);

    QVector<QPair<int, int>> edgeStack;
    for (int i = 0; i < m_n; ++i)
        if (!m_visited[i])
            dfsBcc(i, edgeStack);

    // Remaining edges form a component
    if (!edgeStack.isEmpty()) {
        QSet<int> vertices;
        for (const auto& e : edgeStack) {
            vertices.insert(e.first);
            vertices.insert(e.second);
        }
        QVector<int> comp;
        for (int v : vertices) comp.append(v);
        std::sort(comp.begin(), comp.end());
        m_components.append(comp);
    }

    m_stats.numBridges = m_bridges.size();
    m_stats.numComponents = m_components.size();
    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit computationCompleted(m_bridges.size(), m_components.size(),
                              timer.elapsed());
}

/* ---- Bridges ---- */

QVector<QPair<int, int>> Biconnected10::bridges() const
{
    return m_bridges;
}

/* ---- Articulation points ---- */

QVector<int> Biconnected10::articulationPoints() const
{
    return m_articulations;
}

/* ---- Components ---- */

QVector<QVector<int>> Biconnected10::components() const
{
    return m_components;
}

/* ---- Vertex-to-component mapping ---- */

QVector<int> Biconnected10::vertexComponent() const
{
    return m_vertComp;
}

/* ---- Bridge-block tree ---- */

QVector<QVector<int>> Biconnected10::bridgeBlockTree() const
{
    // Each biconnected component becomes a block node
    // Bridge endpoints connect block nodes
    int numBlocks = m_components.size();

    // Map each vertex to its block(s)
    QVector<QVector<int>> vertBlocks(m_n);
    for (int b = 0; b < numBlocks; ++b)
        for (int v : m_components[b])
            vertBlocks[v].append(b);

    // Bridge-block tree: nodes are blocks, edges are bridges
    QVector<QVector<int>> tree(numBlocks);
    for (const auto& br : m_bridges) {
        int u = br.first, v = br.second;
        // Find common block or connect blocks
        for (int bu : vertBlocks[u])
            for (int bv : vertBlocks[v])
                if (bu != bv) {
                    if (!tree[bu].contains(bv)) tree[bu].append(bv);
                    if (!tree[bv].contains(bu)) tree[bv].append(bu);
                }
    }
    return tree;
}

/* ---- Reset ---- */

void Biconnected10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_n = 0;
    m_adj.clear();
    m_bridges.clear();
    m_articulations.clear();
    m_components.clear();
}
