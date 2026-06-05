/**
 * @file MinimumCut.cpp
 * @brief Karger随机化最小割算法实现
 */

#include "MinimumCut.h"

#include <QElapsedTimer>
#include <QRandomGenerator>
#include <algorithm>
#include <cmath>

/* ---------- 构造函数 ---------- */

MinimumCut::MinimumCut(QObject* parent)
    : QObject(parent)
{
}

/* ---------- 无权Karger最小割 ---------- */

MinimumCut::CutResult MinimumCut::findMinCut(
    int numVertices,
    const QVector<QPair<int, int>>& edges,
    int iterations) const
{
    QElapsedTimer timer;
    timer.start();

    CutResult best;
    if (numVertices < 2 || edges.isEmpty()) {
        best.valid = false;
        return best;
    }

    /* 自动迭代次数: n^2 * ln(n) 保证高概率 */
    int n = numVertices;
    int autoIter = static_cast<int>(n * n * std::log(std::max(n, 2)));
    int iterCount = (iterations > 0) ? iterations : autoIter;

    best.cutSize = edges.size() + 1; /* 初始化为大值 */
    best.valid = true;

    for (int i = 0; i < iterCount; ++i) {
        CutResult result = singleContraction(numVertices, edges);
        if (result.valid && result.cutSize < best.cutSize) {
            best = result;
        }
        m_stats.totalContractions += (n - 2);
    }

    m_stats.totalRuns += iterCount;
    m_stats.totalEdges += edges.size() * iterCount;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalRuns > 0)
        ? m_timeSum / m_stats.totalRuns : 0.0;

    emit cutFound(best.cutSize, iterCount);
    return best;
}

/* ---------- 带权Karger最小割 ---------- */

MinimumCut::CutResult MinimumCut::findMinCutWeighted(
    int numVertices,
    const QVector<WeightedEdge>& edges,
    int iterations) const
{
    QElapsedTimer timer;
    timer.start();

    CutResult best;
    if (numVertices < 2 || edges.isEmpty()) {
        best.valid = false;
        return best;
    }

    int n = numVertices;
    int autoIter = static_cast<int>(n * n * std::log(std::max(n, 2)));
    int iterCount = (iterations > 0) ? iterations : autoIter;

    best.cutSize = 0x7FFFFFFF;
    best.valid = true;

    for (int iter = 0; iter < iterCount; ++iter) {
        /* 构建带权边: 重复边按权重比例 */
        QVector<QPair<int, int>> flatEdges;
        QVector<double> weights;
        double totalWeight = 0.0;

        for (const auto& e : edges) {
            int cnt = std::max(1, static_cast<int>(e.third));
            for (int k = 0; k < cnt; ++k) {
                flatEdges.append({e.first, e.second});
                weights.append(e.third / cnt);
            }
            totalWeight += e.third;
        }

        /* 随机收缩到2个超级顶点 */
        QVector<int> parent(n), rank(n, 0);
        for (int i = 0; i < n; ++i) parent[i] = i;

        int components = n;
        int edgeIdx = 0;

        while (components > 2 && !flatEdges.isEmpty()) {
            int idx = QRandomGenerator::global()->bounded(flatEdges.size());
            int u = flatEdges[idx].first;
            int v = flatEdges[idx].second;

            int ru = findRoot(parent, u);
            int rv = findRoot(parent, v);

            if (ru != rv) {
                unionSets(parent, rank, ru, rv);
                components--;
                m_stats.totalContractions++;
            }

            /* 移除自环 */
            flatEdges.removeAt(idx);
            weights.removeAt(idx);

            /* 防止无限循环 */
            if (++edgeIdx > n * n * 10) break;
        }

        /* 计算割权重 */
        double cutWeight = 0.0;
        for (int i = 0; i < edges.size(); ++i) {
            int ru = findRoot(parent, edges[i].first);
            int rv = findRoot(parent, edges[i].second);
            if (ru != rv) {
                cutWeight += edges[i].third;
            }
        }

        if (cutWeight < best.cutSize) {
            best.cutSize = static_cast<int>(cutWeight + 0.5);
            best.partitions = extractPartitions(parent, n);
        }
    }

    m_stats.totalRuns += iterCount;
    m_stats.totalEdges += edges.size() * iterCount;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalRuns > 0)
        ? m_timeSum / m_stats.totalRuns : 0.0;

    emit cutFound(best.cutSize, iterCount);
    return best;
}

/* ---------- 单次收缩 ---------- */

MinimumCut::CutResult MinimumCut::singleContraction(
    int numVertices,
    const QVector<QPair<int, int>>& edges) const
{
    CutResult result;
    if (numVertices < 2 || edges.isEmpty()) {
        result.valid = false;
        return result;
    }

    int n = numVertices;
    QVector<int> parent(n), rankArr(n, 0);
    for (int i = 0; i < n; ++i) parent[i] = i;

    /* 复制边列表用于随机选择 */
    QVector<QPair<int, int>> remaining = edges;
    int components = n;

    while (components > 2) {
        if (remaining.isEmpty()) break;

        int idx = QRandomGenerator::global()->bounded(remaining.size());
        int u = remaining[idx].first;
        int v = remaining[idx].second;

        int ru = findRoot(parent, u);
        int rv = findRoot(parent, v);

        if (ru != rv) {
            unionSets(parent, rankArr, ru, rv);
            components--;
        }

        remaining.removeAt(idx);
    }

    /* 统计割边 */
    int cutSize = 0;
    for (const auto& edge : edges) {
        int ru = findRoot(parent, edge.first);
        int rv = findRoot(parent, edge.second);
        if (ru != rv) cutSize++;
    }

    result.cutSize = cutSize;
    result.partitions = extractPartitions(parent, n);
    result.valid = true;
    return result;
}

/* ---------- 并查集: 查找根 ---------- */

int MinimumCut::findRoot(QVector<int>& parent, int x) const
{
    while (parent[x] != x) {
        parent[x] = parent[parent[x]]; /* 路径压缩 */
        x = parent[x];
    }
    return x;
}

/* ---------- 并查集: 合并 ---------- */

void MinimumCut::unionSets(QVector<int>& parent, QVector<int>& rankArr,
                            int a, int b) const
{
    if (rankArr[a] < rankArr[b]) {
        parent[a] = b;
    } else if (rankArr[a] > rankArr[b]) {
        parent[b] = a;
    } else {
        parent[b] = a;
        rankArr[a]++;
    }
}

/* ---------- 提取分区 ---------- */

QVector<QSet<int>> MinimumCut::extractPartitions(
    const QVector<int>& parent, int numVertices) const
{
    QMap<int, QSet<int>> groups;
    QVector<int> p = parent; /* 可变副本用于路径压缩 */

    for (int i = 0; i < numVertices; ++i) {
        int root = i;
        while (p[root] != root) root = p[root];
        groups[root].insert(i);
    }

    QVector<QSet<int>> partitions;
    for (auto it = groups.begin(); it != groups.end(); ++it) {
        partitions.append(it.value());
    }
    return partitions;
}

/* ---------- 统计 ---------- */

MinimumCut::Stats MinimumCut::stats() const { return m_stats; }

void MinimumCut::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
