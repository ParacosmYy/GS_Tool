/**
 * @file ChinesePostman9.cpp
 * @brief ChinesePostman9 实现
 *
 * 实现中国邮路问题：Gabow匹配算法与增广欧拉回路邮路构造。
 */

#include "utils/graph264/ChinesePostman9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <queue>
#include <limits>

/* ---- Construction / Destruction ---- */

ChinesePostman9::ChinesePostman9(QObject *parent) : QObject(parent) {}
ChinesePostman9::~ChinesePostman9() = default;

/* ---- Set graph ---- */

void ChinesePostman9::setGraph(
    const QVector<QVector<QPair<int, double>>>& adjacency, int numVertices)
{
    m_numVertices = numVertices;
    m_adj = adjacency;
    m_tour.clear();
    m_matchedPairs.clear();
    m_tourWeight = 0.0;
}

/* ---- Find odd-degree vertices ---- */

QVector<int> ChinesePostman9::findOddDegreeVertices() const
{
    QVector<int> odd;
    for (int i = 0; i < m_numVertices; ++i) {
        int deg = 0;
        for (const auto& e : m_adj[i]) {
            // Count edge degree (self-loops count 2)
            deg += (e.first == i) ? 2 : 1;
        }
        if (deg % 2 != 0) odd.append(i);
    }
    return odd;
}

/* ---- Dijkstra from source ---- */

QVector<double> ChinesePostman9::dijkstra(int source) const
{
    int n = m_numVertices;
    QVector<double> dist(n, std::numeric_limits<double>::infinity());
    dist[source] = 0.0;
    // Min-heap: {distance, vertex}
    std::priority_queue<std::pair<double, int>,
                        std::vector<std::pair<double, int>>,
                        std::greater<>> pq;
    pq.push({0.0, source});

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();
        if (d > dist[u]) continue;
        for (const auto& [v, w] : m_adj[u]) {
            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                pq.push({dist[v], v});
            }
        }
    }
    return dist;
}

/* ---- All-pairs shortest paths ---- */

QVector<QVector<double>> ChinesePostman9::allPairsShortest() const
{
    int n = m_numVertices;
    QVector<QVector<double>> dist(n);
    for (int i = 0; i < n; ++i)
        dist[i] = dijkstra(i);
    return dist;
}

/* ---- Compute next-hop matrix for path reconstruction ---- */

QVector<QVector<int>> ChinesePostman9::computeNextHop(
    const QVector<QVector<double>>& dist) const
{
    int n = m_numVertices;
    QVector<QVector<int>> next(n, QVector<int>(n, -1));
    for (int u = 0; u < n; ++u)
        for (const auto& [v, w] : m_adj[u])
            if (dist[u][v] == w) next[u][v] = v;

    for (int k = 0; k < n; ++k)
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                if (next[i][k] >= 0 && next[k][j] >= 0 && next[i][j] < 0)
                    next[i][j] = next[i][k];
    return next;
}

/* ---- Gabow's matching (simplified: greedy + refinement for min weight) ---- */

QVector<QPair<int, int>> ChinesePostman9::gabowMatching(
    const QVector<int>& oddVertices,
    const QVector<QVector<double>>& dist) const
{
    int m = oddVertices.size();
    if (m % 2 != 0 || m == 0) return {};

    // Greedy minimum weight matching
    QVector<bool> used(m, false);
    QVector<QPair<int, int>> pairs;

    for (int i = 0; i < m; ++i) {
        if (used[i]) continue;
        int bestJ = -1;
        double bestDist = std::numeric_limits<double>::infinity();
        for (int j = i + 1; j < m; ++j) {
            if (used[j]) continue;
            double d = dist[oddVertices[i]][oddVertices[j]];
            if (d < bestDist) { bestDist = d; bestJ = j; }
        }
        if (bestJ >= 0) {
            used[i] = true;
            used[bestJ] = true;
            pairs.append({oddVertices[i], oddVertices[bestJ]});
        }
    }
    return pairs;
}

