/**
 * @file BirchClustering.cpp
 * @brief BIRCH聚类引擎实现 — CF树增量聚类算法
 */

#include "utils/cluster13/BirchClustering.h"

#include <QElapsedTimer>
#include <QtMath>

#include <QMap>

#include <algorithm>
#include <limits>
#include <cmath>

// ClusteringFeature 方法

QVector<double> BirchClustering::ClusteringFeature::centroid() const
{
    if (n == 0) return linearSum;
    QVector<double> c(linearSum.size());
    for (int i = 0; i < linearSum.size(); ++i) {
        c[i] = linearSum[i] / static_cast<double>(n);
    }
    return c;
}

double BirchClustering::ClusteringFeature::radius() const
{
    if (n <= 1) return 0.0;
    double sum = 0.0;
    for (int i = 0; i < linearSum.size(); ++i) {
        double mean = linearSum[i] / static_cast<double>(n);
        /* 方差贡献 = SS/n - (LS/n)^2 */
        double var = squareSum[i] / static_cast<double>(n) - mean * mean;
        sum += var;
    }
    return qSqrt(qMax(0.0, sum));
}

double BirchClustering::ClusteringFeature::diameter() const
{
    if (n <= 1) return 0.0;
    /* 直径 D0 = sqrt( 2*n*SS - 2*LS·LS ) / n */
    double ssTerm = 0.0;
    double lsTerm = 0.0;
    for (int i = 0; i < linearSum.size(); ++i) {
        ssTerm += squareSum[i];
        lsTerm += linearSum[i] * linearSum[i];
    }
    double d0sq = (2.0 * static_cast<double>(n) * ssTerm - 2.0 * lsTerm)
                  / static_cast<double>(n * (n - 1));
    return qSqrt(qMax(0.0, d0sq));
}

void BirchClustering::ClusteringFeature::merge(const ClusteringFeature& other)
{
    n += other.n;
    for (int i = 0; i < linearSum.size(); ++i) {
        linearSum[i] += other.linearSum[i];
        squareSum[i] += other.squareSum[i];
    }
}

// 构造 / 析构

BirchClustering::BirchClustering(double threshold, int branchingFactor,
                                 QObject* parent)
    : QObject(parent)
    , m_threshold(threshold)
    , m_branchingFactor(qMax(2, branchingFactor))
    , m_dimensions(0)
    , m_totalPoints(0)
    , m_rootIndex(-1)
    , m_firstLeaf(-1)
    , m_height(0)
{
}

BirchClustering::~BirchClustering() = default;

// CF树构建

bool BirchClustering::insertPoint(const QVector<double>& point)
{
    QElapsedTimer timer;
    timer.start();

    /* 首次插入: 初始化维度和根节点 */
    if (m_rootIndex < 0) {
        m_dimensions = point.size();
        CFNode root;
        root.isLeaf = true;
        m_nodes.append(root);
        m_rootIndex = 0;
        m_firstLeaf = 0;
        m_height = 1;
    }

    if (point.size() != m_dimensions) return false;

    /* 构造单点CF */
    ClusteringFeature pointCF;
    pointCF.n = 1;
    pointCF.linearSum = point;
    pointCF.squareSum.resize(m_dimensions);
    for (int i = 0; i < m_dimensions; ++i) {
        pointCF.squareSum[i] = point[i] * point[i];
    }

    /* 从根向下查找最近叶条目 */
    QPair<int, int> closest = findClosestLeaf(point);
    int nodeIdx = closest.first;
    int entryIdx = closest.second;

    if (nodeIdx < 0) {
        /* 空叶节点: 直接插入 */
        m_nodes[m_rootIndex].entries.append(pointCF);
    } else {
        ClusteringFeature& entry = m_nodes[nodeIdx].entries[entryIdx];
        /* 尝试吸收: 合并后半径是否超过阈值 */
        ClusteringFeature merged = entry;
        merged.merge(pointCF);
        if (merged.radius() <= m_threshold) {
            entry = merged;
        } else {
            /* 不能吸收: 在同一叶节点新增条目 */
            m_nodes[nodeIdx].entries.append(pointCF);

            /* 检查叶节点是否溢出 */
            if (m_nodes[nodeIdx].entries.size() > m_branchingFactor) {
                splitLeafNode(nodeIdx);
            }
        }
    }

    m_totalPoints++;
    m_stats.totalInsertions++;
    m_timeSum += static_cast<double>(timer.elapsed());
    m_stats.avgProcessingTimeMs =
        m_timeSum / static_cast<double>(m_stats.totalInsertions);

    emit pointInserted(m_totalPoints);
    return true;
}

int BirchClustering::insertBatch(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int count = 0;
    for (const auto& point : data) {
        if (insertPoint(point)) ++count;
    }

    m_stats.totalInsertions += count;
    m_timeSum += static_cast<double>(timer.elapsed());
    quint64 totalOps = m_stats.totalInsertions + m_stats.totalClusterings;
    m_stats.avgProcessingTimeMs =
        (totalOps > 0) ? m_timeSum / static_cast<double>(totalOps) : 0.0;

    return count;
}

