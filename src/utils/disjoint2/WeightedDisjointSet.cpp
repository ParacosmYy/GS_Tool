/**
 * @file WeightedDisjointSet.cpp
 * @brief 加权并查集实现
 */

#include "WeightedDisjointSet.h"
#include <QElapsedTimer>
#include <algorithm>

WeightedDisjointSet::WeightedDisjointSet(int n, QObject* parent)
    : QObject(parent)
    , m_n(n)
    , m_components(n)
    , m_timeSum(0.0)
{
    m_parent.resize(n);
    m_rank.resize(n, 0);
    m_size.resize(n, 1);
    m_weight.resize(n, 0.0);
    m_sumWeight.resize(n, 0.0);
    m_maxWeight.resize(n, 0.0);
    for (int i = 0; i < n; ++i)
        m_parent[i] = i;
}

int WeightedDisjointSet::find(int x)
{
    QElapsedTimer timer;
    timer.start();

    if (x < 0 || x >= m_n) return -1;

    /* 路径压缩 */
    int root = x;
    while (m_parent[root] != root)
        root = m_parent[root];

    while (m_parent[x] != root) {
        int next = m_parent[x];
        m_parent[x] = root;
        x = next;
    }

    m_stats.totalFinds++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalUnions + m_stats.totalFinds;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    return root;
}

int WeightedDisjointSet::unite(int x, int y)
{
    QElapsedTimer timer;
    timer.start();

    int rx = find(x), ry = find(y);
    if (rx < 0 || ry < 0) return -1;
    if (rx == ry) return rx;

    /* 按秩合并 */
    if (m_rank[rx] < m_rank[ry]) std::swap(rx, ry);
    m_parent[ry] = rx;
    m_size[rx] += m_size[ry];
    m_sumWeight[rx] += m_sumWeight[ry];
    m_maxWeight[rx] = std::max(m_maxWeight[rx], m_maxWeight[ry]);

    if (m_rank[rx] == m_rank[ry]) m_rank[rx]++;
    m_components--;

    m_stats.totalUnions++;
    m_stats.totalComponents = m_components;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalUnions + m_stats.totalFinds;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    emit unionCompleted(x, y, rx);
    return rx;
}

bool WeightedDisjointSet::connected(int x, int y)
{
    return find(x) == find(y);
}

int WeightedDisjointSet::componentSize(int x)
{
    int r = find(x);
    return (r >= 0) ? m_size[r] : 0;
}

int WeightedDisjointSet::componentCount() const
{
    return m_components;
}

void WeightedDisjointSet::setWeight(int x, double w)
{
    if (x < 0 || x >= m_n) return;
    int r = find(x);
    m_sumWeight[r] += w - m_weight[x];
    m_maxWeight[r] = std::max(m_maxWeight[r], w);
    m_weight[x] = w;
}

double WeightedDisjointSet::componentWeight(int x)
{
    int r = find(x);
    return (r >= 0) ? m_sumWeight[r] : 0.0;
}

double WeightedDisjointSet::componentMaxWeight(int x)
{
    int r = find(x);
    return (r >= 0) ? m_maxWeight[r] : 0.0;
}

void WeightedDisjointSet::reset(int n)
{
    m_n = n;
    m_components = n;
    m_parent.resize(n);
    m_rank.resize(n, 0);
    m_size.resize(n, 1);
    m_weight.resize(n, 0.0);
    m_sumWeight.resize(n, 0.0);
    m_maxWeight.resize(n, 0.0);
    for (int i = 0; i < n; ++i)
        m_parent[i] = i;
}

WeightedDisjointSet::Stats WeightedDisjointSet::stats() const { return m_stats; }

void WeightedDisjointSet::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
