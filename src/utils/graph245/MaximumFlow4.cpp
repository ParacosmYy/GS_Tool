/**
 * @file MaximumFlow4.cpp
 * @brief MaximumFlow4 实现
 *
 * 实现最大流：Highest-Label推重标签与间隙启发式。
 */

#include "utils/graph245/MaximumFlow4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

MaximumFlow4::MaximumFlow4(QObject *parent) : QObject(parent) {}
MaximumFlow4::~MaximumFlow4() = default;

/* ---- Initialize graph ---- */

void MaximumFlow4::initGraph(int n)
{
    m_n = n;
    m_graph.clear();
    m_graph.resize(n);
    m_stats.numVertices = n;
    m_stats.numEdges = 0;
}

/* ---- Add edge ---- */

void MaximumFlow4::addEdge(int from, int to, double capacity, double revCapacity)
{
    if (from < 0 || from >= m_n || to < 0 || to >= m_n) return;

    Edge forward{to, m_graph[to].size(), capacity, 0.0};
    Edge backward{from, m_graph[from].size(), revCapacity, 0.0};

    m_graph[from].append(forward);
    m_graph[to].append(backward);
    m_stats.numEdges += 2;
}

/* ---- Push ---- */

bool MaximumFlow4::push(int u, QVector<double>& excess, QVector<int>& height)
{
    if (excess[u] <= 0.0) return false;

    for (auto& e : m_graph[u]) {
        if (e.capacity - e.flow <= 0.0) continue;
        if (height[u] != height[e.to] + 1) continue;

        double pushAmt = qMin(excess[u], e.capacity - e.flow);
        e.flow += pushAmt;
        m_graph[e.to][e.rev].flow -= pushAmt;
        excess[u] -= pushAmt;
        excess[e.to] += pushAmt;
        m_stats.numPushes++;
        return true;
    }
    return false;
}

/* ---- Relabel ---- */

void MaximumFlow4::relabel(int u, const QVector<double>& excess, QVector<int>& height)
{
    if (excess[u] <= 0.0) return;

    int minHeight = std::numeric_limits<int>::max();
    for (const auto& e : m_graph[u]) {
        if (e.capacity - e.flow > 0.0)
            minHeight = qMin(minHeight, height[e.to]);
    }

    if (minHeight < std::numeric_limits<int>::max()) {
        height[u] = minHeight + 1;
        m_stats.numRelabels++;
    }
}

/* ---- Gap heuristic ---- */

void MaximumFlow4::gapHeuristic(int gapLevel, QVector<int>& height, QVector<double>& excess)
{
    // If no vertex has height == gapLevel, relabel all above to n+1
    bool hasGap = false;
    for (int i = 0; i < m_n; ++i) {
        if (height[i] == gapLevel) { hasGap = true; break; }
    }
    if (hasGap) return;

    int affected = 0;
    for (int i = 0; i < m_n; ++i) {
        if (height[i] > gapLevel) {
            height[i] = m_n + 1;
            affected++;
        }
    }
    m_stats.numGaps++;
    emit gapDetected(gapLevel, affected);
}

/* ---- Discharge ---- */

void MaximumFlow4::discharge(int u, QVector<double>& excess, QVector<int>& height,
                               QVector<int>& currentEdge, int sink)
{
    while (excess[u] > 0.0) {
        if (currentEdge[u] < m_graph[u].size()) {
            Edge& e = m_graph[u][currentEdge[u]];
            if (e.capacity - e.flow > 0.0 && height[u] == height[e.to] + 1) {
                double pushAmt = qMin(excess[u], e.capacity - e.flow);
                e.flow += pushAmt;
                m_graph[e.to][e.rev].flow -= pushAmt;
                excess[u] -= pushAmt;
                excess[e.to] += pushAmt;
                m_stats.numPushes++;
            } else {
                currentEdge[u]++;
            }
        } else {
            // Relabel and apply gap heuristic
            int oldHeight = height[u];
            relabel(u, excess, height);
            gapHeuristic(oldHeight, height, excess);
            currentEdge[u] = 0;
            if (height[u] == oldHeight) break; // no progress
        }
    }
}

/* ---- BFS reachable ---- */

QVector<int> MaximumFlow4::bfsReachable(int source, const QVector<int>& height) const
{
    QVector<int> reachable;
    QVector<bool> visited(m_n, false);
    QVector<int> queue;
    queue.append(source);
    visited[source] = true;

    while (!queue.isEmpty()) {
        int u = queue.takeFirst();
        reachable.append(u);

        for (const auto& e : m_graph[u]) {
            if (!visited[e.to] && e.capacity - e.flow > 0.0) {
                visited[e.to] = true;
                queue.append(e.to);
            }
        }
    }
    return reachable;
}

/* ---- Max flow ---- */

double MaximumFlow4::maxFlow(int source, int sink)
{
    QElapsedTimer timer;
    timer.start();

    if (source < 0 || sink < 0 || source >= m_n || sink >= m_n) return 0.0;

    // Initialize
    QVector<double> excess(m_n, 0.0);
    QVector<int> height(m_n, 0);
    QVector<int> currentEdge(m_n, 0);

    height[source] = m_n;

    // Saturate all edges from source
    for (auto& e : m_graph[source]) {
        if (e.capacity > 0.0) {
            double flow = e.capacity;
            e.flow = flow;
            m_graph[e.to][e.rev].flow = -flow;
            excess[e.to] += flow;
            excess[source] -= flow;
            m_stats.numPushes++;
        }
    }

    // Highest-label selection: process active vertices from highest to lowest
    int maxH = 2 * m_n;
    for (int h = maxH; h >= 0; --h) {
        bool found = true;
        while (found) {
            found = false;
            for (int u = 0; u < m_n; ++u) {
                if (u != source && u != sink && excess[u] > 0.0 && height[u] == h) {
                    discharge(u, excess, height, currentEdge, sink);
                    found = true;
                }
            }
        }
    }

    // Compute max flow = excess entering sink
    double flow = 0.0;
    for (const auto& e : m_graph[sink])
        flow -= e.flow;  // reverse edges carry negative flow

    m_stats.maxFlow = flow;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit flowComputed(flow, m_stats.numPushes, m_stats.numRelabels, timer.elapsed());
    return flow;
}

/* ---- Min cut ---- */

QVector<int> MaximumFlow4::minCut(int source) const
{
    QVector<int> dummy(m_n, 0);
    return bfsReachable(source, dummy);
}

/* ---- Edge flow ---- */

double MaximumFlow4::edgeFlow(int from, int edgeIdx) const
{
    if (from < 0 || from >= m_n) return 0.0;
    if (edgeIdx < 0 || edgeIdx >= m_graph[from].size()) return 0.0;
    return m_graph[from][edgeIdx].flow;
}

/* ---- Edges from vertex ---- */

QVector<MaximumFlow4::Edge> MaximumFlow4::edgesFrom(int vertex) const
{
    if (vertex < 0 || vertex >= m_n) return QVector<Edge>();
    return m_graph[vertex];
}

/* ---- Reset ---- */

void MaximumFlow4::resetStatistics()
{
    m_graph.clear();
    m_n = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
