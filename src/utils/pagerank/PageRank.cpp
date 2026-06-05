/**
 * @file PageRank.cpp
 * @brief PageRank算法 — 图节点重要性排序
 */

#include "PageRank.h"
#include <QElapsedTimer>
#include <cmath>

PageRank::PageRank(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

void PageRank::addEdge(int from, int to)
{
    m_outLinks[from].append(to);
    m_outDegree[from]++;
    /* 确保目标节点也在图中 */
    if (!m_outLinks.contains(to)) {
        m_outLinks[to] = QVector<int>();
        m_outDegree[to] = 0;
    }
}

void PageRank::addEdges(const QVector<QPair<int, int>>& edges)
{
    for (const auto& edge : edges) {
        addEdge(edge.first, edge.second);
    }
}

QMap<int, double> PageRank::compute(double damping, int maxIterations, double tolerance)
{
    QElapsedTimer timer;
    timer.start();

    int N = nodeCount();
    if (N == 0) return {};

    /* 收集所有节点 */
    QList<int> nodes = m_outLinks.keys();
    QMap<int, int> nodeIndex;
    for (int i = 0; i < nodes.size(); ++i) {
        nodeIndex[nodes[i]] = i;
    }

    /* 构建入链表 */
    QMap<int, QVector<int>> inLinks;
    for (int node : nodes) {
        for (int target : m_outLinks[node]) {
            inLinks[target].append(node);
        }
    }

    /* 初始化rank值 */
    QMap<int, double> ranks;
    double initVal = 1.0 / N;
    for (int node : nodes) {
        ranks[node] = initVal;
    }

    /* 迭代 */
    int iterations = 0;
    double convergence = 0.0;

    for (int iter = 0; iter < maxIterations; ++iter) {
        QMap<int, double> newRanks;

        /* 悬挂节点(dangling node)贡献 */
        double danglingSum = 0.0;
        for (int node : nodes) {
            if (m_outDegree[node] == 0) {
                danglingSum += ranks[node];
            }
        }

        for (int node : nodes) {
            double sum = 0.0;
            for (int src : inLinks[node]) {
                sum += ranks[src] / m_outDegree[src];
            }
            newRanks[node] = (1.0 - damping) / N +
                             damping * (sum + danglingSum / N);
        }

        /* 收敛检测 */
        double diff = 0.0;
        for (int node : nodes) {
            diff += std::abs(newRanks[node] - ranks[node]);
        }

        ranks = newRanks;
        iterations++;
        convergence = diff;

        if (diff < tolerance) break;
    }

    m_stats.totalComputations++;
    m_stats.totalIterations += iterations;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit computationCompleted(iterations, convergence);
    return ranks;
}

QVector<QPair<int, double>> PageRank::topK(const QMap<int, double>& ranks, int k) const
{
    QVector<QPair<int, double>> sorted;
    for (auto it = ranks.constBegin(); it != ranks.constEnd(); ++it) {
        sorted.append({it.key(), it.value()});
    }

    std::sort(sorted.begin(), sorted.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    if (sorted.size() > k) sorted.resize(k);
    return sorted;
}

int PageRank::edgeCount() const
{
    int count = 0;
    for (auto it = m_outDegree.constBegin(); it != m_outDegree.constEnd(); ++it) {
        count += it.value();
    }
    return count;
}

void PageRank::clear()
{
    m_outLinks.clear();
    m_outDegree.clear();
}

void PageRank::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
