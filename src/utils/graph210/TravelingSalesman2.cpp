/**
 * @file TravelingSalesman2.cpp
 * @brief TravelingSalesman2 实现
 *
 * 实现TSP Christofides算法：MST、奇度顶点匹配、欧拉回路、2-opt优化。
 */

#include "utils/graph210/TravelingSalesman2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

TravelingSalesman2::TravelingSalesman2(QObject *parent) : QObject(parent) {}
TravelingSalesman2::~TravelingSalesman2() = default;

/* ---- Configuration ---- */

void TravelingSalesman2::setDistanceMatrix(const QVector<QVector<double>>& dist)
{
    m_n = dist.size();
    m_dist = dist;
}

void TravelingSalesman2::setCities(const QVector<QVector<double>>& coords)
{
    m_n = coords.size();
    m_dist.resize(m_n);
    for (int i = 0; i < m_n; ++i) {
        m_dist[i].resize(m_n, 0.0);
        for (int j = 0; j < m_n; ++j) {
            if (i == j) continue;
            double dx = coords[i][0] - coords[j][0];
            double dy = (coords[i].size() > 1 && coords[j].size() > 1)
                        ? coords[i][1] - coords[j][1] : 0.0;
            m_dist[i][j] = qSqrt(dx * dx + dy * dy);
        }
    }
}

/* ---- MST via Prim ---- */

QVector<QVector<int>> TravelingSalesman2::minimumSpanningTree() const
{
    QVector<QVector<int>> mst(m_n);
    QVector<double> minDist(m_n, std::numeric_limits<double>::max());
    QVector<bool> inMST(m_n, false);
    QVector<int> parent(m_n, -1);

    minDist[0] = 0.0;
    for (int iter = 0; iter < m_n; ++iter) {
        int u = -1;
        double best = std::numeric_limits<double>::max();
        for (int i = 0; i < m_n; ++i) {
            if (!inMST[i] && minDist[i] < best) { best = minDist[i]; u = i; }
        }
        if (u < 0) break;
        inMST[u] = true;
        if (parent[u] >= 0) {
            mst[parent[u]].append(u);
            mst[u].append(parent[u]);
        }
        for (int v = 0; v < m_n; ++v) {
            if (!inMST[v] && u < m_dist.size() && v < m_dist[u].size()
                && m_dist[u][v] < minDist[v]) {
                minDist[v] = m_dist[u][v];
                parent[v] = u;
            }
        }
    }
    return mst;
}

/* ---- Odd-degree vertices ---- */

QVector<int> TravelingSalesman2::oddDegreeVertices(const QVector<QVector<int>>& mst) const
{
    QVector<int> odd;
    for (int i = 0; i < mst.size(); ++i)
        if (mst[i].size() % 2 == 1) odd.append(i);
    return odd;
}

/* ---- Minimum-weight perfect matching (greedy) ---- */

QVector<QPair<int,int>> TravelingSalesman2::minWeightMatching(
    const QVector<int>& oddVertices) const
{
    QVector<QPair<int,int>> matching;
    int m = oddVertices.size();
    if (m < 2) return matching;

    QVector<bool> used(m, false);
    // Greedy: pair closest remaining vertices
    for (int i = 0; i < m; ++i) {
        if (used[i]) continue;
        double bestDist = std::numeric_limits<double>::max();
        int bestJ = -1;
        for (int j = i + 1; j < m; ++j) {
            if (used[j]) continue;
            int vi = oddVertices[i], vj = oddVertices[j];
            double d = (vi < m_dist.size() && vj < m_dist[vi].size())
                       ? m_dist[vi][vj] : 1e9;
            if (d < bestDist) { bestDist = d; bestJ = j; }
        }
        if (bestJ >= 0) {
            matching.append({oddVertices[i], oddVertices[bestJ]});
            used[i] = true;
            used[bestJ] = true;
        }
    }
    return matching;
}

/* ---- Eulerian tour (Hierholzer) ---- */

