#include "MeanShift7.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化均值漂移聚类器
 * @param parent 父对象指针
 */
MeanShift7::MeanShift7(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置核函数带宽参数
 * @param bandwidth 高斯核带宽，决定聚类粒度
 */
void MeanShift7::setBandwidth(double bandwidth)
{
    m_bandwidth = qMax(0.01, bandwidth);
}

/**
 * @brief 设置最大迭代次数
 * @param maxIter 最大漂移迭代次数
 */
void MeanShift7::setMaxIter(int maxIter)
{
    m_maxIter = qMax(1, maxIter);
}

/**
 * @brief 对输入数据集执行均值漂移聚类
 *
 * 对每个样本点执行均值漂移迭代：在高斯核权重下
 * 计算邻域加权均值作为新的中心位置，直到收敛。
 * 收敛到相同位置(距离<带宽/2)的点归为同一簇。
 *
 * @param data 输入数据集
 */
void MeanShift7::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalClusterings++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;
        emit clusteringCompleted(0);
        return;
    }

    const int n = data.size();
    const int dim = data[0].size();
    const double h2 = m_bandwidth * m_bandwidth;

    /* 初始化：每个数据点作为起始中心 */
    QVector<QVector<double>> centers = data;

    /* 对每个中心执行均值漂移迭代 */
    for (int i = 0; i < n; ++i) {
        for (int iter = 0; iter < m_maxIter; ++iter) {
            QVector<double> newCenter(dim, 0.0);
            double totalWeight = 0.0;

            /* 计算高斯加权均值 */
            for (int j = 0; j < n; ++j) {
                double dist2 = 0.0;
                for (int d = 0; d < dim; ++d) {
                    double diff = centers[i][d] - data[j][d];
                    dist2 += diff * diff;
                }
                double weight = std::exp(-dist2 / (2.0 * h2));
                for (int d = 0; d < dim; ++d) {
                    newCenter[d] += weight * data[j][d];
                }
                totalWeight += weight;
            }

            /* 归一化得到新中心 */
            if (totalWeight > 1e-15) {
                for (int d = 0; d < dim; ++d) {
                    newCenter[d] /= totalWeight;
                }
            }

            /* 检查收敛 */
            double shift = 0.0;
            for (int d = 0; d < dim; ++d) {
                double diff = newCenter[d] - centers[i][d];
                shift += diff * diff;
            }
            shift = std::sqrt(shift);

            centers[i] = newCenter;
            if (shift < m_bandwidth * 0.01) break;
        }
    }

    /* 合并收敛到相同位置的簇 */
    QVector<int> clusterLabels(n, -1);
    int numClusters = 0;
    double mergeThreshold = m_bandwidth / 2.0;

    for (int i = 0; i < n; ++i) {
        if (clusterLabels[i] >= 0) continue;
        clusterLabels[i] = numClusters;
        for (int j = i + 1; j < n; ++j) {
            if (clusterLabels[j] >= 0) continue;
            double dist = 0.0;
            for (int d = 0; d < dim; ++d) {
                double diff = centers[i][d] - centers[j][d];
                dist += diff * diff;
            }
            if (std::sqrt(dist) < mergeThreshold) {
                clusterLabels[j] = numClusters;
            }
        }
        numClusters++;
    }

    m_timeSum += timer.elapsed();
    m_stats.totalClusterings++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;
    emit clusteringCompleted(n);
}

/**
 * @brief 重置统计数据
 */
void MeanShift7::resetStatistics()
{
    m_stats.totalClusterings = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
