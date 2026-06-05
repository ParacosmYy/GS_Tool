/**
 * @file DisjointSet.cpp
 * @brief 并查集实现 — 加权路径压缩
 */

#include "utils/disjoint/DisjointSet.h"

#include <QElapsedTimer>

DisjointSet::DisjointSet(QObject* parent)
    : QObject(parent), m_size(0), m_componentCount(0), m_timeSum(0.0) {}

void DisjointSet::makeSet(int n)
{
    m_parent.resize(n);
    m_rank.resize(n, 0);
    m_sz.resize(n, 1);
    for (int i = 0; i < n; ++i) m_parent[i] = i;
    m_size = n;
    m_componentCount = n;
}

int DisjointSet::find(int x)
{
    QElapsedTimer timer;
    timer.start();

    if (x < 0 || x >= m_size) return -1;

    /* 路径压缩 */
    int root = x;
    while (m_parent[root] != root) root = m_parent[root];
    while (m_parent[x] != x) {
        int next = m_parent[x];
        m_parent[x] = root;
        x = next;
    }

    m_stats.totalFinds++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        (m_stats.totalUnions + m_stats.totalFinds > 0)
        ? m_timeSum / (m_stats.totalUnions + m_stats.totalFinds) : 0.0;
    return root;
}

bool DisjointSet::unite(int a, int b)
{
    QElapsedTimer timer;
    timer.start();

    int rootA = find(a);
    int rootB = find(b);
    if (rootA == rootB) return false;

    /* 按秩合并 */
    if (m_rank[rootA] < m_rank[rootB]) {
        m_parent[rootA] = rootB;
        m_sz[rootB] += m_sz[rootA];
    } else if (m_rank[rootA] > m_rank[rootB]) {
        m_parent[rootB] = rootA;
        m_sz[rootA] += m_sz[rootB];
    } else {
        m_parent[rootB] = rootA;
        m_sz[rootA] += m_sz[rootB];
        m_rank[rootA]++;
    }
    m_componentCount--;

    emit componentsUnited(rootA, rootB);

    m_stats.totalUnions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        (m_stats.totalUnions + m_stats.totalFinds > 0)
        ? m_timeSum / (m_stats.totalUnions + m_stats.totalFinds) : 0.0;
    return true;
}

bool DisjointSet::connected(int a, int b)
{
    return find(a) == find(b);
}

int DisjointSet::componentSize(int x)
{
    int root = find(x);
    if (root < 0) return 0;
    return m_sz[root];
}

void DisjointSet::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
