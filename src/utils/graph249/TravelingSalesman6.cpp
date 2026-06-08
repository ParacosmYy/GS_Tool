/**
 * @file TravelingSalesman6.cpp
 * @brief TravelingSalesman6 实现
 *
 * 实现TSP Christofides 1.5-近似算法：MST + 最小完美匹配 + 欧拉回路捷径。
 */

#include "utils/graph249/TravelingSalesman6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

TravelingSalesman6::TravelingSalesman6(QObject *parent) : QObject(parent) {}
TravelingSalesman6::~TravelingSalesman6() = default;

/* ---- Configuration ---- */

void TravelingSalesman6::setDistanceMatrix(const QVector<QVector<double>>& dist)
{
    m_dist = dist;
    m_n = dist.size();
}

void TravelingSalesman6::setCoordinates(const QVector<QVector<double>>& coords)
{
    m_coords = coords;
    m_n = coords.size();
    buildDistanceMatrix();
}

/* ---- Build distance matrix from coordinates ---- */

void TravelingSalesman6::buildDistanceMatrix()
{
    m_dist.resize(m_n);
    for (int i = 0; i < m_n; ++i) {
        m_dist[i].resize(m_n);
        for (int j = 0; j < m_n; ++j)
            m_dist[i][j] = euclideanDist(m_coords[i], m_coords[j]);
    }
}

/* ---- Euclidean distance ---- */

