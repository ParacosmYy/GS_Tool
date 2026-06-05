#include "HierarchicalCluster9.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化层次聚类器
 * @param parent 父对象指针
 */
HierarchicalCluster9::HierarchicalCluster9(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置链接策略名称
 * @param linkage 链接策略: "single"(单链接), "complete"(全链接), "average"(平均链接)
 */
void HierarchicalCluster9::setLinkage(const QString& linkage)
{
    if (linkage == "single" || linkage == "complete" || linkage == "average") {
        m_linkage = linkage;
    }
}

/**
 * @brief 计算两个样本之间的欧氏距离
 * @param a 第一个样本
 * @param b 第二个样本
 * @return 欧氏距离值
 */
static double euclideanDist(const QVector<double>& a, const QVector<double>& b)
{
    double sum = 0.0;
    int dim = qMin(a.size(), b.size());
    for (int i = 0; i < dim; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return std::sqrt(sum);
}

/**
 * @brief 对输入数据集执行层次聚类
 *
 * 使用凝聚层次聚类算法：初始每个样本为一个簇，
 * 每次合并距离最近的两个簇直到只剩一个簇，
 * 支持single/complete/average三种链接策略。
 *
 * @param data 输入数据集，每个元素为一个样本的特征向量
 */
void HierarchicalCluster9::fit(const QVector<QVector<double>>& data)
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

    /* 初始化：每个样本自成一个簇 */
    QVector<QVector<int>> clusters(n);
    for (int i = 0; i < n; ++i) clusters[i].append(i);

    /* 计算初始距离矩阵 */
    m_distMatrix.resize(n);
    for (int i = 0; i < n; ++i) {
        m_distMatrix[i].resize(n, 0.0);
        for (int j = 0; j < n; ++j) {
            m_distMatrix[i][j] = euclideanDist(data[i], data[j]);
        }
    }

    /* 标记已合并的簇 */
    QVector<bool> active(n, true);
    int activeCount = n;

    /* 凝聚过程：每次合并最近的两个簇 */
    while (activeCount > 1) {
        double minDist = 1e18;
        int mergeI = -1, mergeJ = -1;

        /* 寻找最近的两个活跃簇 */
        for (int i = 0; i < n; ++i) {
            if (!active[i]) continue;
            for (int j = i + 1; j < n; ++j) {
                if (!active[j]) continue;
                if (m_distMatrix[i][j] < minDist) {
                    minDist = m_distMatrix[i][j];
                    mergeI = i;
                    mergeJ = j;
                }
            }
        }

        if (mergeI < 0) break;

        /* 合并簇J到簇I */
        clusters[mergeI].append(clusters[mergeJ]);
        active[mergeJ] = false;
        activeCount--;

        /* 更新距离矩阵 */
        for (int k = 0; k < n; ++k) {
            if (!active[k] || k == mergeI) continue;
            double newDist;
            if (m_linkage == "single") {
                newDist = qMin(m_distMatrix[mergeI][k], m_distMatrix[mergeJ][k]);
            } else if (m_linkage == "complete") {
                newDist = qMax(m_distMatrix[mergeI][k], m_distMatrix[mergeJ][k]);
            } else {
                newDist = (m_distMatrix[mergeI][k] + m_distMatrix[mergeJ][k]) / 2.0;
            }
            m_distMatrix[mergeI][k] = newDist;
            m_distMatrix[k][mergeI] = newDist;
        }
    }

    m_timeSum += timer.elapsed();
    m_stats.totalClusterings++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;
    emit clusteringCompleted(n);
}

/**
 * @brief 获取距离矩阵
 * @return 距离矩阵的二维向量
 */
QVector<QVector<double>> HierarchicalCluster9::distanceMatrix() const
{
    return m_distMatrix;
}

/**
 * @brief 重置统计数据
 */
void HierarchicalCluster9::resetStatistics()
{
    m_stats.totalClusterings = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
