/**
 * @file HierarchicalCluster4.cpp
 * @brief 层次聚类4实现 — BIRCH预聚合+快速LINKAGE
 *
 * 本模块实现层次聚类算法，支持BIRCH预聚合加速和多种链接方式
 * (single/complete/average/ward)。使用凝聚策略自底向上构建树状图，
 * 在指定聚类数或截断高度处停止合并。
 *
 * 统计信息跟踪: 聚类次数、处理点数、合并次数、平均耗时。
 */

#include "utils/cluster41/HierarchicalCluster4.h"

#include <QElapsedTimer>
#include <QMap>
#include <QList>
#include <QtMath>

#include <algorithm>
#include <limits>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
HierarchicalCluster4::HierarchicalCluster4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置链接方式
 * @param method 链接方法名称: "single", "complete", "average", "ward"
 */
void HierarchicalCluster4::setLinkage(const QString& method)
{
    m_linkage = method.toLower();
}

/**
 * @brief 设置目标聚类数
 * @param k 聚类数，必须 >= 2
 */
void HierarchicalCluster4::setNumClusters(int k)
{
    m_numClusters = qMax(2, k);
}

/**
 * @brief 设置BIRCH预聚合的距离阈值
 * @param threshold 预聚合阈值，距离小于此值的点被聚合
 */
void HierarchicalCluster4::setPreclusterThreshold(double threshold)
{
    m_preclusterThreshold = qMax(0.0, threshold);
}

/**
 * @brief 对数据进行层次聚类拟合
 *
 * 执行流程:
 * 1. BIRCH预聚合: 将距离小于阈值的点聚合为代表点
 * 2. 计算距离矩阵
 * 3. 凝聚合并: 每次合并距离最近的两个簇
 * 4. 记录树状图(合并对+合并高度)
 *
 * @param data 输入数据，每个元素是一个特征向量
 * @return 每个数据点的聚类标签(0~k-1)
 */
QVector<int> HierarchicalCluster4::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    const int n = data.size();
    if (n == 0) {
        return {};
    }

    m_dendrogram.clear();
    m_heights.clear();

    /* === 步骤1: BIRCH预聚合 === */
    QVector<QVector<double>> points = data;  ///< 工作副本
    QVector<QVector<int>> clusters(n);       ///< 每个簇包含的原始点索引
    QVector<bool> active(n, true);

    for (int i = 0; i < n; ++i) {
        clusters[i].append(i);
    }

    // 对距离小于阈值的点进行预聚合
    if (m_preclusterThreshold > 0.0 && n > m_numClusters) {
        QVector<double> distMat = computeDistanceMatrix(data);
        for (int i = 0; i < n; ++i) {
            if (!active[i]) continue;
            for (int j = i + 1; j < n; ++j) {
                if (!active[j]) continue;
                double d = distMat[i * n + j];
                if (d < m_preclusterThreshold) {
                    // 将 j 合并到 i
                    clusters[i].append(clusters[j]);
                    active[j] = false;
                }
            }
        }
    }

    /* === 步骤2: 收集活跃簇 === */
    QVector<int> activeIds;  ///< 当前活跃簇的索引
    for (int i = 0; i < n; ++i) {
        if (active[i]) {
            activeIds.append(i);
        }
    }

    int numActive = activeIds.size();
    int mergeCount = 0;

    /* === 步骤3: 凝聚合并 === */
    // 计算活跃簇之间的距离矩阵
    QVector<double> distMat = computeDistanceMatrix(data);

    // 簇之间的距离缓存
    int currentN = numActive;
    QVector<QVector<double>> clusterDist(currentN,
                                          QVector<double>(currentN, 0.0));

    // 映射: 活跃索引 -> 原始索引
    QVector<int> idMap = activeIds;

    for (int i = 0; i < currentN; ++i) {
        for (int j = i + 1; j < currentN; ++j) {
            double d = linkageDistance(distMat, n,
                                       clusters[idMap[i]],
                                       clusters[idMap[j]]);
            clusterDist[i][j] = d;
            clusterDist[j][i] = d;
        }
    }

    // 合并追踪: 每个簇的当前索引映射
    QVector<int> mergeMap(currentN);
    for (int i = 0; i < currentN; ++i) {
        mergeMap[i] = i;
    }

    while (currentN > m_numClusters && currentN > 1) {
        // 查找最小距离对
        double minDist = std::numeric_limits<double>::max();
        int minI = 0, minJ = 1;

        for (int i = 0; i < numActive; ++i) {
            if (mergeMap[i] != i) continue;
            for (int j = i + 1; j < numActive; ++j) {
                if (mergeMap[j] != j) continue;
                if (clusterDist[i][j] < minDist) {
                    minDist = clusterDist[i][j];
                    minI = i;
                    minJ = j;
                }
            }
        }

        // 记录树状图
        m_dendrogram.append(qMakePair(idMap[minI], idMap[minJ]));
        m_heights.append(minDist);

        // 合并 minJ -> minI
        clusters[idMap[minI]].append(clusters[idMap[minJ]]);
        mergeMap[minJ] = minI;  ///< 标记 minJ 已合并

        // 更新 minI 到其他簇的距离
        for (int k = 0; k < numActive; ++k) {
            if (k == minI || k == minJ || mergeMap[k] != k) continue;
            double d = linkageDistance(distMat, n,
                                       clusters[idMap[minI]],
                                       clusters[idMap[k]]);
            clusterDist[minI][k] = d;
            clusterDist[k][minI] = d;
        }

        currentN--;
        mergeCount++;
    }

    /* === 步骤4: 生成标签 === */
    QVector<int> labels(n, -1);
    int label = 0;

    for (int i = 0; i < numActive; ++i) {
        if (mergeMap[i] != i) continue;
        for (int idx : clusters[idMap[i]]) {
            labels[idx] = label;
        }
        label++;
    }

    // 更新统计信息
    m_stats.totalClusterings++;
    m_stats.totalPointsProcessed += n;
    m_stats.totalMerges += mergeCount;

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;

    emit clusteringCompleted(n, label);
    return labels;
}