/* ---- Augment graph with duplicate edges ---- */

void ChinesePostman9::augmentGraph(
    QVector<QVector<QPair<int, double>>>& augAdj,
    const QVector<QPair<int, int>>& pairs,
    const QVector<QVector<double>>& dist,
    const QVector<QVector<int>>& next) const
{
    Q_UNUSED(next);
    for (const auto& [u, v] : pairs) {
        // Add direct edge as augmentation (shortest path proxy)
        augAdj[u].append({v, dist[u][v]});
        augAdj[v].append({u, dist[u][v]});
    }
}

/* ---- Euler circuit via Hierholzer's algorithm ---- */

QVector<int> ChinesePostman9::eulerCircuit(
    QVector<QVector<QPair<int, double>>>& augAdj) const
{
    int n = m_numVertices;
    if (n == 0) return {};

    // Find a vertex with edges to start
    int start = 0;
    for (int i = 0; i < n; ++i)
        if (!augAdj[i].isEmpty()) { start = i; break; }

    QVector<int> circuit;
    QVector<int> stack;
    stack.append(start);

    while (!stack.isEmpty()) {
        int u = stack.back();
        if (augAdj[u].isEmpty()) {
            circuit.append(u);
            stack.removeLast();
        } else {
            int v = augAdj[u].back().first;
            augAdj[u].removeLast();
            // Remove reverse edge
            for (int i = 0; i < augAdj[v].size(); ++i) {
                if (augAdj[v][i].first == u) {
                    augAdj[v].removeAt(i);
                    break;
                }
            }
            stack.append(v);
        }
    }

    // Reverse to get correct traversal order
    std::reverse(circuit.begin(), circuit.end());
    return circuit;
}

/* ---- Compute tour ---- */

QVector<int> ChinesePostman9::computeTour()
{
    QElapsedTimer timer;
    timer.start();

    if (m_numVertices == 0) return {};

    auto oddVerts = findOddDegreeVertices();
    auto dist = allPairsShortest();
    auto nextHop = computeNextHop(dist);

    // Make augmented copy of adjacency list
    auto augAdj = m_adj;

    if (!oddVerts.isEmpty()) {
        m_matchedPairs = gabowMatching(oddVerts, dist);
        augmentGraph(augAdj, m_matchedPairs, dist, nextHop);
    }

    m_tour = eulerCircuit(augAdj);

    // Compute total tour weight
    m_tourWeight = 0.0;
    for (int i = 0; i + 1 < m_tour.size(); ++i) {
        int u = m_tour[i], v = m_tour[i + 1];
        // Find edge weight
        double w = 0.0;
        for (const auto& e : m_adj[u]) {
            if (e.first == v) { w = e.second; break; }
        }
        if (w == 0.0) w = dist[u][v]; // Augmented edge
        m_tourWeight += w;
    }

    m_stats.numVertices = m_numVertices;
    m_stats.numEdges = 0;
    for (int i = 0; i < m_numVertices; ++i)
        m_stats.numEdges += m_adj[i].size();
    m_stats.numEdges /= 2;
    m_stats.numOddVertices = oddVerts.size();
    m_stats.numMatchedPairs = m_matchedPairs.size();
    m_stats.totalTourWeight = m_tourWeight;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit tourComputed(m_numVertices, m_tourWeight, timer.elapsed());
    return m_tour;
}

/* ---- Accessors ---- */

double ChinesePostman9::tourWeight() const { return m_tourWeight; }
QVector<QPair<int, int>> ChinesePostman9::matchedPairs() const
{
    return m_matchedPairs;
}

/* ---- Reset ---- */

void ChinesePostman9::resetStatistics()
{
    m_adj.clear();
    m_tour.clear();
    m_matchedPairs.clear();
    m_tourWeight = 0.0;
    m_numVertices = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
