#include "DBSCAN13.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化DBSCAN聚类器
 * @param parent 父对象指针
 */
DBSCAN13::DBSCAN13(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置邻域半径
 * @param epsilon epsilon邻域半径
 */
void DBSCAN13::setEpsilon(double epsilon)
{
    m_epsilon = qMax(0.001, epsilon);
}

/**
 * @brief 设置最小核心点邻居数
 * @param minPts 成为核心点所需的最小邻居数
 */
void DBSCAN13::setMinPts(int minPts)
{
    m_minPts = qMax(1, minPts);
}

/**
 * @brief 查找epsilon邻域内的所有点
 * @param pointIdx 中心点索引
 * @param data 数据集
 * @param epsilon 邻域半径
 * @return 邻域内点的索引列表
 */
static QVector<int> regionQuery(int pointIdx,
                                 const QVector<QVector<double>>& data,
                                 double epsilon)
{
    QVector<int> neighbors;
    int dim = data[pointIdx].size();
    for (int i = 0; i < data.size(); ++i) {
        double dist = 0.0;
        for (int d = 0; d < dim; ++d) {
            double diff = data[pointIdx][d] - data[i][d];
            dist += diff * diff;
        }
        if (std::sqrt(dist) <= epsilon) {
            neighbors.append(i);
        }
    }
    return neighbors;
}

/**
 * @brief 对输入数据执行DBSCAN聚类
 *
 * 1. 计算每个点的epsilon邻域
 * 2. 邻居数>=minPts的点标记为核心点
 * 3. 从未访问的核心点开始扩展簇(密度可达)
 * 4. 不属于任何簇的点标记为噪声
 *
 * @param data 输入数据集
 */
void DBSCAN13::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalClustered++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClustered;
        emit clusteringCompleted(0, 0);
        return;
    }

    const int n = data.size();
    QVector<int> labels(n, -1); /* -1=未分类, -2=噪声 */
    int clusterId = 0;
    int noiseCount = 0;

    for (int i = 0; i < n; ++i) {
        if (labels[i] != -1) continue;

        /* 查找epsilon邻域 */
        QVector<int> neighbors = regionQuery(i, data, m_epsilon);

        if (neighbors.size() < m_minPts) {
            /* 标记为噪声(后续可能被核心点吸收) */
            labels[i] = -2;
            continue;
        }

        /* 创建新簇 */
        labels[i] = clusterId;

        /* 扩展簇：处理所有密度可达的点 */
        QVector<int> seedSet = neighbors;
        int idx = 0;
        while (idx < seedSet.size()) {
            int q = seedSet[idx++];
            if (labels[q] == -2) {
                labels[q] = clusterId; /* 噪声点归入簇 */
            }
            if (labels[q] != -1) continue;
            labels[q] = clusterId;

            QVector<int> qNeighbors = regionQuery(q, data, m_epsilon);
            if (qNeighbors.size() >= m_minPts) {
                for (int nn : qNeighbors) {
                    if (labels[nn] == -1 || labels[nn] == -2) {
                        seedSet.append(nn);
                    }
                }
            }
        }

        clusterId++;
    }

    /* 统计噪声点数 */
    for (int i = 0; i < n; ++i) {
        if (labels[i] == -2) noiseCount++;
    }

    m_timeSum += timer.elapsed();
    m_stats.totalClustered++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClustered;
    emit clusteringCompleted(clusterId, noiseCount);
}

/**
 * @brief 重置统计数据
 */
void DBSCAN13::resetStatistics()
{
    m_stats.totalClustered = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