/**
 * @brief 在指定高度处截断树状图，获取聚类结果
 * @param h 截断高度
 * @return 每个聚类包含的点索引集合
 */
QVector<QVector<int>> HierarchicalCluster4::cutAtHeight(double h) const
{
    if (m_dendrogram.isEmpty()) {
        return {};
    }

    // 从所有点各自为一个簇开始
    const int n = m_dendrogram.size() + 1;  ///< n-1次合并 -> n个初始点

    // 确定实际数据点数量 (通过树状图推断)
    int maxIdx = 0;
    for (const auto& pair : m_dendrogram) {
        maxIdx = qMax(maxIdx, qMax(pair.first, pair.second));
    }
    const int numPoints = maxIdx + 1;

    QVector<int> parent(numPoints);
    for (int i = 0; i < numPoints; ++i) {
        parent[i] = i;
    }

    // 仅执行高度 <= h 的合并
    for (int i = 0; i < m_heights.size() && i < m_dendrogram.size(); ++i) {
        if (m_heights[i] > h) break;

        int a = m_dendrogram[i].first;
        int b = m_dendrogram[i].second;
        if (a < numPoints && b < numPoints) {
            parent[b] = a;
        }
    }

    // 按根节点分组
    QMap<int, QVector<int>> groups;
    for (int i = 0; i < numPoints; ++i) {
        int root = i;
        while (parent[root] != root) {
            root = parent[root];
        }
        groups[root].append(i);
    }

    return groups.values().toVector();
}

/**
 * @brief 计算完整距离矩阵 (展平为1D数组)
 * @param data 输入数据点集合
 * @return 距离矩阵，distMat[i*n+j] 为点i和点j的欧氏距离
 */
QVector<double> HierarchicalCluster4::computeDistanceMatrix(
    const QVector<QVector<double>>& data) const
{
    const int n = data.size();
    QVector<double> dist(n * n, 0.0);

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double sum = 0.0;
            const int dim = qMin(data[i].size(), data[j].size());
            for (int d = 0; d < dim; ++d) {
                double diff = data[i][d] - data[j][d];
                sum += diff * diff;
            }
            double d = qSqrt(sum);
            dist[i * n + j] = d;
            dist[j * n + i] = d;
        }
    }
    return dist;
}

/**
 * @brief 根据链接方式计算两个簇之间的距离
 * @param dist 原始距离矩阵
 * @param n 数据点总数
 * @param c1 簇1的点索引集合
 * @param c2 簇2的点索引集合
 * @return 簇间距离
 */
double HierarchicalCluster4::linkageDistance(const QVector<double>& dist,
                                             int n,
                                             const QVector<int>& c1,
                                             const QVector<int>& c2) const
{
    if (c1.isEmpty() || c2.isEmpty()) {
        return 0.0;
    }

    if (m_linkage == "single") {
        // 单链接: 最小距离
        double minD = std::numeric_limits<double>::max();
        for (int i : c1) {
            for (int j : c2) {
                if (i < n && j < n) {
                    minD = qMin(minD, dist[i * n + j]);
                }
            }
        }
        return minD;
    }

    if (m_linkage == "complete") {
        // 全链接: 最大距离
        double maxD = 0.0;
        for (int i : c1) {
            for (int j : c2) {
                if (i < n && j < n) {
                    maxD = qMax(maxD, dist[i * n + j]);
                }
            }
        }
        return maxD;
    }

    if (m_linkage == "ward") {
        // Ward链接: 增量平方和
        double sum = 0.0;
        for (int i : c1) {
            for (int j : c2) {
                if (i < n && j < n) {
                    double d = dist[i * n + j];
                    sum += d * d;
                }
            }
        }
        // Ward距离 = 2 * n1 * n2 / (n1 + n2) * 平均平方距离
        double factor = 2.0 * c1.size() * c2.size()
                        / static_cast<double>(c1.size() + c2.size());
        return qSqrt(sum / (c1.size() * c2.size())) * qSqrt(factor);
    }

    // 默认: 平均链接
    double sum = 0.0;
    int count = 0;
    for (int i : c1) {
        for (int j : c2) {
            if (i < n && j < n) {
                sum += dist[i * n + j];
                count++;
            }
        }
    }
    return count > 0 ? sum / count : 0.0;
}

/**
 * @brief 重置所有统计信息
 */
void HierarchicalCluster4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
