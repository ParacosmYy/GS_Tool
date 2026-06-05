/**
 * @file DBSCAN5.cpp
 * @brief 变密度DBSCAN实现 - 自适应Epsilon邻域聚类
 *
 * 在标准DBSCAN基础上增加局部密度自适应机制:
 * 对每个点使用其k近邻距离作为局部密度估计，
 * 动态调整Epsilon阈值以适应不同密度区域。
 */

#include "utils/cluster35/DBSCAN5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/**
 * @brief 构造函数，初始化默认参数(minPts=5, eps=0.5)
 * @param parent 父QObject
 */
DBSCAN5::DBSCAN5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置最小邻域点数
 * @param pts 核心点的最小邻居数量
 */
void DBSCAN5::setMinPoints(int pts)
{
    m_minPts = qMax(2, pts);
}

/**
 * @brief 设置全局Epsilon半径
 * @param eps 邻域搜索半径
 */
void DBSCAN5::setEpsilon(double eps)
{
    m_eps = qMax(1e-10, eps);
}

/**
 * @brief 启用/禁用变密度模式
 * @param enable true启用自适应Epsilon
 */
void DBSCAN5::setVariableDensity(bool enable)
{
    m_varDensity = enable;
}

/**
 * @brief 计算两点之间的欧氏距离
 * @param a 第一个点的坐标
 * @param b 第二个点的坐标
 * @return 欧氏距离
 */
static double euclideanDist(const QVector<double>& a, const QVector<double>& b)
{
    double sum = 0.0;
    int dim = qMin(a.size(), b.size());
    for (int i = 0; i < dim; ++i) {
        double d = a[i] - b[i];
        sum += d * d;
    }
    return qSqrt(sum);
}

/**
 * @brief 查询点p的k近邻距离(第k小距离)
 * @param points 所有点
 * @param pIdx 查询点索引
 * @param k 近邻数量
 * @return 第k近邻的距离
 */
static double kNNDistance(const QVector<QVector<double>>& points, int pIdx, int k)
{
    const int n = points.size();
    QVector<double> dists;
    dists.reserve(n - 1);

    for (int i = 0; i < n; ++i) {
        if (i == pIdx) continue;
        dists.append(euclideanDist(points[pIdx], points[i]));
    }

    std::sort(dists.begin(), dists.end());
    k = qMin(k, dists.size());
    return (k > 0) ? dists[k - 1] : 0.0;
}

/**
 * @brief 执行DBSCAN聚类
 *
 * 标准DBSCAN流程:
 * 1. 对每个未分类点，查找Epsilon邻域内的所有点
 * 2. 如果邻居数 >= minPts，创建新簇并扩展
 * 3. 否则标记为噪声(可能在后续被吸收)
 *
 * 变密度模式:
 * 对每个点使用自适应Epsilon: eps_local = median(kNN distances) * scaleFactor
 *
 * @param data 输入数据点集(每行一个点的坐标向量)
 * @return 每个点的聚类标签(-1表示噪声)
 */
QVector<int> DBSCAN5::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    const int n = data.size();
    QVector<int> labels(n, -1);
    int clusterId = 0;

    if (n == 0) {
        m_stats.totalClusterings++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;
        return labels;
    }

    /* 计算自适应Epsilon(变密度模式) */
    QVector<double> localEps(n, m_eps);
    if (m_varDensity) {
        int knnK = qMin(m_minPts, n - 1);
        for (int i = 0; i < n; ++i) {
            double knnDist = kNNDistance(data, i, knnK);
            /* 自适应Epsilon: 使用kNN距离的1.5倍作为局部邻域半径 */
            localEps[i] = qMax(knnDist * 1.5, 1e-10);
        }
    }

    /* 预计算距离矩阵(对小数据集) 或按需计算 */
    auto regionQuery = [&](int pIdx, double eps) -> QVector<int> {
        QVector<int> neighbors;
        neighbors.reserve(n / 4);
        for (int i = 0; i < n; ++i) {
            if (i == pIdx) {
                neighbors.append(i);
                continue;
            }
            double d = euclideanDist(data[pIdx], data[i]);
            if (d <= eps)
                neighbors.append(i);
        }
        return neighbors;
    };

    /* DBSCAN主循环 */
    QVector<bool> visited(n, false);

    for (int i = 0; i < n; ++i) {
        if (visited[i]) continue;
        visited[i] = true;

        double eps_i = m_varDensity ? (localEps[i] + m_eps) / 2.0 : m_eps;
        QVector<int> neighbors = regionQuery(i, eps_i);

        if (neighbors.size() < m_minPts) {
            /* 噪声点(可能在后续被吸收到某个簇) */
            continue;
        }

        /* 创建新簇 */
        labels[i] = clusterId;

        /* 扩展簇: 使用队列处理密度可达的点 */
        QVector<int> seeds = neighbors;
        int seedIdx = 0;

        while (seedIdx < seeds.size()) {
            int q = seeds[seedIdx++];

            if (!visited[q]) {
                visited[q] = true;
                double eps_q = m_varDensity ? (localEps[q] + m_eps) / 2.0 : m_eps;
                QVector<int> qNeighbors = regionQuery(q, eps_q);

                if (qNeighbors.size() >= m_minPts) {
                    /* 核心点: 将新邻居加入种子集 */
                    for (int nb : qNeighbors) {
                        if (!visited[nb] && labels[nb] < 0)
                            seeds.append(nb);
                    }
                }
            }

            /* 如果q尚未分配到任何簇，分配到当前簇 */
            if (labels[q] < 0) {
                labels[q] = clusterId;
            }
        }

        clusterId++;
    }

    /* 统计噪声点数 */
    int noise = 0;
    for (int l : labels)
        if (l < 0) noise++;

    m_stats.totalClusterings++;
    m_stats.totalPointsProcessed += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;

    emit clusteringComplete(clusterId, noise);
    return labels;
}

/**
 * @brief 重置所有统计数据
 */
void DBSCAN5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
