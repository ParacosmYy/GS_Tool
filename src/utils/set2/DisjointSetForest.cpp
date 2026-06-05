/**
 * @file DisjointSetForest.cpp
 * @brief 并查集森林实现
 */

#include "DisjointSetForest.h"
#include <QElapsedTimer>
#include <QMap>

DisjointSetForest::DisjointSetForest(int n, QObject* parent)
    : QObject(parent)
    , m_parent(n)
    , m_rank(n, 0)
    , m_setCount(n)
    , m_timeSum(0.0)
{
    for (int i = 0; i < n; ++i)
        m_parent[i] = i;
}

int DisjointSetForest::addElement()
{
    int id = m_parent.size();
    m_parent.append(id);
    m_rank.append(0);
    m_setCount++;
    return id;
}

int DisjointSetForest::find(int x) const
{
    QElapsedTimer timer;
    timer.start();

    /* 路径压缩 */
    int result = x;
    while (m_parent[result] != result)
        result = m_parent[result];

    /* 第二遍压缩路径 */
    while (m_parent[x] != x) {
        int next = m_parent[x];
        m_parent[x] = result;
        x = next;
    }

    m_stats.totalFinds++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalUnions + m_stats.totalFinds;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    return result;
}

bool DisjointSetForest::unionSets(int x, int y)
{
    QElapsedTimer timer;
    timer.start();

    int rootX = find(x);
    int rootY = find(y);

    if (rootX == rootY) return false;

    /* 按秩合并 */
    if (m_rank[rootX] < m_rank[rootY]) {
        m_parent[rootX] = rootY;
    } else if (m_rank[rootX] > m_rank[rootY]) {
        m_parent[rootY] = rootX;
    } else {
        m_parent[rootY] = rootX;
        m_rank[rootX]++;
    }

    m_setCount--;

    m_stats.totalUnions++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalUnions + m_stats.totalFinds;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    emit setsUnited(rootX, rootY, m_setCount);
    return true;
}

bool DisjointSetForest::isConnected(int x, int y) const
{
    return find(x) == find(y);
}

int DisjointSetForest::setCount() const { return m_setCount; }

QVector<QVector<int>> DisjointSetForest::groups() const
{
    QMap<int, QVector<int>> groupMap;
    for (int i = 0; i < m_parent.size(); ++i) {
        int root = find(i);
        groupMap[root].append(i);
    }
    return groupMap.values().toVector();
}

int DisjointSetForest::setSize(int x) const
{
    int root = find(x);
    int count = 0;
    for (int i = 0; i < m_parent.size(); ++i) {
        if (find(i) == root) count++;
    }
    return count;
}

int DisjointSetForest::size() const { return m_parent.size(); }

void DisjointSetForest::reset(int n)
{
    m_parent.resize(n);
    m_rank.resize(n);
    m_rank.fill(0);
    for (int i = 0; i < n; ++i)
        m_parent[i] = i;
    m_setCount = n;
}

DisjointSetForest::Stats DisjointSetForest::stats() const { return m_stats; }

void DisjointSetForest::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
