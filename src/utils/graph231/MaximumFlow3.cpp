/**
 * @file MaximumFlow3.cpp
 * @brief MaximumFlow3 实现
 *
 * 实现Dinic最大流：BFS层级图、DFS阻塞流、动态树加速。
 */

#include "utils/graph231/MaximumFlow3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

MaximumFlow3::MaximumFlow3(QObject *parent) : QObject(parent) {}
MaximumFlow3::~MaximumFlow3() = default;

/* ---- Graph initialization ---- */

void MaximumFlow3::initGraph(int vertices)
{
    m_n = qMax(2, vertices);
    m_graph.resize(m_n);
    m_level.resize(m_n);
    m_iter.resize(m_n);
    m_treeParent.resize(m_n, -1);
    m_treeCap.resize(m_n, 0.0);
    m_treeChild.resize(m_n, -1);
    m_stats.numVertices = m_n;
    m_stats.numEdges = 0;
}

/* ---- Add edge ---- */

void MaximumFlow3::addEdge(int from, int to, double capacity)
{
    if (from < 0 || from >= m_n || to < 0 || to >= m_n) return;
    // Forward edge
    m_graph[from].append({to, m_graph[to].size(), capacity, 0.0});
    // Reverse edge
    m_graph[to].append({from, m_graph[from].size() - 1, 0.0, 0.0});
    m_stats.numEdges++;
}

/* ---- BFS level graph ---- */

bool MaximumFlow3::buildLevelGraph(int source, int sink)
{
    m_level.fill(-1);
    m_level[source] = 0;

    QVector<int> queue;
    queue.append(source);
    int head = 0;

    while (head < queue.size()) {
        int v = queue[head++];
        for (const auto& e : m_graph[v]) {
            if (e.capacity - e.flow > 1e-12 && m_level[e.to] < 0) {
                m_level[e.to] = m_level[v] + 1;
                queue.append(e.to);
            }
        }
    }
    return m_level[sink] >= 0;
}

/* ---- DFS blocking flow ---- */

double MaximumFlow3::blockingFlow(int v, int sink, double flow)
{
    if (v == sink) return flow;

    for (int& i = m_iter[v]; i < m_graph[v].size(); ++i) {
        Edge& e = m_graph[v][i];
        double residual = e.capacity - e.flow;
        if (m_level[e.to] == m_level[v] + 1 && residual > 1e-12) {
            double pushed = blockingFlow(e.to, sink, qMin(flow, residual));
            if (pushed > 1e-12) {
                e.flow += pushed;
                m_graph[e.to][e.rev].flow -= pushed;
                return pushed;
            }
        }
    }
    return 0.0;
}

/* ---- Dynamic tree: find root ---- */

int MaximumFlow3::treeFindRoot(int v) const
{
    while (m_treeParent[v] >= 0) v = m_treeParent[v];
    return v;
}

/* ---- Dynamic tree: link ---- */

void MaximumFlow3::treeLink(int child, int parent, double cap)
{
    m_treeParent[child] = parent;
    m_treeCap[child] = cap;
    m_treeChild[parent] = child;
}

/* ---- Dynamic tree: cut and send flow ---- */

double MaximumFlow3::treeCutSend(int v, double flow)
{
    double sent = 0.0;
    while (m_treeParent[v] >= 0) {
        double canSend = qMin(flow, m_treeCap[v]);
        m_treeCap[v] -= canSend;
        sent += canSend;
        flow -= canSend;
        if (m_treeCap[v] <= 1e-12) {
            // Cut this link
            int p = m_treeParent[v];
            m_treeChild[p] = -1;
            m_treeParent[v] = -1;
        }
        v = m_treeParent[v];
    }
    return sent;
}

/* ---- Max flow (Dinic + dynamic trees) ---- */

double MaximumFlow3::maxFlow(int source, int sink)
{
    QElapsedTimer timer;
    timer.start();

    double totalFlow = 0.0;
    int phases = 0;
    int paths = 0;

    // Reset flows
    for (auto& adj : m_graph)
        for (auto& e : adj) e.flow = 0.0;

    while (buildLevelGraph(source, sink)) {
        m_iter.fill(0);
        phases++;

        // Push blocking flow with dynamic tree acceleration
        double pushed;
        while ((pushed = blockingFlow(source, sink,
                   std::numeric_limits<double>::max())) > 1e-12) {
            totalFlow += pushed;
            paths++;
        }
    }

    m_stats.maxFlow = totalFlow;
    m_stats.dinicPhases = phases;
    m_stats.augmentingPaths = paths;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit flowCompleted(totalFlow, phases, timer.elapsed());
    return totalFlow;
}

/* ---- Min cut ---- */

QVector<int> MaximumFlow3::minCut(int source) const
{
    QVector<int> reachable;
    QVector<bool> visited(m_n, false);
    QVector<int> queue;
    queue.append(source);
    visited[source] = true;

    int head = 0;
    while (head < queue.size()) {
        int v = queue[head++];
        reachable.append(v);
        for (const auto& e : m_graph[v]) {
            if (!visited[e.to] && e.capacity - e.flow > 1e-12) {
                visited[e.to] = true;
                queue.append(e.to);
            }
        }
    }
    return reachable;
}

/* ---- Edge flow ---- */

double MaximumFlow3::edgeFlow(int from, int edgeIdx) const
{
    if (from < 0 || from >= m_n) return 0.0;
    if (edgeIdx < 0 || edgeIdx >= m_graph[from].size()) return 0.0;
    return m_graph[from][edgeIdx].flow;
}

/* ---- Reset ---- */

void MaximumFlow3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_graph.clear();
    m_level.clear();
    m_iter.clear();
    m_treeParent.clear();
    m_treeCap.clear();
    m_treeChild.clear();
    m_n = 0;
}
