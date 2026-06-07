/**
 * @file ChinesePostman2.cpp
 * @brief ChinesePostman2 实现
 *
 * 实现有向图中国邮路问题：弧平衡、最小费用流匹配、欧拉回路构建。
 */

#include "utils/graph211/ChinesePostman2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

ChinesePostman2::ChinesePostman2(QObject *parent) : QObject(parent) {}
ChinesePostman2::~ChinesePostman2() = default;

/* ---- Configuration ---- */

void ChinesePostman2::setMaxIterations(int iter) { m_maxIter = qMax(100, iter); }

/* ---- Build adjacency list ---- */

QVector<QVector<QPair<int, double>>> ChinesePostman2::buildAdjList(
    int n, const QVector<Arc>& arcs) const
{
    QVector<QVector<QPair<int, double>>> adj(n);
    for (const auto& a : arcs) {
        if (a.from >= 0 && a.from < n && a.to >= 0 && a.to < n)
            adj[a.from].append({a.to, a.cost});
    }
    return adj;
}

/* ---- Arc balance ---- */

QVector<int> ChinesePostman2::arcBalance(int numNodes, const QVector<Arc>& arcs) const
{
    QVector<int> balance(numNodes, 0);
    for (const auto& a : arcs) {
        if (a.from >= 0 && a.from < numNodes) balance[a.from]++;
        if (a.to >= 0 && a.to < numNodes) balance[a.to]--;
    }
    return balance;
}

/* ---- Floyd-Warshall all-pairs shortest ---- */

QVector<QVector<double>> ChinesePostman2::allPairsShortest(
    int numNodes, const QVector<Arc>& arcs) const
{
    const double INF = std::numeric_limits<double>::max() / 2.0;
    QVector<QVector<double>> dist(numNodes, QVector<double>(numNodes, INF));

    for (int i = 0; i < numNodes; ++i) dist[i][i] = 0.0;
    for (const auto& a : arcs) {
        if (a.from >= 0 && a.from < numNodes && a.to >= 0 && a.to < numNodes)
            dist[a.from][a.to] = qMin(dist[a.from][a.to], a.cost);
    }

    for (int k = 0; k < numNodes; ++k)
        for (int i = 0; i < numNodes; ++i)
            for (int j = 0; j < numNodes; ++j)
                if (dist[i][k] + dist[k][j] < dist[i][j])
                    dist[i][j] = dist[i][k] + dist[k][j];

    return dist;
}

/* ---- Min-cost flow (greedy matching) ---- */

QVector<ChinesePostman2::Arc> ChinesePostman2::minCostFlow(
    const QVector<int>& balance, const QVector<QVector<double>>& dist) const
{
    int n = balance.size();
    QVector<Arc> extra;

    // Separate surplus (out > in) and deficit (in > out) nodes
    QVector<QPair<int, int>> surplus, deficit;
    for (int i = 0; i < n; ++i) {
        if (balance[i] > 0) surplus.append({i, balance[i]});
        else if (balance[i] < 0) deficit.append({i, -balance[i]});
    }

    // Greedy matching: pair surplus with nearest deficit
    QVector<int> supply = surplus.size() > 0 ?
        QVector<int>(surplus.size(), 0) : QVector<int>();
    for (int i = 0; i < surplus.size(); ++i) supply[i] = surplus[i].second;
    QVector<int> demand = deficit.size() > 0 ?
        QVector<int>(deficit.size(), 0) : QVector<int>();
    for (int j = 0; j < deficit.size(); ++j) demand[j] = deficit[j].second;

    for (int i = 0; i < surplus.size() && supply[i] > 0; ++i) {
        // Find cheapest deficit
        while (supply[i] > 0) {
            int bestJ = -1;
            double bestCost = std::numeric_limits<double>::max();
            for (int j = 0; j < deficit.size(); ++j) {
                if (demand[j] <= 0) continue;
                int si = surplus[i].first, dj = deficit[j].first;
                if (dist[si][dj] < bestCost) {
                    bestCost = dist[si][dj];
                    bestJ = j;
                }
            }
            if (bestJ < 0) break;

            int flow = qMin(supply[i], demand[bestJ]);
            Arc a;
            a.from = surplus[i].first;
            a.to = deficit[bestJ].first;
            a.cost = bestCost;
            a.multiplicity = flow;
            extra.append(a);

            supply[i] -= flow;
            demand[bestJ] -= flow;
        }
    }

    return extra;
}

/* ---- Euler tour (Hierholzer) ---- */

QVector<int> ChinesePostman2::eulerTour(int numNodes, const QVector<Arc>& arcs,
                                          const QVector<Arc>& extraArcs) const
{
    // Build multigraph adjacency with available edge counts
    QVector<QVector<QPair<int, int>>> adj(numNodes); // (to, count)
    for (const auto& a : arcs) {
        adj[a.from].append({a.to, 1});
    }
    for (const auto& a : extraArcs) {
        for (int m = 0; m < a.multiplicity; ++m)
            adj[a.from].append({a.to, 1});
    }

    // Find starting node with non-zero degree
    int start = 0;
    for (int i = 0; i < numNodes; ++i) {
        if (!adj[i].isEmpty()) { start = i; break; }
    }

    QVector<int> tour;
    QVector<int> stack;
    stack.append(start);

    while (!stack.isEmpty()) {
        int v = stack.back();
        if (adj[v].isEmpty()) {
            tour.append(v);
            stack.removeLast();
        } else {
            auto edge = adj[v].takeLast();
            stack.append(edge.first);
        }
    }

    std::reverse(tour.begin(), tour.end());
    return tour;
}

/* ---- Solve ---- */

QVector<int> ChinesePostman2::solve(int numNodes, const QVector<Arc>& arcs)
{
    QElapsedTimer timer;
    timer.start();

    if (numNodes <= 0 || arcs.isEmpty()) return {};

    // Step 1: Compute arc balance
    auto balance = arcBalance(numNodes, arcs);

    // Step 2: Check if Eulerian (all balanced)
    bool isEulerian = true;
    for (int b : balance)
        if (b != 0) { isEulerian = false; break; }

    // Step 3: Compute shortest paths
    auto dist = allPairsShortest(numNodes, arcs);

    // Step 4: Min-cost flow to balance
    QVector<Arc> extraArcs;
    double extraCost = 0.0;
    if (!isEulerian) {
        extraArcs = minCostFlow(balance, dist);
        for (const auto& a : extraArcs)
            extraCost += a.cost * a.multiplicity;
    }

    // Step 5: Compute total cost
    double baseCost = 0.0;
    for (const auto& a : arcs) baseCost += a.cost;
    m_tourCost = baseCost + extraCost;

    // Step 6: Extract Euler tour
    auto tour = eulerTour(numNodes, arcs, extraArcs);

    m_stats.totalSolves++;
    m_stats.numNodes = numNodes;
    m_stats.numArcs = arcs.size();
    m_stats.totalCost = m_tourCost;
    m_stats.extraArcs = extraArcs.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(numNodes, extraArcs.size(), m_tourCost, timer.elapsed());
    return tour;
}

/* ---- Accessors ---- */

double ChinesePostman2::tourCost() const { return m_tourCost; }

/* ---- Reset ---- */

void ChinesePostman2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_tourCost = 0.0;
}
