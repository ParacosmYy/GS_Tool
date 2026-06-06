/**
 * @file ChinesePostman.cpp
 * @brief ChinesePostman 实现
 *
 * 实现中国邮路问题：奇度顶点配对、Floyd最短路、最小权完美匹配、欧拉回路构造。
 */

#include "utils/graph195/ChinesePostman.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

ChinesePostman::ChinesePostman(QObject *parent) : QObject(parent) {}
ChinesePostman::~ChinesePostman() = default;

/* ---- Set graph ---- */

void ChinesePostman::setGraph(int numVertices, const QVector<Edge>& edges)
{
    m_numVertices = numVertices;
    m_edges = edges;
    m_adj.resize(numVertices);
    for (auto& list : m_adj) list.clear();

    for (const auto& e : edges) {
        m_adj[e.from].append({e.to, e.weight});
        m_adj[e.to].append({e.from, e.weight});
    }
}

/* ---- Find odd-degree vertices ---- */

QVector<int> ChinesePostman::findOddDegreeVertices() const
{
    QVector<int> odds;
    for (int v = 0; v < m_numVertices; ++v) {
        if (m_adj[v].size() % 2 != 0)
            odds.append(v);
    }
    return odds;
}

/* ---- Floyd-Warshall ---- */

QVector<QVector<double>> ChinesePostman::allPairsShortestPath() const
{
    int n = m_numVertices;
    QVector<QVector<double>> dist(n, QVector<double>(n, 1e30));

    for (int i = 0; i < n; ++i) dist[i][i] = 0.0;
    for (const auto& e : m_edges) {
        dist[e.from][e.to] = qMin(dist[e.from][e.to], e.weight);
        dist[e.to][e.from] = qMin(dist[e.to][e.from], e.weight);
    }

    for (int k = 0; k < n; ++k)
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                if (dist[i][k] + dist[k][j] < dist[i][j])
                    dist[i][j] = dist[i][k] + dist[k][j];

    return dist;
}

/* ---- Min-weight matching (brute-force for small sets) ---- */

void ChinesePostman::matchSearch(const QVector<int>& odds, int idx, double currentCost,
                                   QVector<bool>& used, double& bestCost,
                                   QVector<QPair<int, int>>& currentPairs,
                                   QVector<QPair<int, int>>& bestPairs,
                                   const QVector<QVector<double>>& dist) const
{
    if (idx >= odds.size()) {
        if (currentCost < bestCost) {
            bestCost = currentCost;
            bestPairs = currentPairs;
        }
        return;
    }
    if (used[idx]) {
        matchSearch(odds, idx + 1, currentCost, used, bestCost, currentPairs, bestPairs, dist);
        return;
    }

    used[idx] = true;
    for (int j = idx + 1; j < odds.size(); ++j) {
        if (!used[j]) {
            used[j] = true;
            currentPairs.append({odds[idx], odds[j]});
            double addedCost = dist[odds[idx]][odds[j]];
            matchSearch(odds, idx + 1, currentCost + addedCost, used, bestCost,
                        currentPairs, bestPairs, dist);
            currentPairs.removeLast();
            used[j] = false;
        }
    }
    used[idx] = false;
}

double ChinesePostman::minWeightMatching(const QVector<int>& odds,
                                          const QVector<QVector<double>>& dist,
                                          QVector<QPair<int, int>>& pairs) const
{
    int n = odds.size();
    if (n == 0) return 0.0;

    QVector<bool> used(n, false);
    double bestCost = 1e30;
    QVector<QPair<int, int>> currentPairs;
    pairs.clear();

    matchSearch(odds, 0, 0.0, used, bestCost, currentPairs, pairs, dist);
    return bestCost;
}

/* ---- Euler circuit (Hierholzer) ---- */

void ChinesePostman::hierholzer(int start, QVector<QVector<QPair<int, double>>>& augAdj,
                                 QVector<int>& circuit) const
{
    QVector<int> stack;
    stack.append(start);

    while (!stack.isEmpty()) {
        int v = stack.last();
        if (augAdj[v].isEmpty()) {
            circuit.append(v);
            stack.removeLast();
        } else {
            auto [u, w] = augAdj[v].last();
            augAdj[v].removeLast();

            // Remove reverse edge
            for (int i = augAdj[u].size() - 1; i >= 0; --i) {
                if (augAdj[u][i].first == v) {
                    augAdj[u].removeAt(i);
                    break;
                }
            }
            stack.append(u);
        }
    }

    // Reverse to get correct order
    std::reverse(circuit.begin(), circuit.end());
}

QVector<int> ChinesePostman::eulerCircuit(const QVector<QPair<int, int>>& extraEdges) const
{
    // Build augmented adjacency with extra edges (duplicated)
    auto augAdj = m_adj;
    for (const auto& [u, v] : extraEdges) {
        // Find edge weight
        double w = 1.0;
        for (const auto& [nei, wt] : m_adj[u]) {
            if (nei == v) { w = wt; break; }
        }
        augAdj[u].append({v, w});
        augAdj[v].append({u, w});
    }

    QVector<int> circuit;
    if (m_numVertices > 0)
        hierholzer(0, augAdj, circuit);
    return circuit;
}

/* ---- Total cost ---- */

double ChinesePostman::totalCost() const { return m_totalCost; }

/* ---- Solve ---- */

QVector<int> ChinesePostman::solve()
{
    QElapsedTimer timer;
    timer.start();

    if (m_numVertices == 0 || m_edges.isEmpty()) return {};

    // Step 1: Find odd-degree vertices
    auto odds = findOddDegreeVertices();

    // Step 2: All-pairs shortest path
    auto dist = allPairsShortestPath();

    // Step 3: Min-weight matching of odd vertices
    QVector<QPair<int, int>> pairs;
    double matchingCost = minWeightMatching(odds, dist, pairs);

    // Step 4: Compute base edge cost
    double baseCost = 0.0;
    for (const auto& e : m_edges) baseCost += e.weight;

    m_totalCost = baseCost + matchingCost;

    // Step 5: Construct Euler circuit
    auto circuit = eulerCircuit(pairs);

    m_stats.totalRuns++;
    m_stats.numVertices = m_numVertices;
    m_stats.numEdges = m_edges.size();
    m_stats.numOddVertices = odds.size();
    m_stats.totalCost = m_totalCost;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalRuns;

    emit solved(m_edges.size(), m_totalCost);
    return circuit;
}

/* ---- Reset ---- */

void ChinesePostman::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
