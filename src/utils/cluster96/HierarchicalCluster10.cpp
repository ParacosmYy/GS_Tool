#include "HierarchicalCluster10.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file HierarchicalCluster10.cpp
 * @brief 层次聚类分析器实现
 *
 * 实现凝聚型(Agglomerative)层次聚类，每步合并距离最近的两个簇，
 * 支持single/complete/average/ward四种链接策略。
 */

/**
 * @brief 构造函数，初始化默认链接策略和距离度量
 * @param parent 父QObject对象指针
 */
HierarchicalCluster10::HierarchicalCluster10(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置链接策略
 * @param method 链接方法名称: single/complete/average/ward
 */
void HierarchicalCluster10::setLinkage(const QString& method)
{
    if (method == "single" || method == "complete" ||
        method == "average" || method == "ward") {
        m_linkage = method;
    }
}

/**
 * @brief 设置距离度量
 * @param metric 距离度量名称: euclidean/manhattan/cosine
 */
void HierarchicalCluster10::setMetric(const QString& metric)
{
    if (metric == "euclidean" || metric == "manhattan" || metric == "cosine") {
        m_metric = metric;
    }
}

/**
 * @brief 计算两个数据点之间的距离
 * @param a 第一个数据点
 * @param b 第二个数据点
 * @return 距离值
 */
static double computeDistance(const QVector<double>& a, const QVector<double>& b,
                              const QString& metric)
{
    if (a.size() != b.size() || a.isEmpty()) return 0.0;

    if (metric == "manhattan") {
        double dist = 0.0;
        for (int i = 0; i < a.size(); ++i) {
            dist += std::fabs(a[i] - b[i]);
        }
        return dist;
    } else if (metric == "cosine") {
        double dot = 0.0, normA = 0.0, normB = 0.0;
        for (int i = 0; i < a.size(); ++i) {
            dot += a[i] * b[i];
            normA += a[i] * a[i];
            normB += b[i] * b[i];
        }
        const double denom = std::sqrt(normA) * std::sqrt(normB);
        return (denom > 1e-12) ? 1.0 - dot / denom : 1.0;
    } else {
        // 欧氏距离
        double dist = 0.0;
        for (int i = 0; i < a.size(); ++i) {
            dist += (a[i] - b[i]) * (a[i] - b[i]);
        }
        return std::sqrt(dist);
    }
}

/**
 * @brief 拟合数据，执行层次聚类
 *
 * 凝聚型层次聚类流程:
 * 1. 初始化每个点为一个簇
 * 2. 计算所有簇间距离矩阵
 * 3. 合并距离最近的两个簇
 * 4. 更新距离矩阵
 * 5. 重复直到只剩一个簇
 *
 * @param data 输入数据矩阵，每行为一个数据点
 */
void HierarchicalCluster10::fit(const QVector<QVector<double>>& data)
{
    if (data.size() < 2) return;

    QElapsedTimer timer;
    timer.start();

    const int n = data.size();

    // 初始化: 每个点为一个簇
    QVector<QVector<int>> clusters(n);
    for (int i = 0; i < n; ++i) {
        clusters[i].append(i);
    }

    // 计算初始距离矩阵
    QVector<QVector<double>> distMatrix(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            distMatrix[i][j] = computeDistance(data[i], data[j], m_metric);
            distMatrix[j][i] = distMatrix[i][j];
        }
    }

    int mergeCount = 0;

    // 凝聚合并
    for (int step = 0; step < n - 1; ++step) {
        // 找到距离最近的两个簇
        double minDist = 1e18;
        int mergeI = 0, mergeJ = 1;

        for (int i = 0; i < clusters.size(); ++i) {
            if (clusters[i].isEmpty()) continue;
            for (int j = i + 1; j < clusters.size(); ++j) {
                if (clusters[j].isEmpty()) continue;
                if (distMatrix[i][j] < minDist) {
                    minDist = distMatrix[i][j];
                    mergeI = i;
                    mergeJ = j;
                }
            }
        }

        // 合并簇J到簇I
        clusters[mergeI].append(clusters[mergeJ]);
        clusters[mergeJ].clear();
        mergeCount++;

        // 更新距离矩阵
        for (int k = 0; k < n; ++k) {
            if (k == mergeI || clusters[k].isEmpty()) continue;
            // 根据链接策略更新距离
            if (m_linkage == "single") {
                distMatrix[mergeI][k] = qMin(distMatrix[mergeI][k], distMatrix[mergeJ][k]);
            } else if (m_linkage == "complete") {
                distMatrix[mergeI][k] = qMax(distMatrix[mergeI][k], distMatrix[mergeJ][k]);
            } else {
                // average
                distMatrix[mergeI][k] = (distMatrix[mergeI][k] + distMatrix[mergeJ][k]) / 2.0;
            }
            distMatrix[k][mergeI] = distMatrix[mergeI][k];
        }
    }

    // 更新统计信息
    m_stats.totalPoints += n;
    m_stats.totalMerges += mergeCount;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, mergeCount);

    emit clusteringCompleted(1);
}

/**
 * @brief 重置所有统计信息
 */
void HierarchicalCluster10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
