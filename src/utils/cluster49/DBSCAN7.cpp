/**
 * @file DBSCAN7.cpp
 * @brief DBSCAN密度聚类算法实现
 *
 * 基于密度的空间聚类应用（DBSCAN）算法，通过epsilon邻域和minPts参数
 * 自动发现任意形状的簇，并能识别噪声点。支持多维数据聚类。
 *
 * 算法流程:
 * 1. 对每个未访问点执行邻域查询
 * 2. 若邻域内点数 >= minPts，创建新簇并扩展
 * 3. 扩展过程中递归地将密度可达点加入当前簇
 * 4. 不属于任何簇的点标记为噪声
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/cluster49/DBSCAN7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject对象指针
 */
DBSCAN7::DBSCAN7(QObject* parent)
    : QObject(parent)
    , m_eps(0.5)
    , m_minPts(5)
    , m_numClusters(0)
    , m_timeSum(0.0)
{
}

/**
 * @brief 设置epsilon邻域半径
 * @param eps 邻域半径，必须为正值，控制簇的紧密程度
 */
void DBSCAN7::setEpsilon(double eps)
{
    m_eps = qMax(1e-10, eps);
}

/**
 * @brief 设置最小点数阈值
 * @param minPts 形成核心点所需的最小邻域点数，必须 >= 1
 */
void DBSCAN7::setMinPoints(int minPts)
{
    m_minPts = qMax(1, minPts);
}

/**
 * @brief 执行DBSCAN聚类
 *
 * 对输入的多维数据执行基于密度的聚类分析。每个数据点被标记为:
 * - >= 0: 所属簇编号
 * - -1: 噪声点（不属于任何簇）
 *
 * @param data 输入数据，每个QVector<double>代表一个多维样本
 * @return 聚类标签数组，长度与data相同。标签值 >= 0为簇编号，-1为噪声
 */
QVector<int> DBSCAN7::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    const int n = data.size();
    QVector<int> labels(n, -2);  ///< -2=未访问, -1=噪声, >=0=簇编号
    m_numClusters = 0;

    if (n == 0) {
        emit clusteringCompleted(0, 0);
        return labels;
    }

    /* 主循环: 遍历所有未访问的数据点 */
    for (int i = 0; i < n; ++i) {
        if (labels[i] != -2) {
            continue;  ///< 已处理过的点跳过
        }

        /* 查询epsilon邻域内的所有点 */
        QVector<int> neighbors = rangeQuery(data, i);

        if (neighbors.size() < m_minPts) {
            /* 邻域点数不足，标记为噪声 */
            labels[i] = -1;
            continue;
        }

        /* 创建新簇，从当前核心点开始扩展 */
        int clusterId = m_numClusters;
        m_numClusters++;
        labels[i] = clusterId;

        /* 使用种子集合进行簇扩展 */
        QVector<int> seedSet = neighbors;
        int seedIdx = 0;

        while (seedIdx < seedSet.size()) {
            int currentPt = seedSet[seedIdx];
            seedIdx++;

            /* 噪声点可以重新归类到当前簇 */
            if (labels[currentPt] == -1) {
                labels[currentPt] = clusterId;
            }

            /* 跳过已处理的点 */
            if (labels[currentPt] != -2) {
                continue;
            }

            /* 标记当前点属于当前簇 */
            labels[currentPt] = clusterId;

            /* 查询当前点的邻域，判断是否为核心点 */
            QVector<int> currentNeighbors = rangeQuery(data, currentPt);

            if (currentNeighbors.size() >= m_minPts) {
                /* 当前点为核心点，将其邻域点加入种子集合 */
                for (int neighborIdx : currentNeighbors) {
                    if (labels[neighborIdx] == -2 || labels[neighborIdx] == -1) {
                        seedSet.append(neighborIdx);
                    }
                }
            }
        }
    }

    /* 更新统计信息 */
    double elapsed = timer.elapsed();
    m_stats.totalClusterings++;
    m_stats.totalPoints += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;

    emit clusteringCompleted(n, m_numClusters);
    return labels;
}

/**
 * @brief 从指定点开始扩展簇
 *
 * 递归地将密度可达的点加入指定簇。遍历邻域内的所有点，
 * 若邻域点数满足minPts阈值则继续向外扩展。
 *
 * @param data 输入数据集
 * @param pt 起始点索引
 * @param labels 聚类标签数组（会被修改）
 * @param cluster 当前簇编号
 * @return 被加入当前簇的所有点索引
 */
QVector<int> DBSCAN7::expandCluster(const QVector<QVector<double>>& data,
                                     int pt,
                                     QVector<int>& labels,
                                     int cluster)
{
    QVector<int> clusterPoints;
    clusterPoints.append(pt);
    labels[pt] = cluster;

    QVector<int> neighbors = rangeQuery(data, pt);

    if (neighbors.size() < m_minPts) {
        return clusterPoints;
    }

    /* 使用工作列表进行广度优先扩展 */
    QList<int> workList;
    for (int idx : neighbors) {
        if (labels[idx] == -2 || labels[idx] == -1) {
            workList.append(idx);
        }
    }

    while (!workList.isEmpty()) {
        int currentPt = workList.takeFirst();

        if (labels[currentPt] == -1) {
            labels[currentPt] = cluster;
        }
        if (labels[currentPt] != -2) {
            continue;
        }

        labels[currentPt] = cluster;
        clusterPoints.append(currentPt);

        QVector<int> currentNeighbors = rangeQuery(data, currentPt);
        if (currentNeighbors.size() >= m_minPts) {
            for (int neighborIdx : currentNeighbors) {
                if (labels[neighborIdx] == -2 || labels[neighborIdx] == -1) {
                    workList.append(neighborIdx);
                }
            }
        }
    }

    return clusterPoints;
}

/**
 * @brief 查询指定点的epsilon邻域
 *
 * 计算目标点与数据集中所有其他点的欧几里得距离，
 * 返回距离小于epsilon的点索引列表。
 *
 * @param data 输入数据集
 * @param pt 目标点索引
 * @return 邻域内所有点的索引列表（包含目标点自身）
 */
QVector<int> DBSCAN7::rangeQuery(const QVector<QVector<double>>& data, int pt) const
{
    QVector<int> neighbors;
    const QVector<double>& target = data[pt];
    const int dims = target.size();

    for (int i = 0; i < data.size(); ++i) {
        double distSq = 0.0;
        const QVector<double>& candidate = data[i];

        /* 计算欧几里得距离的平方，避免开根号 */
        for (int d = 0; d < dims && d < candidate.size(); ++d) {
            double diff = target[d] - candidate[d];
            distSq += diff * diff;
        }

        /* 处理维度不匹配的额外维度 */
        if (candidate.size() > dims) {
            for (int d = dims; d < candidate.size(); ++d) {
                distSq += candidate[d] * candidate[d];
            }
        } else if (candidate.size() < dims) {
            for (int d = candidate.size(); d < dims; ++d) {
                distSq += target[d] * target[d];
            }
        }

        if (distSq <= m_eps * m_eps) {
            neighbors.append(i);
        }
    }

    return neighbors;
}

/**
 * @brief 重置所有统计计数器
 *
 * 将聚类次数、总点数、平均处理时间等统计指标归零。
 */
void DBSCAN7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
