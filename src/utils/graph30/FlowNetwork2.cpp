/**
 * @file FlowNetwork2.cpp
 * @brief 最大流网络(Dinic算法)实现
 */

#include "FlowNetwork2.h"
#include <QElapsedTimer>
#include <queue>
#include <algorithm>
#include <limits>

FlowNetwork2::FlowNetwork2(int nodeCount, QObject* parent)
    : QObject(parent)
    , m_n(nodeCount)
    , m_timeSum(0.0)
{
    m_graph.resize(m_n);
    m_level.resize(m_n);
}

void FlowNetwork2::addEdge(int from, int to, double capacity,
                            bool bidirectional)
{
    Edge forward = {to, static_cast<int>(m_graph[to].size()), capacity};
    Edge backward = {from, static_cast<int>(m_graph[from].size()),
                     bidirectional ? capacity : 0.0};
    m_graph[from].append(forward);
    m_graph[to].append(backward);
}

double FlowNetwork2::maxFlow(int source, int sink)
{
    QElapsedTimer timer;
    timer.start();

    double totalFlow = 0.0;
    int augmentPaths = 0;

    while (buildLevelGraph(source, sink)) {
        QVector<int> iter(m_n, 0);
        double pushed;
        while ((pushed = sendFlow(source, sink,
                                   std::numeric_limits<double>::max(), iter)) > 1e-12) {
            totalFlow += pushed;
            augmentPaths++;
        }
    }

    m_lastMaxFlow = totalFlow;
    m_lastSource = source;
    m_lastSink = sink;

    m_stats.totalMaxFlows++;
    m_stats.totalAugmentPaths += augmentPaths;
    m_timeSum += timer.elapsed();
    if (m_stats.totalMaxFlows > 0)
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalMaxFlows;

    emit maxFlowCompleted(totalFlow, augmentPaths);
    return totalFlow;
}

QVector<int> FlowNetwork2::minCut(int source) const
{
    QVector<bool> visited(m_n, false);
    std::queue<int> q;
    q.push(source);
    visited[source] = true;

    while (!q.empty()) {
        int u = q.front();
        q.pop();
        for (const Edge& e : m_graph[u]) {
            if (!visited[e.to] && e.cap > 1e-12) {
                visited[e.to] = true;
                q.push(e.to);
            }
        }
    }

    QVector<int> cut;
    for (int i = 0; i < m_n; ++i)
        if (visited[i]) cut.append(i);
    return cut;
}

QList<FlowNetwork2::EdgeInfo> FlowNetwork2::edgeFlows() const
{
    QList<EdgeInfo> result;
    for (int u = 0; u < m_n; ++u) {
        for (int i = 0; i < m_graph[u].size(); ++i) {
            const Edge& e = m_graph[u][i];
            if (i < m_graph[e.to].size()) {
                const Edge& rev = m_graph[e.to][e.rev];
                if (rev.rev == i && u < e.to) {
                    EdgeInfo info;
                    info.from = u;
                    info.to = e.to;
                    info.capacity = e.cap + m_graph[e.to][e.rev].cap;
                    info.flow = m_graph[e.to][e.rev].cap;
                    result.append(info);
                }
            }
        }
    }
    return result;
}

void FlowNetwork2::resetFlows()
{
    for (int u = 0; u < m_n; ++u) {
        for (Edge& e : m_graph[u]) {
            if (e.rev >= 0 && e.rev < m_graph[e.to].size()) {
                Edge& rev = m_graph[e.to][e.rev];
                if (rev.rev >= 0 && rev.rev < m_graph[u].size() &&
                    &m_graph[u][rev.rev] == &e) {
                    /* 恢复原始容量 */
                    double total = e.cap + rev.cap;
                    if (u < e.to) {
                        e.cap = total;
                        rev.cap = 0.0;
                    }
                }
            }
        }
    }
}

bool FlowNetwork2::buildLevelGraph(int source, int sink)
{
    std::fill(m_level.begin(), m_level.end(), -1);
    std::queue<int> q;
    m_level[source] = 0;
    q.push(source);

    while (!q.empty()) {
        int u = q.front();
        q.pop();
        for (const Edge& e : m_graph[u]) {
            if (m_level[e.to] < 0 && e.cap > 1e-12) {
                m_level[e.to] = m_level[u] + 1;
                q.push(e.to);
            }
        }
    }

    m_stats.totalBfsLayers++;
    return m_level[sink] >= 0;
}

double FlowNetwork2::sendFlow(int u, int sink, double flow, QVector<int>& iter)
{
    if (u == sink) return flow;

    for (int& i = iter[u]; i < m_graph[u].size(); ++i) {
        Edge& e = m_graph[u][i];
        if (m_level[e.to] == m_level[u] + 1 && e.cap > 1e-12) {
            double pushed = sendFlow(e.to, sink, qMin(flow, e.cap), iter);
            if (pushed > 1e-12) {
                e.cap -= pushed;
                m_graph[e.to][e.rev].cap += pushed;
                return pushed;
            }
        }
    }
    return 0.0;
}

void FlowNetwork2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
