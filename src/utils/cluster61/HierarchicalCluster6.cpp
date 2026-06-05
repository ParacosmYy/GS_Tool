/**
 * @file HierarchicalCluster6.cpp
 * @brief 层次聚类算法实现 — 凝聚式自底向上聚类
 *
 * 支持ward/complete/average/single四种链接策略，
 * 通过逐步合并最近的簇对构建树状图，输出聚类标签和合并历史。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/cluster61/HierarchicalCluster6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject指针
 */
HierarchicalCluster6::HierarchicalCluster6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置链接策略
 * @param type 链接类型: "ward"(默认), "complete", "average", "single"
 */
void HierarchicalCluster6::setLinkage(const QString& type)
{
    if (type == "ward" || type == "complete" || type == "average" || type == "single") {
        m_linkage = type;
    }
}

/**
 * @brief 设置目标聚类数量
 * @param k 聚类数，必须 >= 1
 */
void HierarchicalCluster6::setNumClusters(int k)
{
    m_k = qMax(1, k);
}

/**
 * @brief 执行层次聚类
 *
 * 凝聚式流程:
 * 1. 每个点初始为独立簇
 * 2. 计算所有簇对之间的距离
 * 3. 合并距离最小的簇对
 * 4. 重复直到达到目标聚类数
 * 5. 记录合并历史(树状图)
 *
 * @param points 输入数据点集合
 * @return 聚类标签向量
 */
QVector<int> HierarchicalCluster6::cluster(const QVector<QVector<double>>& points)
{
    QElapsedTimer timer;
    timer.start();

    const int n = points.size();
    if (n == 0) {
        return {};
    }

    const int dim = points[0].size();
    const int actualK = qMin(m_k, n);

    /* 初始化: 每个点为一个簇 */
    QVector<QVector<int>> clusters(n);
    for (int i = 0; i < n; ++i) {
        clusters[i].append(i);
    }

    m_dendrogram.clear();
    m_mergeDist.clear();

    /* 预计算所有点对之间的欧氏距离 */
    QVector<QVector<double>> distMatrix(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double d = 0.0;
            for (int dd = 0; dd < dim; ++dd) {
                double diff = points[i][dd] - points[j][dd];
                d += diff * diff;
            }
            distMatrix[i][j] = distMatrix[j][i] = qSqrt(d);
        }
    }

    /* 计算簇间距离矩阵 */
    QVector<QVector<double>> clusterDist(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            clusterDist[i][j] = clusterDist[j][i] = distMatrix[i][j];
        }
    }

    /* 合并标记: 哪些簇仍然活跃 */
    QVector<bool> active(n, true);

    /* 迭代合并 */
    int numClusters = n;
    int nextId = n; /* 新簇的编号 */

    while (numClusters > actualK) {
        /* 寻找最近的活跃簇对 */
        double minDist = 1e18;
        int mergeI = -1, mergeJ = -1;

        for (int i = 0; i < n; ++i) {
            if (!active[i]) continue;
            for (int j = i + 1; j < n; ++j) {
                if (!active[j]) continue;
                if (clusterDist[i][j] < minDist) {
                    minDist = clusterDist[i][j];
                    mergeI = i;
                    mergeJ = j;
                }
            }
        }

        if (mergeI < 0) break;

        /* 记录合并历史 */
        m_dendrogram.append(qMakePair(mergeI, mergeJ));
        m_mergeDist.append(minDist);

        /* 合并: 将mergeJ并入mergeI */
        clusters[mergeI].append(clusters[mergeJ]);
        clusters[mergeJ].clear();
        active[mergeJ] = false;

        /* 更新cluster距离 */
        for (int k = 0; k < n; ++k) {
            if (!active[k] || k == mergeI) continue;
            double newDist = clusterDistance(points, clusters[mergeI], clusters[k]);
            clusterDist[mergeI][k] = clusterDist[k][mergeI] = newDist;
        }

        numClusters--;
        nextId++;
    }

    /* 生成聚类标签 */
    QVector<int> labels(n, 0);
    int label = 0;
    for (int i = 0; i < n; ++i) {
        if (!active[i] || clusters[i].isEmpty()) continue;
        for (int idx : clusters[i]) {
            labels[idx] = label;
        }
        label++;
    }

    /* 最终合并高度 */
    double finalHeight = m_mergeDist.isEmpty() ? 0.0 : m_mergeDist.last();

    /* 更新统计信息 */
    m_stats.totalClusterings++;
    m_stats.totalPoints += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;

    emit clusteringCompleted(actualK, finalHeight);
    return labels;
}

/**
 * @brief 重置所有统计数据
 */
void HierarchicalCluster6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 计算两个簇之间的距离
 *
 * 根据链接策略选择不同的距离度量:
 * - ward: Ward方差最小化(合并后SSE增量)
 * - complete: 最远点距离
 * - average: 平均距离
 * - single: 最近点距离
 *
 * @param pts 原始数据点
 * @param a 第一个簇的点索引集合
 * @param b 第二个簇的点索引集合
 * @return 簇间距离
 */
double HierarchicalCluster6::clusterDistance(
    const QVector<QVector<double>>& pts,
    const QVector<int>& a,
    const QVector<int>& b)
{
    if (a.isEmpty() || b.isEmpty()) return 1e18;

    const int dim = pts[0].size();

    if (m_linkage == "single") {
        /* 最近点距离 */
        double minD = 1e18;
        for (int i : a) {
            for (int j : b) {
                double d = 0.0;
                for (int dd = 0; dd < dim; ++dd) {
                    double diff = pts[i][dd] - pts[j][dd];
                    d += diff * diff;
                }
                minD = qMin(minD, qSqrt(d));
            }
        }
        return minD;
    }

    if (m_linkage == "complete") {
        /* 最远点距离 */
        double maxD = 0.0;
        for (int i : a) {
            for (int j : b) {
                double d = 0.0;
                for (int dd = 0; dd < dim; ++dd) {
                    double diff = pts[i][dd] - pts[j][dd];
                    d += diff * diff;
                }
                maxD = qMax(maxD, qSqrt(d));
            }
        }
        return maxD;
    }

    if (m_linkage == "average") {
        /* 平均距离 */
        double sum = 0.0;
        int count = 0;
        for (int i : a) {
            for (int j : b) {
                double d = 0.0;
                for (int dd = 0; dd < dim; ++dd) {
                    double diff = pts[i][dd] - pts[j][dd];
                    d += diff * diff;
                }
                sum += qSqrt(d);
                ++count;
            }
        }
        return (count > 0) ? sum / count : 0.0;
    }

    /* ward: 合并后的SSE增量 */
    /* 计算两个簇的质心 */
    QVector<double> centroidA(dim, 0.0);
    QVector<double> centroidB(dim, 0.0);
    for (int i : a) {
        for (int dd = 0; dd < dim; ++dd) {
            centroidA[dd] += pts[i][dd];
        }
    }
    for (int j : b) {
        for (int dd = 0; dd < dim; ++dd) {
            centroidB[dd] += pts[j][dd];
        }
    }
    for (int dd = 0; dd < dim; ++dd) {
        centroidA[dd] /= a.size();
        centroidB[dd] /= b.size();
    }
    /* Ward距离 = |a|*|b|/(|a|+|b|) * ||centroidA - centroidB||^2 */
    double dist2 = 0.0;
    for (int dd = 0; dd < dim; ++dd) {
        double diff = centroidA[dd] - centroidB[dd];
        dist2 += diff * diff;
    }
    double na = static_cast<double>(a.size());
    double nb = static_cast<double>(b.size());
    return (na * nb) / (na + nb) * dist2;
}
