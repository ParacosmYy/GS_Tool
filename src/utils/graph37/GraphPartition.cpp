/**
 * @file GraphPartition.cpp
 * @brief 图分割实现 — Kernighan-Lin/FM细化/多层递归二分
 */

#include "utils/graph37/GraphPartition.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>
#include <limits>
#include <vector>

/** @brief 构造函数 @param parent 父对象 */
GraphPartition::GraphPartition(QObject* parent)
    : QObject(parent)
    , m_method(PartitionMethod::KernighanLin)
    , m_maxIter(50)
    , m_tolerance(0.05)
{
}

/** @brief 设置分割方法 @param method 方法 */
void GraphPartition::setMethod(PartitionMethod method)
{
    m_method = method;
}

/** @brief 设置最大迭代 @param maxIter 最大迭代 */
void GraphPartition::setMaxIterations(int maxIter)
{
    m_maxIter = qMax(1, maxIter);
}

/** @brief 设置平衡约束 @param tolerance 容忍度 */
void GraphPartition::setBalanceTolerance(double tolerance)
{
    m_tolerance = qBound(0.0, tolerance, 1.0);
}

/** @brief 执行图分割 @param numVertices 顶点数 @param edges 边列表 @return 分割结果 */
GraphPartition::PartitionResult GraphPartition::partition(
    int numVertices, const QList<Edge>& edges)
{
    QElapsedTimer timer;
    timer.start();

    PartitionResult result;
    if (numVertices < 2) {
        result.partition.resize(numVertices, 0);
        return result;
    }

    switch (m_method) {
    case PartitionMethod::KernighanLin:
        result = kernighanLin(numVertices, edges);
        break;
    case PartitionMethod::FiducciaMattheyses:
        result = fiducciaMattheyses(numVertices, edges);
        break;
    case PartitionMethod::MultilevelBisection:
        result = multilevelBisection(numVertices, edges);
        break;
    }

    double elapsed = timer.elapsed();
    m_stats.totalPartitions++;
    m_stats.totalVerticesProcessed += numVertices;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalPartitions);
    if (result.edgeCut < m_stats.bestEdgeCut || m_stats.bestEdgeCut == 0) {
        m_stats.bestEdgeCut = result.edgeCut;
    }

    emit partitionComplete(result.edgeCut, result.iterations);
    return result;
}

/** @brief 多路分割 @param numVertices 顶点数 @param edges 边列表 @param numParts 分区数 @return 分割结果 */
GraphPartition::PartitionResult GraphPartition::multiWayPartition(
    int numVertices, const QList<Edge>& edges, int numParts)
{
    PartitionResult result;
    result.partition.resize(numVertices, 0);

    if (numParts <= 1 || numVertices < 2) return result;

    /* 递归二分 */
    QList<int> activeVertices;
    for (int i = 0; i < numVertices; ++i) activeVertices.append(i);

    int currentPartId = 0;
    QList<QList<int>> queues;
    queues.append(activeVertices);

    while (queues.size() < numParts && !queues.isEmpty()) {
        QList<int> subset = queues.takeFirst();
        if (subset.size() <= 1) {
            for (int v : subset) result.partition[v] = currentPartId++;
            continue;
        }

        /* 提取子图边 */
        QSet<int> subsetSet(subset.begin(), subset.end());
        QList<Edge> subEdges;
        for (const auto& e : edges) {
            if (subsetSet.contains(e.source) && subsetSet.contains(e.target)) {
                subEdges.append(e);
            }
        }

        /* 重映射索引 */
        QMap<int, int> remap;
        for (int i = 0; i < subset.size(); ++i) remap[subset[i]] = i;
        QList<Edge> mappedEdges;
        for (const auto& e : subEdges) {
            Edge me;
            me.source = remap[e.source];
            me.target = remap[e.target];
            me.weight = e.weight;
            mappedEdges.append(me);
        }

        auto subResult = partition(subset.size(), mappedEdges);

        QList<int> left, right;
        for (int i = 0; i < subset.size(); ++i) {
            if (subResult.partition[i] == 0) left.append(subset[i]);
            else right.append(subset[i]);
        }

        queues.append(left);
        queues.append(right);
    }

    for (int i = 0; i < queues.size(); ++i) {
        for (int v : queues[i]) result.partition[v] = i;
    }

    result.edgeCut = computeEdgeCut(result.partition, edges);
    result.imbalance = computeImbalance(result.partition);
    return result;
}