void BirchClustering::clearTree()
{
    m_nodes.clear();
    m_rootIndex = -1;
    m_firstLeaf = -1;
    m_height = 0;
    m_totalPoints = 0;
    m_dimensions = 0;
}

// 全局聚类

BirchClustering::ClusterResult BirchClustering::cluster(int k)
{
    QElapsedTimer timer;
    timer.start();

    ClusterResult result;
    result.success = false;

    /* 收集所有叶条目 */
    QVector<ClusteringFeature> leafEntries;
    int nodeIdx = m_firstLeaf;
    while (nodeIdx >= 0) {
        for (const auto& entry : m_nodes[nodeIdx].entries) {
            leafEntries.append(entry);
        }
        nodeIdx = m_nodes[nodeIdx].nextLeaf;
    }

    result.leafEntries = leafEntries.size();
    result.treeHeight = m_height;

    if (leafEntries.isEmpty()) {
        m_stats.totalClusterings++;
        emit clusteringCompleted(0);
        return result;
    }

    /* 自动确定簇数: sqrt(n)启发式 */
    int targetK = (k > 0) ? k : qMax(1, static_cast<int>(qSqrt(
        static_cast<double>(leafEntries.size()))));
    targetK = qMin(targetK, leafEntries.size());

    /* 对叶CF条目做凝聚聚类 */
    QVector<int> entryLabels = agglomerativeCluster(leafEntries, targetK);

    /* 合并同簇CF */
    QVector<ClusteringFeature> finalClusters(targetK);
    for (int i = 0; i < targetK; ++i) {
        finalClusters[i].linearSum.resize(m_dimensions);
        finalClusters[i].squareSum.resize(m_dimensions);
    }

    for (int i = 0; i < leafEntries.size(); ++i) {
        int label = entryLabels[i];
        finalClusters[label].merge(leafEntries[i]);
    }

    /* 为所有原始输入点分配标签 */
    result.labels.resize(m_totalPoints);
    int ptIdx = 0;
    nodeIdx = m_firstLeaf;
    while (nodeIdx >= 0) {
        for (int e = 0; e < m_nodes[nodeIdx].entries.size(); ++e) {
            int label = entryLabels[ptIdx < entryLabels.size() ? ptIdx : 0];
            int nPts = m_nodes[nodeIdx].entries[e].n;
            for (int p = 0; p < nPts && ptIdx < m_totalPoints; ++p) {
                result.labels[ptIdx++] = label;
            }
        }
        nodeIdx = m_nodes[nodeIdx].nextLeaf;
    }

    result.clusters = finalClusters;
    result.success = true;

    m_stats.totalClusterings++;
    m_timeSum += static_cast<double>(timer.elapsed());
    quint64 totalOps = m_stats.totalInsertions + m_stats.totalClusterings;
    m_stats.avgProcessingTimeMs =
        m_timeSum / static_cast<double>(totalOps);

    emit clusteringCompleted(targetK);
    return result;
}

// 查询

int BirchClustering::leafEntryCount() const
{
    int count = 0;
    int nodeIdx = m_firstLeaf;
    while (nodeIdx >= 0 && nodeIdx < m_nodes.size()) {
        count += m_nodes[nodeIdx].entries.size();
        nodeIdx = m_nodes[nodeIdx].nextLeaf;
    }
    return count;
}

int BirchClustering::treeHeight() const
{
    return m_height;
}

// 统计

BirchClustering::Stats BirchClustering::stats() const
{
    return m_stats;
}

void BirchClustering::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

// 内部实现

QPair<int, int> BirchClustering::findClosestLeaf(const QVector<double>& point)
{
    if (m_rootIndex < 0 || m_nodes.isEmpty()) return {-1, -1};

    int nodeIdx = m_rootIndex;

    /* 从根向下遍历到叶 */
    while (!m_nodes[nodeIdx].isLeaf) {
        double bestDist = std::numeric_limits<double>::max();
        int bestEntry = 0;

        for (int i = 0; i < m_nodes[nodeIdx].entries.size(); ++i) {
            double d = 0.0;
            QVector<double> cent = m_nodes[nodeIdx].entries[i].centroid();
            for (int j = 0; j < m_dimensions; ++j) {
                double diff = point[j] - cent[j];
                d += diff * diff;
            }
            if (d < bestDist) {
                bestDist = d;
                bestEntry = i;
            }
        }

        /* 选择最近CF对应的子节点 */
        if (bestEntry < m_nodes[nodeIdx].children.size()) {
            nodeIdx = m_nodes[nodeIdx].children[bestEntry];
        } else {
            break;
        }
    }

    /* 到达叶节点: 找最近条目 */
    double bestDist = std::numeric_limits<double>::max();
    int bestEntry = 0;

    for (int i = 0; i < m_nodes[nodeIdx].entries.size(); ++i) {
        double d = 0.0;
        QVector<double> cent = m_nodes[nodeIdx].entries[i].centroid();
        for (int j = 0; j < m_dimensions; ++j) {
            double diff = point[j] - cent[j];
            d += diff * diff;
        }
        if (d < bestDist) {
            bestDist = d;
            bestEntry = i;
        }
    }

    return {nodeIdx, bestEntry};
}