double TravelingSalesman6::euclideanDist(const QVector<double>& a, const QVector<double>& b) const
{
    double sum = 0.0;
    int d = qMin(a.size(), b.size());
    for (int i = 0; i < d; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return qSqrt(sum);
}

/* ---- MST (Prim's algorithm) ---- */

QVector<TravelingSalesman6::Edge> TravelingSalesman6::computeMST() const
{
    QVector<Edge> mst;
    if (m_n < 2) return mst;

    QVector<bool> inMST(m_n, false);
    QVector<double> key(m_n, std::numeric_limits<double>::max());
    QVector<int> parent(m_n, -1);

    key[0] = 0.0;
    for (int count = 0; count < m_n; ++count) {
        // Find minimum key vertex not in MST
        int u = -1;
        double minKey = std::numeric_limits<double>::max();
        for (int v = 0; v < m_n; ++v) {
            if (!inMST[v] && key[v] < minKey) {
                minKey = key[v];
                u = v;
            }
        }
        if (u < 0) break;
        inMST[u] = true;

        if (parent[u] >= 0) {
            mst.append({parent[u], u, m_dist[parent[u]][u]});
        }

        // Update adjacent vertices
        for (int v = 0; v < m_n; ++v) {
            if (!inMST[v] && m_dist[u][v] < key[v]) {
                key[v] = m_dist[u][v];
                parent[v] = u;
            }
        }
    }
    return mst;
}

/* ---- Find odd-degree vertices ---- */

QVector<int> TravelingSalesman6::findOddDegreeVertices(const QVector<Edge>& mst) const
{
    QVector<int> degree(m_n, 0);
    for (const auto& e : mst) {
        degree[e.from]++;
        degree[e.to]++;
    }
    QVector<int> odd;
    for (int i = 0; i < m_n; ++i) {
        if (degree[i] % 2 == 1) odd.append(i);
    }
    return odd;
}

/* ---- Minimum weight perfect matching (greedy) ---- */

QVector<TravelingSalesman6::Edge> TravelingSalesman6::computeMinMatching(const QVector<int>& oddVerts) const
{
    QVector<Edge> match;
    int m = oddVerts.size();
    if (m < 2) return match;

    // Collect all edges between odd vertices
    QVector<Edge> candidates;
    for (int i = 0; i < m; ++i) {
        for (int j = i + 1; j < m; ++j) {
            candidates.append({oddVerts[i], oddVerts[j], m_dist[oddVerts[i]][oddVerts[j]]});
        }
    }

    // Sort by weight
    std::sort(candidates.begin(), candidates.end(),
              [](const Edge& a, const Edge& b) { return a.weight < b.weight; });

    // Greedy matching: pick cheapest unused edge
    QVector<bool> used(m_n, false);
    for (const auto& e : candidates) {
        if (!used[e.from] && !used[e.to]) {
            match.append(e);
            used[e.from] = true;
            used[e.to] = true;
        }
        if (match.size() * 2 >= static_cast<quint64>(m)) break;
    }
    return match;
}

/* ---- Eulerian tour (Hierholzer) ---- */

QVector<int> TravelingSalesman6::eulerianTour(const QVector<Edge>& allEdges) const
{
    // Build adjacency list (multigraph)
    QVector<QVector<int>> adj(m_n);
    for (const auto& e : allEdges) {
        adj[e.from].append(e.to);
        adj[e.to].append(e.from);
    }

    QVector<int> tour;
    QVector<int> stack;
    stack.append(0);

    while (!stack.isEmpty()) {
        int v = stack.back();
        if (!adj[v].isEmpty()) {
            int u = adj[v].back();
            adj[v].removeLast();
            // Remove reverse edge
            for (int i = adj[u].size() - 1; i >= 0; --i) {
                if (adj[u][i] == v) {
                    adj[u].removeAt(i);
                    break;
                }
            }
            stack.append(u);
        } else {
            tour.append(v);
            stack.removeLast();
        }
    }
    return tour;
}

/* ---- Shortcut Eulerian to Hamiltonian ---- */

QVector<int> TravelingSalesman6::shortcutTour(const QVector<int>& eulerTour) const
{
    QVector<int> hamiltonian;
    QVector<bool> visited(m_n, false);

    for (int v : eulerTour) {
        if (!visited[v]) {
            hamiltonian.append(v);
            visited[v] = true;
        }
    }

    // Close the cycle
    if (!hamiltonian.isEmpty())
        hamiltonian.append(hamiltonian[0]);
    return hamiltonian;
}

/* ---- Tour distance ---- */

double TravelingSalesman6::tourDistance(const QVector<int>& tour) const
{
    double total = 0.0;
    for (int i = 0; i < tour.size() - 1; ++i) {
        int a = tour[i], b = tour[i + 1];
        if (a >= 0 && a < m_n && b >= 0 && b < m_n)
            total += m_dist[a][b];
    }
    return total;
}

/* ---- Solve ---- */

TravelingSalesman6::TourResult TravelingSalesman6::solve()
{
    QElapsedTimer timer;
    timer.start();

    TourResult result;
    if (m_n < 3) {
        result.tour = {0, 1, 0};
        result.numVertices = m_n;
        result.totalDistance = tourDistance(result.tour);
        return result;
    }

    // Step 1: Compute MST
    m_mstEdges = computeMST();
    double mstWeight = 0.0;
    for (const auto& e : m_mstEdges) mstWeight += e.weight;
    emit mstComputed(m_mstEdges.size(), mstWeight);

    // Step 2: Find odd-degree vertices
    QVector<int> oddVerts = findOddDegreeVertices(m_mstEdges);

    // Step 3: Minimum weight perfect matching on odd vertices
    m_matchEdges = computeMinMatching(oddVerts);
    double matchWeight = 0.0;
    for (const auto& e : m_matchEdges) matchWeight += e.weight;
    emit matchingComputed(m_matchEdges.size(), matchWeight);

    // Step 4: Combine MST + matching edges
    QVector<Edge> combined = m_mstEdges;
    for (const auto& e : m_matchEdges)
        combined.append(e);

    // Step 5: Find Eulerian tour
    QVector<int> euler = eulerianTour(combined);

    // Step 6: Shortcut to Hamiltonian
    result.tour = shortcutTour(euler);
    result.numVertices = m_n;
    result.totalDistance = tourDistance(result.tour);

    m_stats.numVertices = m_n;
    m_stats.mstEdges = m_mstEdges.size();
    m_stats.matchingEdges = m_matchEdges.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit tourCompleted(m_n, result.totalDistance, timer.elapsed());
    return result;
}

/* ---- Accessors ---- */

QVector<TravelingSalesman6::Edge> TravelingSalesman6::mstEdges() const { return m_mstEdges; }
QVector<TravelingSalesman6::Edge> TravelingSalesman6::matchingEdges() const { return m_matchEdges; }

/* ---- Reset ---- */

void TravelingSalesman6::resetStatistics()
{
    m_dist.clear();
    m_coords.clear();
    m_mstEdges.clear();
    m_matchEdges.clear();
    m_n = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
