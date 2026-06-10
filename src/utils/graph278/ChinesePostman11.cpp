/**
 * @file ChinesePostman11.cpp
 * @brief ChinesePostman11 实现
 *
 * 实现中国邮路问题：Hierholzer回路遍历与死边最小化最优欧拉环游构造。
 */

#include "utils/graph278/ChinesePostman11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

ChinesePostman11::ChinesePostman11(QObject *parent)
    : QObject(parent) {}

ChinesePostman11::~ChinesePostman11() = default;

/* ---- Set graph ---- */

void ChinesePostman11::setGraph(int numVertices, const QVector<Edge>& edges)
{
    m_n = qMax(1, numVertices);
    m_edges = edges;
    m_duplicates.clear();
    m_tour.clear();
    m_tourCost = 0.0;
}

/* ---- Find odd degree vertices ---- */

QVector<int> ChinesePostman11::findOddDegreeVertices() const
{
    QVector<int> degree(m_n, 0);
    for (const auto& e : m_edges) {
        degree[e.from]++;
        degree[e.to]++;
    }
    QVector<int> odd;
    for (int i = 0; i < m_n; ++i)
        if (degree[i] % 2 != 0) odd.append(i);
    return odd;
}

/* ---- Floyd-Warshall all-pairs shortest path ---- */

void ChinesePostman11::floydWarshall(QVector<QVector<double>>& dist,
                                      QVector<QVector<int>>& next) const
{
    dist.assign(m_n, QVector<double>(m_n, std::numeric_limits<double>::max()));
    next.assign(m_n, QVector<int>(m_n, -1));

    for (int i = 0; i < m_n; ++i) dist[i][i] = 0.0;

    for (const auto& e : m_edges) {
        if (e.weight < dist[e.from][e.to]) {
            dist[e.from][e.to] = e.weight;
            dist[e.to][e.from] = e.weight;
            next[e.from][e.to] = e.to;
            next[e.to][e.from] = e.from;
        }
    }

    for (int k = 0; k < m_n; ++k)
        for (int i = 0; i < m_n; ++i)
            for (int j = 0; j < m_n; ++j)
                if (dist[i][k] + dist[k][j] < dist[i][j]) {
                    dist[i][j] = dist[i][k] + dist[k][j];
                    next[i][j] = next[i][k];
                }
}

/* ---- Minimum weight perfect matching (greedy heuristic) ---- */

QVector<QPair<int, int>> ChinesePostman11::minWeightMatching(
    const QVector<int>& oddVerts,
    const QVector<QVector<double>>& dist)
{
    // Build all pairs sorted by distance (greedy matching)
    QVector<QPair<double, QPair<int, int>>> pairs;
    for (int i = 0; i < oddVerts.size(); ++i)
        for (int j = i + 1; j < oddVerts.size(); ++j)
            pairs.append({dist[oddVerts[i]][oddVerts[j]], {oddVerts[i], oddVerts[j]}});

    std::sort(pairs.begin(), pairs.end());

    QVector<bool> used(m_n, false);
    QVector<QPair<int, int>> matching;
    for (const auto& p : pairs) {
        int u = p.second.first, v = p.second.second;
        if (!used[u] && !used[v]) {
            matching.append({u, v});
            used[u] = used[v] = true;
        }
    }
    return matching;
}

/* ---- Build adjacency from original + duplicated edges ---- */

QVector<QVector<QPair<int, int>>> ChinesePostman11::buildAdjacency() const
{
    QVector<QVector<QPair<int, int>>> adj(m_n);
    // Each edge gets a unique ID for Hierholzer to track
    int edgeId = 0;
    auto addEdge = [&](int u, int v) {
        adj[u].append({v, edgeId});
        adj[v].append({u, edgeId});
        edgeId++;
    };

    for (const auto& e : m_edges) addEdge(e.from, e.to);
    for (const auto& e : m_duplicates) addEdge(e.from, e.to);

    return adj;
}

/* ---- Hierholzer Euler tour ---- */

QVector<int> ChinesePostman11::hierholzerTour()
{
    auto adj = buildAdjacency();
    if (adj.isEmpty() || m_n == 0) return {};

    // Track used edges by ID
    QSet<int> usedEdges;

    int start = 0;
    // Find a vertex with at least one edge
    for (int i = 0; i < m_n; ++i) {
        if (!adj[i].isEmpty()) { start = i; break; }
    }

    QVector<int> stack;
    QVector<int> circuit;
    stack.append(start);

    while (!stack.isEmpty()) {
        int v = stack.last();
        // Find unused edge from v
        int found = -1;
        while (!adj[v].isEmpty()) {
            auto [to, eid] = adj[v].last();
            if (usedEdges.contains(eid)) {
                adj[v].pop_back();
                continue;
            }
            found = to;
            usedEdges.insert(eid);
            adj[v].pop_back();
            break;
        }

        if (found >= 0) {
            stack.append(found);
        } else {
            circuit.append(v);
            stack.removeLast();
        }
    }

    // Circuit is in reverse order
    std::reverse(circuit.begin(), circuit.end());
    return circuit;
}

/* ---- Solve ---- */

QVector<int> ChinesePostman11::solve()
{
    QElapsedTimer timer;
    timer.start();

    // Step 1: Find odd-degree vertices
    QVector<int> oddVerts = findOddDegreeVertices();

    // Step 2: If no odd vertices, graph is already Eulerian
    // Step 3: Compute shortest paths between all pairs
    QVector<QVector<double>> dist;
    QVector<QVector<int>> next;
    floydWarshall(dist, next);

    // Step 4: Minimum weight perfect matching on odd vertices
    double matchCost = 0.0;
    m_duplicates.clear();

    if (oddVerts.size() > 0) {
        auto matching = minWeightMatching(oddVerts, dist);
        for (const auto& [u, v] : matching) {
            // Reconstruct shortest path and add edges
            int cur = u;
            while (cur != v) {
                int nxt = next[cur][v];
                if (nxt < 0) break;
                Edge dup;
                dup.from = cur;
                dup.to = nxt;
                dup.weight = dist[cur][nxt];
                dup.isDuplicate = true;
                m_duplicates.append(dup);
                matchCost += dup.weight;
                cur = nxt;
            }
        }
    }

    // Step 5: Build Euler tour using Hierholzer
    m_tour = hierholzerTour();

    // Step 6: Compute total cost
    m_tourCost = 0.0;
    for (const auto& e : m_edges) m_tourCost += e.weight;
    m_tourCost += matchCost;

    double elapsed = timer.elapsed();
    m_stats.numVertices = m_n;
    m_stats.numEdges = m_edges.size();
    m_stats.numOddVertices = oddVerts.size();
    m_stats.totalCost = m_tourCost;
    m_stats.matchingCost = matchCost;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit tourComputed(m_n, m_tourCost, matchCost, elapsed);
    return m_tour;
}

/* ---- Accessors ---- */

double ChinesePostman11::tourCost() const { return m_tourCost; }

QVector<ChinesePostman11::Edge> ChinesePostman11::duplicatedEdges() const
{
    return m_duplicates;
}

/* ---- Reset ---- */

void ChinesePostman11::resetStatistics()
{
    m_n = 0;
    m_edges.clear();
    m_duplicates.clear();
    m_tour.clear();
    m_tourCost = 0.0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