QVector<int> TravelingSalesman2::eulerianTour(
    const QVector<QVector<int>>& multigraph) const
{
    QVector<int> tour;
    if (multigraph.isEmpty()) return tour;

    // Build adjacency with edge tracking
    QVector<QVector<int>> adj = multigraph;
    QVector<int> stack;
    stack.append(0);

    while (!stack.isEmpty()) {
        int v = stack.back();
        if (!adj[v].isEmpty()) {
            int u = adj[v].back();
            adj[v].removeLast();
            // Remove reverse edge
            for (int i = 0; i < adj[u].size(); ++i) {
                if (adj[u][i] == v) { adj[u].removeAt(i); break; }
            }
            stack.append(u);
        } else {
            tour.append(v);
            stack.removeLast();
        }
    }

    std::reverse(tour.begin(), tour.end());
    return tour;
}

/* ---- Shortcut to Hamiltonian ---- */

QVector<int> TravelingSalesman2::shortcutTour(const QVector<int>& euler) const
{
    QVector<int> tour;
    QVector<bool> visited(m_n, false);
    for (int v : euler) {
        if (!visited[v]) {
            visited[v] = true;
            tour.append(v);
        }
    }
    return tour;
}

/* ---- Solve ---- */

QVector<int> TravelingSalesman2::solve()
{
    QElapsedTimer timer;
    timer.start();

    if (m_n <= 1) return {0};
    if (m_n == 2) return {0, 1};

    // Step 1: MST
    auto mst = minimumSpanningTree();

    // Step 2: Find odd-degree vertices
    auto odd = oddDegreeVertices(mst);

    // Step 3: Minimum-weight perfect matching on odd vertices
    auto matching = minWeightMatching(odd);

    // Step 4: Combine MST + matching into multigraph
    QVector<QVector<int>> multi = mst;
    for (const auto& edge : matching) {
        multi[edge.first].append(edge.second);
        multi[edge.second].append(edge.first);
    }

    // Step 5: Eulerian tour
    auto euler = eulerianTour(multi);

    // Step 6: Shortcut to Hamiltonian
    auto tour = shortcutTour(euler);

    // Step 7: 2-opt improvement
    tour = twoOpt(tour);

    double cost = tourCost(tour);
    double mstCost = lowerBound();

    m_stats.totalSolves++;
    m_stats.numCities = m_n;
    m_stats.tourCost = cost;
    m_stats.approxRatio = (mstCost > 0) ? cost / mstCost : 0.0;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(m_n, cost, timer.elapsed());
    return tour;
}

/* ---- Tour cost ---- */

double TravelingSalesman2::tourCost(const QVector<int>& tour) const
{
    double cost = 0.0;
    for (int i = 0; i < tour.size(); ++i) {
        int j = (i + 1) % tour.size();
        int vi = tour[i], vj = tour[j];
        if (vi < m_dist.size() && vj < m_dist[vi].size())
            cost += m_dist[vi][vj];
    }
    return cost;
}

/* ---- 2-opt ---- */

QVector<int> TravelingSalesman2::twoOpt(const QVector<int>& tour) const
{
    if (tour.size() < 4) return tour;
    QVector<int> best = tour;
    double bestCost = tourCost(best);
    bool improved = true;
    int maxIter = 100;

    while (improved && maxIter-- > 0) {
        improved = false;
        for (int i = 0; i < best.size() - 1; ++i) {
            for (int j = i + 2; j < best.size(); ++j) {
                QVector<int> candidate = best;
                std::reverse(candidate.begin() + i + 1, candidate.begin() + j + 1);
                double c = tourCost(candidate);
                if (c < bestCost) {
                    best = candidate;
                    bestCost = c;
                    improved = true;
                }
            }
        }
    }
    return best;
}

/* ---- Lower bound (MST cost) ---- */

double TravelingSalesman2::lowerBound() const
{
    auto mst = minimumSpanningTree();
    double cost = 0.0;
    for (int i = 0; i < m_n; ++i)
        for (int j : mst[i])
            if (i < m_dist.size() && j < m_dist[i].size() && i < j)
                cost += m_dist[i][j];
    return cost;
}

/* ---- Reset ---- */

void TravelingSalesman2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_n = 0;
    m_dist.clear();
}