/** @brief 重置统计 */
void GraphPartition::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief Kernighan-Lin双向分割 @param n 顶点数 @param edges 边列表 @return 分割结果 */
GraphPartition::PartitionResult GraphPartition::kernighanLin(
    int n, const QList<Edge>& edges)
{
    PartitionResult result;
    result.partition = initialBisection(n);

    /* 构建邻接表 */
    QVector<QVector<QPair<int, double>>> adj(n);
    for (const auto& e : edges) {
        if (e.source >= 0 && e.source < n && e.target >= 0 && e.target < n) {
            adj[e.source].append({e.target, e.weight});
            adj[e.target].append({e.source, e.weight});
        }
    }

    int bestCut = computeEdgeCut(result.partition, edges);
    QVector<int> bestPart = result.partition;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        /* 计算每个顶点的D值(外部代价-内部代价) */
        QVector<double> D(n, 0.0);
        for (int v = 0; v < n; ++v) {
            for (const auto& [u, w] : adj[v]) {
                if (result.partition[v] == result.partition[u]) {
                    D[v] -= w;
                } else {
                    D[v] += w;
                }
            }
        }

        /* 贪心交换 */
        QVector<bool> locked(n, false);
        int halfN = n / 2;
        QVector<QPair<int, int>> swaps;

        for (int step = 0; step < halfN; ++step) {
            double bestGain = -std::numeric_limits<double>::max();
            int bestA = -1, bestB = -1;

            for (int a = 0; a < n; ++a) {
                if (locked[a] || result.partition[a] != 0) continue;
                for (int b = 0; b < n; ++b) {
                    if (locked[b] || result.partition[b] != 1) continue;
                    double cab = 0.0;
                    for (const auto& [u, w] : adj[a]) {
                        if (u == b) { cab = w; break; }
                    }
                    double gain = D[a] + D[b] - 2.0 * cab;
                    if (gain > bestGain) {
                        bestGain = gain;
                        bestA = a;
                        bestB = b;
                    }
                }
            }

            if (bestA < 0) break;
            swaps.append({bestA, bestB});
            locked[bestA] = true;
            locked[bestB] = true;

            /* 更新D值 */
            for (const auto& [u, w] : adj[bestA]) {
                if (!locked[u]) {
                    D[u] += (result.partition[u] == 0) ? 2.0 * w : -2.0 * w;
                }
            }
            for (const auto& [u, w] : adj[bestB]) {
                if (!locked[u]) {
                    D[u] += (result.partition[u] == 1) ? 2.0 * w : -2.0 * w;
                }
            }
        }

        /* 找最佳前k次交换 */
        int currentCut = bestCut;
        int bestK = 0;
        int tempCut = currentCut;
        for (int k = 0; k < swaps.size(); ++k) {
            tempCut -= static_cast<int>(
                computeEdgeCut(result.partition, edges) - tempCut);
            if (tempCut < currentCut) {
                currentCut = tempCut;
                bestK = k + 1;
            }
        }

        /* 应用最佳交换 */
        for (int k = 0; k < bestK; ++k) {
            result.partition[swaps[k].first] = 1;
            result.partition[swaps[k].second] = 0;
        }

        result.edgeCut = computeEdgeCut(result.partition, edges);
        result.iterations = iter + 1;

        emit iterationProgress(iter + 1, result.edgeCut);

        if (result.edgeCut < bestCut) {
            bestCut = result.edgeCut;
            bestPart = result.partition;
        } else {
            break;
        }
    }

    result.partition = bestPart;
    result.edgeCut = bestCut;
    result.imbalance = computeImbalance(result.partition);
    return result;
}

