/**
 * @file HamiltonianCycle5.cpp
 * @brief HamiltonianCycle5 实现
 *
 * 实现哈密顿回路：Held-Karp动态规划与位掩码状态表示。
 */

#include "utils/graph266/HamiltonianCycle5.h"

#include <QElapsedTimer>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

HamiltonianCycle5::HamiltonianCycle5(QObject *parent) : QObject(parent) {}
HamiltonianCycle5::~HamiltonianCycle5() = default;

/* ---- Popcount helper ---- */

int HamiltonianCycle5::popcount(int mask)
{
    int count = 0;
    while (mask) { count += mask & 1; mask >>= 1; }
    return count;
}

/* ---- Set adjacency matrix ---- */

void HamiltonianCycle5::setGraph(const QVector<QVector<double>>& adjMatrix)
{
    m_n = adjMatrix.size();
    m_adj = adjMatrix;

    int edges = 0;
    for (int i = 0; i < m_n; ++i)
        for (int j = i + 1; j < m_n; ++j)
            if (m_adj[i][j] > 0 || m_adj[j][i] > 0) edges++;

    m_stats.numVertices = m_n;
    m_stats.numEdges = edges;
}

/* ---- Held-Karp DP ---- */

void HamiltonianCycle5::heldKarp()
{
    int n = m_n;
    int fullMask = (1 << n) - 1;

    // dp[mask][v] = minimum cost to visit set mask ending at vertex v
    int numMasks = 1 << n;
    m_dp.resize(numMasks);
    m_parent.resize(numMasks);
    for (int mask = 0; mask < numMasks; ++mask) {
        m_dp[mask].resize(n, std::numeric_limits<double>::max());
        m_parent[mask].resize(n, -1);
    }

    // Base case: start at vertex 0
    m_dp[1][0] = 0.0;

    // Fill DP table
    for (int mask = 1; mask <= fullMask; ++mask) {
        if (!(mask & 1)) continue; // Must include vertex 0

        for (int v = 0; v < n; ++v) {
            if (!(mask & (1 << v))) continue;
            if (m_dp[mask][v] >= std::numeric_limits<double>::max() / 2) continue;

            for (int u = 0; u < n; ++u) {
                if (mask & (1 << u)) continue; // u not yet visited
                if (m_adj[v][u] <= 0) continue; // No edge

                int newMask = mask | (1 << u);
                double newCost = m_dp[mask][v] + m_adj[v][u];
                if (newCost < m_dp[newMask][u]) {
                    m_dp[newMask][u] = newCost;
                    m_parent[newMask][u] = v;
                }
            }
        }
        m_stats.numDPStates++;
    }

    // Find best returning edge to vertex 0
    m_bestWeight = std::numeric_limits<double>::max();
    int bestLast = -1;

    for (int v = 1; v < n; ++v) {
        if (m_dp[fullMask][v] >= std::numeric_limits<double>::max() / 2) continue;
        if (m_adj[v][0] <= 0) continue; // No return edge

        double totalCost = m_dp[fullMask][v] + m_adj[v][0];
        if (totalCost < m_bestWeight) {
            m_bestWeight = totalCost;
            bestLast = v;
        }
    }

    // Update parent for return edge
    if (bestLast >= 0)
        m_parent[fullMask | 1][0] = bestLast;
}

/* ---- Reconstruct path ---- */

QVector<int> HamiltonianCycle5::reconstructPath() const
{
    int n = m_n;
    int fullMask = (1 << n) - 1;

    // Find ending vertex
    int last = -1;
    double bestCost = std::numeric_limits<double>::max();
    for (int v = 1; v < n; ++v) {
        if (m_dp[fullMask][v] >= std::numeric_limits<double>::max() / 2) continue;
        if (m_adj[v][0] <= 0) continue;
        double cost = m_dp[fullMask][v] + m_adj[v][0];
        if (cost < bestCost) { bestCost = cost; last = v; }
    }

    if (last < 0) return {};

    // Backtrace through parent table
    QVector<int> path;
    int mask = fullMask;
    int v = last;

    while (v != -1) {
        path.prepend(v);
        int prev = m_parent[mask][v];
        mask ^= (1 << v);
        v = prev;
    }

    path.prepend(0); // Complete cycle
    return path;
}

/* ---- Find minimum weight Hamiltonian cycle ---- */

QVector<int> HamiltonianCycle5::findMinCycle()
{
    QElapsedTimer timer;
    timer.start();

    if (m_n < 2) return {};

    heldKarp();
    m_bestPath = reconstructPath();

    m_stats.minCycleWeight = m_bestWeight;
    m_stats.numHamiltonianCycles = m_bestPath.isEmpty() ? 0 : 1;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit cycleFound(m_n, m_bestWeight, timer.elapsed());
    return m_bestPath;
}

/* ---- Check if Hamiltonian cycle exists ---- */

bool HamiltonianCycle5::hasCycle() const
{
    if (m_n < 3) return false;
    if (m_dp.isEmpty()) return false;

    int fullMask = (1 << m_n) - 1;
    for (int v = 0; v < m_n; ++v) {
        if (m_dp[fullMask][v] < std::numeric_limits<double>::max() / 2
            && m_adj[v][0] > 0)
            return true;
    }
    return false;
}

/* ---- Get cycle weight ---- */

double HamiltonianCycle5::cycleWeight() const { return m_bestWeight; }

/* ---- Reset ---- */

void HamiltonianCycle5::resetStatistics()
{
    m_adj.clear();
    m_bestPath.clear();
    m_dp.clear();
    m_parent.clear();
    m_bestWeight = 0.0;
    m_n = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