void BirchClustering::splitLeafNode(int nodeIndex)
{
    /* 找距离最远的两个CF作为种子 */
    const auto& entries = m_nodes[nodeIndex].entries;
    int n = entries.size();
    double maxDist = -1.0;
    int seedA = 0, seedB = 1;

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double d = cfDistance(entries[i], entries[j]);
            if (d > maxDist) {
                maxDist = d;
                seedA = i;
                seedB = j;
            }
        }
    }

    /* 创建新叶节点 */
    CFNode newLeaf;
    newLeaf.isLeaf = true;

    /* 分配条目到两个节点 */
    QVector<ClusteringFeature> groupA, groupB;
    for (int i = 0; i < n; ++i) {
        double dA = cfDistance(entries[i], entries[seedA]);
        double dB = cfDistance(entries[i], entries[seedB]);
        if (dA <= dB) {
            groupA.append(entries[i]);
        } else {
            groupB.append(entries[i]);
        }
    }

    m_nodes[nodeIndex].entries = groupA;
    newLeaf.entries = groupB;

    /* 更新叶链表 */
    newLeaf.nextLeaf = m_nodes[nodeIndex].nextLeaf;
    newLeaf.prevLeaf = nodeIndex;
    if (m_nodes[nodeIndex].nextLeaf >= 0) {
        m_nodes[m_nodes[nodeIndex].nextLeaf].prevLeaf = m_nodes.size();
    }
    m_nodes[nodeIndex].nextLeaf = m_nodes.size();

    int newIdx = m_nodes.size();
    m_nodes.append(newLeaf);

    m_stats.totalTreeSplits++;
    emit treeSplit(1);
}

double BirchClustering::cfDistance(const ClusteringFeature& a,
                                    const ClusteringFeature& b) const
{
    QVector<double> ca = a.centroid();
    QVector<double> cb = b.centroid();
    double sum = 0.0;
    int dims = qMin(ca.size(), cb.size());
    for (int i = 0; i < dims; ++i) {
        double diff = ca[i] - cb[i];
        sum += diff * diff;
    }
    return qSqrt(sum);
}

QVector<int> BirchClustering::agglomerativeCluster(
    const QVector<ClusteringFeature>& entries, int k)
{
    int n = entries.size();
    if (n <= k) {
        QVector<int> labels(n);
        for (int i = 0; i < n; ++i) labels[i] = i;
        return labels;
    }

    /* 初始: 每个条目自成一簇 */
    QVector<int> labels(n);
    QVector<int> clusterSize(n, 1);
    QVector<ClusteringFeature> clusterCF = entries;
    int numClusters = n;

    for (int i = 0; i < n; ++i) labels[i] = i;

    /* 距离矩阵(上三角) */
    QVector<QVector<double>> dist(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            dist[i][j] = cfDistance(entries[i], entries[j]);
        }
    }

    /* 凝聚合并 */
    while (numClusters > k) {
        double minDist = std::numeric_limits<double>::max();
        int mergeA = -1, mergeB = -1;

        /* 找最近两个活跃簇 */
        for (int i = 0; i < n; ++i) {
            if (clusterSize[i] == 0) continue;
            for (int j = i + 1; j < n; ++j) {
                if (clusterSize[j] == 0) continue;
                if (dist[i][j] < minDist) {
                    minDist = dist[i][j];
                    mergeA = i;
                    mergeB = j;
                }
            }
        }

        if (mergeA < 0) break;

        /* 合并 mergeB -> mergeA */
        clusterCF[mergeA].merge(clusterCF[mergeB]);
        clusterSize[mergeA] += clusterSize[mergeB];
        clusterSize[mergeB] = 0;

        /* 更新距离(质心链接) */
        for (int j = 0; j < n; ++j) {
            if (j == mergeA || clusterSize[j] == 0) continue;
            double d = cfDistance(clusterCF[mergeA], clusterCF[j]);
            dist[qMin(mergeA, j)][qMax(mergeA, j)] = d;
        }

        numClusters--;
    }

    /* 重新编号标签为 [0, numClusters) */
    QMap<int, int> remap;
    int nextLabel = 0;
    for (int i = 0; i < n; ++i) {
        if (clusterSize[i] > 0) {
            remap[i] = nextLabel++;
        }
    }

    QVector<int> finalLabels(n);
    for (int i = 0; i < n; ++i) {
        int root = labels[i];
        while (clusterSize[root] == 0) {
            bool found = false;
            for (int j = 0; j < n; ++j) {
                if (clusterSize[j] > 0 && j != root) { found = true; break; }
            }
            if (!found) break;
        }
        finalLabels[i] = remap.value(root, 0);
    }

    return finalLabels;
}
