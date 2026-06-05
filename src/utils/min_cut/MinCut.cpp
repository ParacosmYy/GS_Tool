/**
 * @file MinCut.cpp
 * @brief 最小割实现 — Stoer-Wagner全局最小割算法
 */

#include "utils/min_cut/MinCut.h"

#include <QElapsedTimer>
#include <cmath>
#include <limits>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
MinCut::MinCut(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 计算全局最小割权重
 * @param adjacency 邻接矩阵(adjacency[i][j]为边权重)
 * @return 最小割的边总权重
 *
 * Stoer-Wagner算法: 执行V-1轮，每轮通过最大权重扩张
 * 找到当前图的s-t最小割(最后加入的顶点t与倒数第二个s之间的割)，
 * 然后合并s和t继续迭代。全局最小割为所有s-t割中的最小值。
 */
double MinCut::compute(const QVector<QVector<double>>& adjacency)
{
    QElapsedTimer timer;
    timer.start();

    int n = adjacency.size();
    if (n < 2) {
        m_timeSumMs += timer.nsecsElapsed() / 1e6;
        ++m_stats.totalComputed;
        m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalComputed;
        emit computationCompleted(0.0);
        return 0.0;
    }

    /* 拷贝邻接矩阵(算法会修改它) */
    QVector<QVector<double>> graph = adjacency;

    /* 顶点合并标记: merged[i] = j 表示i已合并到j */
    QVector<int> merged(n);
    for (int i = 0; i < n; ++i) merged[i] = i;

    /* 每个超级顶点包含的原始顶点 */
    QVector<QVector<int>> components(n);
    for (int i = 0; i < n; ++i) components[i].append(i);

    double bestCut = std::numeric_limits<double>::max();
    QVector<int> bestPartition;

    /* 逐轮收缩顶点 */
    for (int phase = 0; phase < n - 1; ++phase) {
        /* 有效顶点列表 */
        QVector<int> active;
        for (int i = 0; i < n; ++i) {
            if (merged[i] == i) active.append(i);
        }

        if (active.size() < 2) break;

        /* 最大权重扩张 */
        QVector<bool> inSet(n, false);
        QVector<double> weights(n, 0.0);

        int prev = -1, last = -1;

        for (int step = 0; step < active.size(); ++step) {
            /* 选择不在集合中、权重最大的顶点 */
            int bestIdx = -1;
            double bestWeight = -1.0;
            for (int v : active) {
                if (!inSet[v] && weights[v] > bestWeight) {
                    bestWeight = weights[v];
                    bestIdx = v;
                }
            }

            if (bestIdx < 0) break;

            inSet[bestIdx] = true;
            prev = last;
            last = bestIdx;

            /* 更新邻居权重 */
            for (int v : active) {
                if (!inSet[v]) {
                    weights[v] += graph[bestIdx][v];
                }
            }
        }

        /* 最后加入的last与倒数第二个prev之间的s-t割 */
        double cutWeight = 0.0;
        for (int v : active) {
            if (v != last) {
                cutWeight += graph[last][v];
            }
        }

        if (cutWeight < bestCut) {
            bestCut = cutWeight;
            bestPartition = components[last];
        }

        /* 合并 last 到 prev */
        merged[last] = prev;
        components[prev].append(components[last]);
        components[last].clear();

        /* 更新邻接矩阵 */
        for (int v = 0; v < n; ++v) {
            if (v != last && v != prev) {
                graph[prev][v] += graph[last][v];
                graph[v][prev] += graph[v][last];
            }
            graph[last][v] = 0.0;
            graph[v][last] = 0.0;
        }
    }

    m_partition = bestPartition;

    /* 更新统计 */
    double elapsed = timer.nsecsElapsed() / 1e6;
    m_timeSumMs += elapsed;
    ++m_stats.totalComputed;
    m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalComputed;

    emit computationCompleted(bestCut);
    return bestCut;
}

/**
 * @brief 获取最小割的划分结果
 * @return 第一个分区的顶点索引列表
 */
QVector<int> MinCut::cutPartition() const
{
    return m_partition;
}

/** @brief 重置统计信息 */
void MinCut::resetStatistics()
{
    m_stats = Stats{};
    m_timeSumMs = 0.0;
    m_partition.clear();
}
