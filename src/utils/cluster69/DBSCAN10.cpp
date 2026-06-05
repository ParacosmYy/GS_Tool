/**
 * @file DBSCAN10.cpp
 * @brief DBSCAN密度聚类算法实现
 *
 * 实现基于密度的空间聚类应用DBSCAN(Density-Based Spatial
 * Clustering of Applications with Noise)，支持自动发现簇数和
 * 噪声点识别。
 */

#include "utils/cluster69/DBSCAN10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
DBSCAN10::DBSCAN10(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置邻域半径
 * @param eps epsilon半径
 */
void DBSCAN10::setEpsilon(double eps)
{
    m_eps = qBound(0.001, eps, 1000.0);
}

/**
 * @brief 设置核心点最小邻居数
 * @param minPts 最小点数
 */
void DBSCAN10::setMinPoints(int minPts)
{
    m_minPts = qMax(2, minPts);
}

/**
 * @brief 对数据点进行DBSCAN聚类
 * @param points 输入数据点集合
 * @return 每个点的聚类标签（-1表示噪声点）
 *
 * 算法流程:
 * 1. 预计算距离矩阵
 * 2. 对每个未分类点查找epsilon邻域
 * 3. 若邻域内点数 >= minPts，创建新簇并扩展
 * 4. 否则标记为噪声点
 */
QVector<int> DBSCAN10::cluster(const QVector<QVector<double>>& points)
{
    QElapsedTimer timer;
    timer.start();

    const int N = points.size();
    if (N == 0) return QVector<int>();

    const int D = points[0].size();
    QVector<int> labels(N, -1); /* -1 = 未分类 */
    m_numClusters = 0;
    m_noiseCount = 0;

    /* 阶段1: 预计算距离矩阵（加速邻域查询） */
    QVector<QVector<double>> distMatrix(N, QVector<double>(N, 0.0));
    for (int i = 0; i < N; ++i) {
        for (int j = i + 1; j < N; ++j) {
            double dist = 0.0;
            for (int d = 0; d < D; ++d) {
                double diff = points[i][d] - points[j][d];
                dist += diff * diff;
            }
            dist = qSqrt(dist);
            distMatrix[i][j] = dist;
            distMatrix[j][i] = dist;
        }
    }

    /* 阶段2: 预计算每个点的邻域列表 */
    QVector<QVector<int>> neighborhoods(N);
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            if (i != j && distMatrix[i][j] <= m_eps) {
                neighborhoods[i].append(j);
            }
        }
    }

    /* 阶段3: DBSCAN主循环 */
    for (int i = 0; i < N; ++i) {
        if (labels[i] != -1) continue; /* 跳过已分类点 */

        /* 查找epsilon邻域内的点 */
        const QVector<int>& neighbors = neighborhoods[i];

        if (neighbors.size() < m_minPts) {
            /* 邻域内点数不足，标记为噪声（可能后续被重新标记为边界点） */
            labels[i] = -1;
            continue;
        }

        /* 创建新簇 */
        int clusterId = m_numClusters;
        m_numClusters++;
        labels[i] = clusterId;

        /* 阶段4: 扩展簇 - 使用种子集合BFS */
        QVector<int> seeds = neighbors;
        int seedIdx = 0;
        while (seedIdx < seeds.size()) {
            int q = seeds[seedIdx];
            seedIdx++;

            /* 噪声点重新标记为边界点 */
            if (labels[q] == -1) {
                labels[q] = clusterId;
            }

            /* 跳过已分类的非噪声点 */
            if (labels[q] != -1 && q != i) continue;

            labels[q] = clusterId;

            /* 查找q的邻域 */
            const QVector<int>& qNeighbors = neighborhoods[q];

            /* 如果q是核心点，将其邻域加入种子集合 */
            if (qNeighbors.size() >= m_minPts) {
                for (int n : qNeighbors) {
                    if (labels[n] == -1) {
                        if (!seeds.contains(n)) {
                            seeds.append(n);
                        }
                    }
                }
            }
        }
    }

    /* 统计噪声点数量 */
    m_noiseCount = 0;
    for (int i = 0; i < N; ++i) {
        if (labels[i] == -1) m_noiseCount++;
    }

    /* 更新统计信息 */
    qint64 elapsed = timer.elapsed();
    m_stats.totalClusterings++;
    m_stats.totalPoints += N;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;

    emit clusteringCompleted(m_numClusters, m_noiseCount);
    return labels;
}

/**
 * @brief 重置统计信息
 */
void DBSCAN10::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}
