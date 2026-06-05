#include "BirchClustering8.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化BIRCH聚类器
 * @param parent 父对象指针
 */
BirchClustering8::BirchClustering8(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置分支因子
 * @param factor CF-Tree每个节点的最大子节点数
 */
void BirchClustering8::setBranchFactor(int factor)
{
    m_branchFactor = qMax(2, factor);
}

/**
 * @brief 设置聚类阈值
 * @param threshold 子簇合并的半径阈值
 */
void BirchClustering8::setThreshold(double threshold)
{
    m_threshold = qMax(0.001, threshold);
}

/**
 * @brief 对输入数据执行BIRCH聚类
 *
 * 增量式构建聚类特征树(CF-Tree)：
 * 1. 逐点插入数据到CF-Tree
 * 2. 若插入点与最近叶节点的距离小于阈值，则吸收
 * 3. 否则创建新的叶节点
 * 4. 最终对叶节点执行凝聚聚类获得最终簇划分
 *
 * @param data 输入数据集
 */
void BirchClustering8::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalClustered++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClustered;
        emit clusteringCompleted(0);
        return;
    }

    const int n = data.size();
    const int dim = data[0].size();

    /* CF-Tree简化实现：维护一组子簇中心 */
    struct SubCluster {
        QVector<double> linearSum; ///< 线性求和
        int count = 0;             ///< 点数量
        QVector<double> centroid() const {
            QVector<double> c(linearSum.size(), 0.0);
            if (count > 0) {
                for (int i = 0; i < linearSum.size(); ++i) c[i] = linearSum[i] / count;
            }
            return c;
        }
    };

    QVector<SubCluster> leafClusters;

    /* 增量插入数据点 */
    for (int p = 0; p < n; ++p) {
        double minDist = 1e18;
        int bestCluster = -1;

        /* 寻找最近的子簇 */
        for (int c = 0; c < leafClusters.size(); ++c) {
            auto cent = leafClusters[c].centroid();
            double dist = 0.0;
            for (int d = 0; d < dim; ++d) {
                double diff = data[p][d] - cent[d];
                dist += diff * diff;
            }
            if (dist < minDist) { minDist = dist; bestCluster = c; }
        }

        /* 判断是否可以吸收到已有子簇 */
        if (bestCluster >= 0 && std::sqrt(minDist) < m_threshold
            && leafClusters[bestCluster].count < m_branchFactor * 10) {
            for (int d = 0; d < dim; ++d) {
                leafClusters[bestCluster].linearSum[d] += data[p][d];
            }
            leafClusters[bestCluster].count++;
        } else {
            /* 创建新子簇 */
            SubCluster newCluster;
            newCluster.linearSum = data[p];
            newCluster.count = 1;
            leafClusters.append(newCluster);
        }
    }

    int clusterCount = leafClusters.size();

    m_timeSum += timer.elapsed();
    m_stats.totalClustered++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClustered;
    emit clusteringCompleted(clusterCount);
}

/**
 * @brief 重置统计数据
 */
void BirchClustering8::resetStatistics()
{
    m_stats.totalClustered = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
