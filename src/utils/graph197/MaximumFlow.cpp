/**
 * @file MaximumFlow.cpp
 * @brief MaximumFlow 实现
 *
 * 实现Dinic最大流算法：BFS层次图分层、DFS阻塞流增广、当前弧优化。
 */

#include "utils/graph197/MaximumFlow.h"

#include <QElapsedTimer>
#include <QQueue>
#include <algorithm>

/* ---- Construction / Destruction ---- */

MaximumFlow::MaximumFlow(int numVertices, QObject *parent)
    : QObject(parent), m_n(numVertices),
      m_graph(numVertices), m_level(numVertices), m_iter(numVertices)
{
}

MaximumFlow::~MaximumFlow() = default;

/* ---- Add edge ---- */

void MaximumFlow::addEdge(int from, int to, double capacity)
{
    if (from < 0 || from >= m_n || to < 0 || to >= m_n) return;

    // Forward edge
    m_graph[from].append({to, m_graph[to].size(), capacity});
    // Reverse edge (residual)
    m_graph[to].append({from, m_graph[from].size() - 1, 0.0});
}

/* ---- BFS: build level graph ---- */

bool MaximumFlow::bfs(int source, int sink)
{
    m_level.fill(-1);
    m_level[source] = 0;

    QQueue<int> queue;
    queue.enqueue(source);

    while (!queue.isEmpty()) {
        int v = queue.dequeue();
        for (const auto& e : m_graph[v]) {
            if (e.capacity > 1e-12 && m_level[e.to] < 0) {
                m_level[e.to] = m_level[v] + 1;
                queue.enqueue(e.to);
            }
        }
    }
    return m_level[sink] >= 0;
}

/* ---- DFS: blocking flow ---- */

double MaximumFlow::dfs(int v, int sink, double f)
{
    if (v == sink) return f;

    for (; m_iter[v] < m_graph[v].size(); ++m_iter[v]) {
        auto& e = m_graph[v][m_iter[v]];
        if (e.capacity > 1e-12 && m_level[v] < m_level[e.to]) {
            double d = dfs(e.to, sink, qMin(f, e.capacity));
            if (d > 1e-12) {
                e.capacity -= d;
                m_graph[e.to][e.rev].capacity += d;
                return d;
            }
        }
    }
    return 0.0;
}

/* ---- Dinic DFS with current-arc ---- */

double MaximumFlow::dinicDfs(int v, int sink, double f)
{
    if (v == sink) return f;
    double totalPushed = 0.0;

    for (int& i = m_iter[v]; i < m_graph[v].size(); ++i) {
        auto& e = m_graph[v][i];
        if (e.capacity > 1e-12 && m_level[v] + 1 == m_level[e.to]) {
            double push = dinicDfs(e.to, sink, qMin(f - totalPushed, e.capacity));
            if (push > 1e-12) {
                e.capacity -= push;
                m_graph[e.to][e.rev].capacity += push;
                totalPushed += push;
                if (qAbs(totalPushed - f) < 1e-12) break;
            }
        }
    }
    return totalPushed;
}

/* ---- Max flow ---- */

double MaximumFlow::maxFlow(int source, int sink)
{
    QElapsedTimer timer;
    timer.start();

    // Reset residual capacities by re-adding original capacities
    // (If this is the first run, capacities are already set)
    double flow = 0.0;

    while (bfs(source, sink)) {
        m_iter.fill(0);
        double pushed = 0.0;
        while ((pushed = dinicDfs(source, sink, 1e18)) > 1e-12) {
            flow += pushed;
        }
    }

    int edgeCount = 0;
    for (int i = 0; i < m_n; ++i) edgeCount += m_graph[i].size();
    edgeCount /= 2;

    m_stats.totalRuns++;
    m_stats.numVertices = m_n;
    m_stats.numEdges = edgeCount;
    m_stats.maxFlow = flow;
    m_stats.minCutCapacity = flow;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalRuns;

    emit flowComputed(source, sink, flow);
    return flow;
}

/* ---- Min cut ---- */

QVector<int> MaximumFlow::minCut(int source, int sink) const
{
    // BFS from source in residual graph to find reachable vertices
    QVector<bool> visited(m_n, false);
    QQueue<int> queue;
    queue.enqueue(source);
    visited[source] = true;

    while (!queue.isEmpty()) {
        int v = queue.dequeue();
        for (const auto& e : m_graph[v]) {
            if (e.capacity > 1e-12 && !visited[e.to]) {
                visited[e.to] = true;
                queue.enqueue(e.to);
            }
        }
    }

    QVector<int> cut;
    for (int i = 0; i < m_n; ++i)
        if (visited[i]) cut.append(i);
    Q_UNUSED(sink)
    return cut;
}

/* ---- Edge flow ---- */

double MaximumFlow::edgeFlow(int from, int to) const
{
    for (const auto& e : m_graph[from]) {
        if (e.to == to) {
            // Original capacity minus residual
            return m_graph[to][e.rev].capacity;
        }
    }
    return 0.0;
}

/* ---- Reset ---- */

void MaximumFlow::reset()
{
    for (int i = 0; i < m_n; ++i) m_graph[i].clear();
    m_level.resize(m_n);
    m_iter.resize(m_n);
}

void MaximumFlow::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
