#include "DBSCAN14.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file DBSCAN14.cpp
 * @brief DBSCAN密度聚类算法实现
 *
 * DBSCAN(Density-Based Spatial Clustering of Applications with Noise):
 * - 核心点: epsilon邻域内至少有MinPts个点
 * - 边界点: 在核心点的邻域内但自身非核心点
 * - 噪声点: 既非核心点也不在任何核心点邻域内
 */

/**
 * @brief 构造函数，初始化默认聚类参数
 * @param parent 父QObject对象指针
 */
DBSCAN14::DBSCAN14(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置邻域半径参数
 * @param epsilon epsilon邻域半径
 */
void DBSCAN14::setEpsilon(double epsilon)
{
    m_epsilon = qMax(0.001, epsilon);
}

/**
 * @brief 设置最小邻域点数
 * @param minPts 成为核心点所需的最少邻域点数
 */
void DBSCAN14::setMinPts(int minPts)
{
    m_minPts = qMax(2, minPts);
}

/**
 * @brief 计算欧氏距离
 */
static double euclideanDist(const QVector<double>& a, const QVector<double>& b)
{
    double dist = 0.0;
    for (int i = 0; i < a.size(); ++i) {
        dist += (a[i] - b[i]) * (a[i] - b[i]);
    }
    return std::sqrt(dist);
}

/**
 * @brief 对输入数据执行DBSCAN聚类
 *
 * 算法流程:
 * 1. 计算所有点对的距离
 * 2. 对每个未分类点，查询其epsilon邻域
 * 3. 如果邻域点数>=MinPts，创建新簇并扩展
 * 4. 否则标记为噪声(可能后续被边界点回收)
 *
 * @param data 输入数据矩阵
 * @return 每个点的簇标签(-1为噪声)
 */
QVector<int> DBSCAN14::fit(const QVector<QVector<double>>& data)
{
    if (data.isEmpty()) return {};

    QElapsedTimer timer;
    timer.start();

    const int n = data.size();
    QVector<int> labels(n, -1); // -1=未分类, -2=噪声
    int clusterId = 0;

    // 预计算邻域
    QVector<QVector<int>> neighbors(n);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i != j && euclideanDist(data[i], data[j]) <= m_epsilon) {
                neighbors[i].append(j);
            }
        }
    }

    // DBSCAN主循环
    for (int i = 0; i < n; ++i) {
        if (labels[i] != -1) continue; // 已分类

        // 检查是否为核心点
        if (neighbors[i].size() < m_minPts) {
            labels[i] = -2; // 暂时标记为噪声
            continue;
        }

        // 创建新簇
        labels[i] = clusterId;

        // 扩展簇: 使用队列进行广度优先扩展
        QVector<int> queue = neighbors[i];
        int qIdx = 0;

        while (qIdx < queue.size()) {
            const int j = queue[qIdx++];

            if (labels[j] == -2) {
                // 噪声点变为边界点
                labels[j] = clusterId;
            }
            if (labels[j] != -1) continue; // 已处理

            labels[j] = clusterId;

            // 如果j也是核心点，将其邻域加入队列
            if (neighbors[j].size() >= m_minPts) {
                for (int k : neighbors[j]) {
                    if (labels[k] == -1 || labels[k] == -2) {
                        queue.append(k);
                    }
                }
            }
        }

        clusterId++;
    }

    // 统计噪声点数
    int noiseCount = 0;
    for (int label : labels) {
        if (label == -2) noiseCount++;
    }

    // 更新统计信息
    m_stats.totalClustered += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalClustered / n);

    emit clusteringCompleted(clusterId, noiseCount);
    return labels;
}

/**
 * @brief 重置所有统计信息
 */
void DBSCAN14::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
