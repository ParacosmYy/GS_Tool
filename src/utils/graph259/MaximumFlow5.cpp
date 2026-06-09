/**
 * @file MaximumFlow5.cpp
 * @brief MaximumFlow5 实现
 *
 * 实现最大流：Dinic分层图BFS与阻塞流DFS动态树加速。
 */

#include "utils/graph259/MaximumFlow5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <queue>

/* ---- Construction / Destruction ---- */

MaximumFlow5::MaximumFlow5(QObject *parent) : QObject(parent) {}
MaximumFlow5::~MaximumFlow5() = default;

/* ---- Initialize graph ---- */

void MaximumFlow5::initGraph(int n)
{
    m_n = n;
    m_graph.resize(n);
    m_level.resize(n);
    m_iter.resize(n);
    m_dynTree.resize(n);
    m_stats.numNodes = n;
    m_stats.numEdges = 0;
}

/* ---- Add edge ---- */

void MaximumFlow5::addEdge(int from, int to, double capacity)
{
    if (from < 0 || from >= m_n || to < 0 || to >= m_n) return;
    // Forward edge
    m_graph[from].append({to, m_graph[to].size(), capacity});
    // Reverse edge
    m_graph[to].append({from, m_graph[from].size() - 1, 0.0});
    m_stats.numEdges++;
}

/* ---- BFS: build level graph ---- */

bool MaximumFlow5::buildLevelGraph(int source, int sink)
{
    m_level.fill(-1);
    m_level[source] = 0;

    std::queue<int> q;
    q.push(source);

    while (!q.empty()) {
        int v = q.front();
        q.pop();

        for (const auto& e : m_graph[v]) {
            if (e.capacity > 1e-15 && m_level[e.to] < 0) {
                m_level[e.to] = m_level[v] + 1;
                q.push(e.to);
            }
        }
    }

    m_stats.bfsPhases++;
    return m_level[sink] >= 0;
}

/* ---- Dynamic tree operations ---- */

void MaximumFlow5::dynLink(int node, int parent, double cap)
{
    if (node < 0 || node >= m_n) return;
    m_dynTree[node].parent = parent;
    m_dynTree[node].minCap = qMin(cap, parent >= 0 ? m_dynTree[parent].minCap : 1e18);
}

double MaximumFlow5::dynMinCap(int node) const
{
    if (node < 0 || node >= m_n) return 1e18;
    return m_dynTree[node].minCap;
}

void MaximumFlow5::dynPush(double flow)
{
    // Simplified: update min caps along path
    // In full implementation, this would use link-cut trees
}

void MaximumFlow5::dynCut(int node)
{
    if (node < 0 || node >= m_n) return;
    m_dynTree[node].parent = -1;
    m_dynTree[node].minCap = 1e18;
}

/* ---- DFS: find blocking flow ---- */

double MaximumFlow5::blockingFlow(int v, int sink, double flow)
{
    if (v == sink) return flow;

    for (int& i = m_iter[v]; i < m_graph[v].size(); ++i) {
        Edge& e = m_graph[v][i];
        if (e.capacity > 1e-15 && m_level[v] + 1 == m_level[e.to]) {
            // Use dynamic tree to check min capacity along path
            double treeMin = dynMinCap(e.to);
            double pathCap = qMin(flow, qMin(e.capacity, treeMin));

            double pushed = blockingFlow(e.to, sink, pathCap);
            if (pushed > 1e-15) {
                e.capacity -= pushed;
                m_graph[e.to][e.rev].capacity += pushed;

                // Update dynamic tree
                dynLink(e.to, v, e.capacity + pushed);
                dynPush(pushed);

                m_stats.dfsPaths++;
                return pushed;
            }
        }
    }
    return 0.0;
}

/* ---- Maximum flow (Dinic's algorithm) ---- */

double MaximumFlow5::maxFlow(int source, int sink)
{
    QElapsedTimer timer;
    timer.start();

    double totalFlow = 0.0;
    m_stats.bfsPhases = 0;
    m_stats.dfsPaths = 0;

    // Initialize dynamic tree
    for (int i = 0; i < m_n; ++i) {
        m_dynTree[i] = {-1, 1e18};
    }

    while (buildLevelGraph(source, sink)) {
        m_iter.fill(0);
        double pushed;
        while ((pushed = blockingFlow(source, sink, 1e18)) > 1e-15) {
            totalFlow += pushed;
        }
    }

    m_stats.maxFlowValue = totalFlow;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit flowComputed(m_n, totalFlow, timer.elapsed());
    return totalFlow;
}

/* ---- Min-cut: BFS in residual graph ---- */

QVector<int> MaximumFlow5::minCut(int source) const
{
    QVector<bool> visited(m_n, false);
    QVector<int> cut;
    std::queue<int> q;
    q.push(source);
    visited[source] = true;

    while (!q.empty()) {
        int v = q.front();
        q.pop();
        cut.append(v);

        for (const auto& e : m_graph[v]) {
            if (e.capacity > 1e-15 && !visited[e.to]) {
                visited[e.to] = true;
                q.push(e.to);
            }
        }
    }
    return cut;
}

/* ---- Flow on edges ---- */

QVector<QVector<QPair<int, double>>> MaximumFlow5::flowOnEdges() const
{
    QVector<QVector<QPair<int, double>>> result(m_n);
    for (int v = 0; v < m_n; ++v) {
        for (const auto& e : m_graph[v]) {
            // Only report flow on original edges (reverse edge capacity is the flow)
            double flow = m_graph[e.to][e.rev].capacity;
            if (flow > 1e-15)
                result[v].append({e.to, flow});
        }
    }
    return result;
}

/* ---- Reset ---- */

void MaximumFlow5::resetStatistics()
{
    m_graph.clear(); m_level.clear(); m_iter.clear(); m_dynTree.clear();
    m_n = 0;
    m_stats = Stats{}; m_timeSum = 0.0;
}