/** @brief Fiduccia-Mattheyses细化 @param n 顶点数 @param edges 边列表 @return 分割结果 */
GraphPartition::PartitionResult GraphPartition::fiducciaMattheyses(
    int n, const QList<Edge>& edges)
{
    PartitionResult result;
    result.partition = initialBisection(n);

    QVector<QVector<QPair<int, double>>> adj(n);
    for (const auto& e : edges) {
        if (e.source >= 0 && e.source < n && e.target >= 0 && e.target < n) {
            adj[e.source].append({e.target, e.weight});
            adj[e.target].append({e.source, e.weight});
        }
    }

    int bestCut = computeEdgeCut(result.partition, edges);
    QVector<int> bestPart = result.partition;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        QVector<double> gain(n, 0.0);
        for (int v = 0; v < n; ++v) {
            for (const auto& [u, w] : adj[v]) {
                gain[v] += (result.partition[v] != result.partition[u]) ? w : -w;
            }
        }

        QVector<bool> moved(n, false);
        int maxMoves = qMax(1, static_cast<int>(n * m_tolerance));

        for (int m = 0; m < maxMoves; ++m) {
            double bestGain = -std::numeric_limits<double>::max();
            int bestV = -1;
            int part0Count = 0, part1Count = 0;
            for (int v = 0; v < n; ++v) {
                if (moved[v]) continue;
                if (result.partition[v] == 0) part0Count++;
                else part1Count++;
                if (gain[v] > bestGain) {
                    bestGain = gain[v];
                    bestV = v;
                }
            }

            if (bestV < 0 || bestGain <= 0) break;

            moved[bestV] = true;
            int oldPart = result.partition[bestV];
            result.partition[bestV] = 1 - oldPart;

            for (const auto& [u, w] : adj[bestV]) {
                if (!moved[u]) {
                    gain[u] += (result.partition[u] == oldPart) ? 2.0 * w : -2.0 * w;
                }
            }
        }

        result.edgeCut = computeEdgeCut(result.partition, edges);
        result.iterations = iter + 1;
        emit iterationProgress(iter + 1, result.edgeCut);

        if (result.edgeCut < bestCut) {
            bestCut = result.edgeCut;
            bestPart = result.partition;
        } else {
            break;
        }
    }

    result.partition = bestPart;
    result.edgeCut = bestCut;
    result.imbalance = computeImbalance(bestPart);
    return result;
}

/** @brief 多层递归二分 @param n 顶点数 @param edges 边列表 @return 分割结果 */
GraphPartition::PartitionResult GraphPartition::multilevelBisection(
    int n, const QList<Edge>& edges)
{
    /* 简化实现: 先粗化到小图，再用KL分割，然后投影回原 */
    if (n <= 64) return kernighanLin(n, edges);

    /* 随机匹配粗化 */
    QVector<int> mapping(n);
    for (int i = 0; i < n; ++i) mapping[i] = i / 2;

    int coarseN = (n + 1) / 2;
    QList<Edge> coarseEdges;
    QMap<QPair<int, int>, double> edgeAgg;
    for (const auto& e : edges) {
        int cu = mapping[e.source];
        int cv = mapping[e.target];
        if (cu == cv) continue;
        auto key = qMakePair(qMin(cu, cv), qMax(cu, cv));
        edgeAgg[key] += e.weight;
    }
    for (auto it = edgeAgg.constBegin(); it != edgeAgg.constEnd(); ++it) {
        coarseEdges.append({it.key().first, it.key().second, it.value()});
    }

    /* 递归分割粗化图 */
    auto coarseResult = multilevelBisection(coarseN, coarseEdges);

    /* 投影回原始图 */
    PartitionResult result;
    result.partition.resize(n);
    for (int i = 0; i < n; ++i) {
        result.partition[i] = coarseResult.partition[mapping[i]];
    }

    /* 用FM细化 */
    auto refined = fiducciaMattheyses(n, edges);
    refined.partition = result.partition;
    result.edgeCut = computeEdgeCut(result.partition, edges);
    result.imbalance = computeImbalance(result.partition);
    return result;
}

/** @brief 初始二分 @param n 顶点数 @return 分区向量 */
QVector<int> GraphPartition::initialBisection(int n) const
{
    QVector<int> part(n, 0);
    for (int i = n / 2; i < n; ++i) part[i] = 1;
    return part;
}

/** @brief 计算边割数 @param part 分区 @param edges 边列表 @return 边割数 */
int GraphPartition::computeEdgeCut(const QVector<int>& part,
                                    const QList<Edge>& edges) const
{
    int cut = 0;
    for (const auto& e : edges) {
        if (e.source >= 0 && e.source < part.size()
            && e.target >= 0 && e.target < part.size()) {
            if (part[e.source] != part[e.target]) {
                cut += static_cast<int>(e.weight);
            }
        }
    }
    return cut;
}

/** @brief 计算不平衡度 @param part 分区 @return 不平衡度 */
double GraphPartition::computeImbalance(const QVector<int>& part) const
{
    if (part.isEmpty()) return 0.0;
    int count0 = 0, count1 = 0;
    for (int p : part) {
        if (p == 0) count0++;
        else count1++;
    }
    double ideal = part.size() / 2.0;
    return qAbs(count0 - ideal) / ideal;
}
