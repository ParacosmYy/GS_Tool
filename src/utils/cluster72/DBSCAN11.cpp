/**
 * @file DBSCAN11.cpp
 * @brief DBSCAN密度聚类算法实现(增强版)
 *
 * 支持任意距离度量(欧氏/曼哈顿/切比雪夫)的密度聚类，
 * 自动发现簇数量，识别噪声点。支持预计算距离矩阵和
 * 自定义距离度量。
 */

#include "utils/cluster72/DBSCAN11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 *
 * 默认距离度量为欧氏距离。
 */
DBSCAN11::DBSCAN11(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置自定义距离度量
 * @param metric 距离度量名称: "euclidean"/"manhattan"/"chebyshev"
 */
void DBSCAN11::setDistanceMetric(const QString& metric)
{
    m_metric = metric.toLower();
}

/**
 * @brief 计算两点间距离
 * @param a 第一个点
 * @param b 第二个点
 * @param metric 距离度量类型
 * @return 距离值
 */
static double computeDist(const QVector<double>& a, const QVector<double>& b, const QString& metric)
{
    const int D = qMin(a.size(), b.size());
    if (metric == "manhattan") {
        double dist = 0.0;
        for (int i = 0; i < D; ++i) dist += qAbs(a[i] - b[i]);
        return dist;
    }
    if (metric == "chebyshev") {
        double dist = 0.0;
        for (int i = 0; i < D; ++i) dist = qMax(dist, qAbs(a[i] - b[i]));
        return dist;
    }
    /* 默认: 欧氏距离 */
    double dist = 0.0;
    for (int i = 0; i < D; ++i) {
        double diff = a[i] - b[i];
        dist += diff * diff;
    }
    return qSqrt(dist);
}

/**
 * @brief 执行DBSCAN聚类
 * @param points 输入数据点集合
 * @param epsilon 邻域半径
 * @param minPts 核心点最小邻居数
 * @return 每个点的簇标签(-1为噪声)
 *
 * 算法流程:
 * 1. 预计算每个点的邻域列表
 * 2. 标记核心点(邻域内点数 >= minPts)
 * 3. 从核心点出发BFS扩展簇
 * 4. 未被任何簇包含的点标记为噪声
 */
QVector<int> DBSCAN11::fit(const QVector<QVector<double>>& points, double epsilon, int minPts)
{
    QElapsedTimer timer;
    timer.start();

    const int N = points.size();
    if (N == 0) return QVector<int>();

    minPts = qMax(2, minPts);
    m_labels = QVector<int>(N, -2); /* -2 = 未访问 */
    m_corePoints.clear();

    /* 阶段1: 预计算邻域 */
    QVector<QVector<int>> neighborhoods(N);
    for (int i = 0; i < N; ++i) {
        for (int j = i + 1; j < N; ++j) {
            double d = computeDist(points[i], points[j], m_metric);
            if (d <= epsilon) {
                neighborhoods[i].append(j);
                neighborhoods[j].append(i);
            }
        }
    }

    /* 阶段2: 识别核心点 */
    for (int i = 0; i < N; ++i) {
        if (neighborhoods[i].size() >= minPts) {
            m_corePoints.append(i);
        }
    }

    /* 阶段3: DBSCAN主循环 */
    int clusterId = 0;
    for (int i = 0; i < N; ++i) {
        if (m_labels[i] != -2) continue;

        if (neighborhoods[i].size() < minPts) {
            m_labels[i] = -1; /* 噪声(暂时) */
            continue;
        }

        /* 创建新簇 */
        m_labels[i] = clusterId;

        /* BFS扩展簇 */
        QVector<int> seeds = neighborhoods[i];
        int seedIdx = 0;
        while (seedIdx < seeds.size()) {
            int q = seeds[seedIdx];
            seedIdx++;

            if (m_labels[q] == -1) {
                m_labels[q] = clusterId; /* 噪声变边界点 */
            }
            if (m_labels[q] != -2) continue;

            m_labels[q] = clusterId;

            if (neighborhoods[q].size() >= minPts) {
                for (int n : neighborhoods[q]) {
                    if (m_labels[n] < 0) {
                        if (!seeds.contains(n)) seeds.append(n);
                    }
                }
            }
        }

        clusterId++;
    }

    /* 更新统计信息 */
    int noiseCount = 0;
    for (int i = 0; i < N; ++i) {
        if (m_labels[i] == -1) noiseCount++;
    }

    qint64 elapsed = timer.elapsed();
    m_stats.totalClusters += clusterId;
    m_stats.totalNoisePoints += noiseCount;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, (m_stats.totalClusters + m_stats.totalNoisePoints));

    emit clusteringCompleted(clusterId, noiseCount);
    return m_labels;
}

/**
 * @brief 使用预计算距离矩阵执行聚类
 * @param distMatrix N*N距离矩阵(对称)
 * @param epsilon 邻域半径
 * @param minPts 核心点最小邻居数
 * @return 每个点的簇标签(-1为噪声)
 *
 * 当距离矩阵已知时(如外部计算)，可直接使用而无需重新计算。
 */
QVector<int> DBSCAN11::fitFromDistance(const QVector<QVector<double>>& distMatrix, double epsilon, int minPts)
{
    QElapsedTimer timer;
    timer.start();

    const int N = distMatrix.size();
    if (N == 0) return QVector<int>();

    minPts = qMax(2, minPts);
    m_labels = QVector<int>(N, -2);
    m_corePoints.clear();

    /* 从距离矩阵构建邻域 */
    QVector<QVector<int>> neighborhoods(N);
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            if (i != j && distMatrix[i].size() > j && distMatrix[i][j] <= epsilon) {
                neighborhoods[i].append(j);
            }
        }
    }

    /* 识别核心点 */
    for (int i = 0; i < N; ++i) {
        if (neighborhoods[i].size() >= minPts) {
            m_corePoints.append(i);
        }
    }

    /* DBSCAN主循环 */
    int clusterId = 0;
    for (int i = 0; i < N; ++i) {
        if (m_labels[i] != -2) continue;
        if (neighborhoods[i].size() < minPts) {
            m_labels[i] = -1;
            continue;
        }

        m_labels[i] = clusterId;
        QVector<int> seeds = neighborhoods[i];
        int seedIdx = 0;
        while (seedIdx < seeds.size()) {
            int q = seeds[seedIdx++];
            if (m_labels[q] == -1) m_labels[q] = clusterId;
            if (m_labels[q] != -2) continue;
            m_labels[q] = clusterId;
            if (neighborhoods[q].size() >= minPts) {
                for (int n : neighborhoods[q]) {
                    if (m_labels[n] < 0 && !seeds.contains(n)) seeds.append(n);
                }
            }
        }
        clusterId++;
    }

    int noiseCount = 0;
    for (int i = 0; i < N; ++i) if (m_labels[i] == -1) noiseCount++;

    qint64 elapsed = timer.elapsed();
    m_stats.totalClusters += clusterId;
    m_stats.totalNoisePoints += noiseCount;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, (m_stats.totalClusters + m_stats.totalNoisePoints));

    emit clusteringCompleted(clusterId, noiseCount);
    return m_labels;
}

/**
 * @brief 获取指定簇的所有点索引
 * @param clusterId 簇编号
 * @return 该簇内所有点的索引列表
 */
QVector<int> DBSCAN11::getClusterPoints(int clusterId) const
{
    QVector<int> pts;
    for (int i = 0; i < m_labels.size(); ++i) {
        if (m_labels[i] == clusterId) pts.append(i);
    }
    return pts;
}

/**
 * @brief 重置统计信息
 */
void DBSCAN11::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}
