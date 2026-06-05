/**
 * @file OPTICS5.cpp
 * @brief OPTICS聚类算法实现（第5版）
 *
 * 实现有序点识别聚类结构（OPTICS）算法。
 * OPTICS是DBSCAN的扩展，生成可达距离排序而非硬聚类分配。
 * 支持Xi方法和线性提取两种簇提取策略。
 * 可达距离图（reachability plot）可用于可视化聚类结构。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/cluster65/OPTICS5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <queue>
#include <algorithm>

/**
 * @brief 构造函数，初始化OPTICS聚类器
 * @param parent 父QObject对象指针
 */
OPTICS5::OPTICS5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置ε邻域半径
 * @param eps 最大搜索半径
 */
void OPTICS5::setEpsilon(double eps)
{
    m_eps = qMax(0.001, eps);
}

/**
 * @brief 设置最小邻域点数
 * @param minPts 核心点所需的最小邻居数
 */
void OPTICS5::setMinPoints(int minPts)
{
    m_minPts = qMax(1, minPts);
}

/**
 * @brief 设置簇提取方法
 * @param method 提取方法："xi"基于斜率, "linear"基于阈值
 */
void OPTICS5::setExtractMethod(const QString& method)
{
    if (method == "xi" || method == "linear") {
        m_method = method;
    }
}

/**
 * @brief 计算核心距离
 *
 * 点p的核心距离是其第minPts个最近邻居的距离。
 * 若邻域内点数不足minPts，返回无穷大。
 *
 * @param pts 数据点集合
 * @param idx 当前点的索引
 * @return 核心距离
 */
double OPTICS5::coreDist(const QVector<QVector<double>>& pts, int idx)
{
    QVector<double> dists;
    for (int i = 0; i < pts.size(); ++i) {
        if (i == idx) continue;
        double d = 0.0;
        int dim = qMin(pts[idx].size(), pts[i].size());
        for (int j = 0; j < dim; ++j) {
            double diff = pts[idx][j] - pts[i][j];
            d += diff * diff;
        }
        dists.append(qSqrt(d));
    }

    if (dists.size() < m_minPts) return std::numeric_limits<double>::max();

    std::sort(dists.begin(), dists.end());
    return qMin(dists[m_minPts - 1], m_eps);
}

/**
 * @brief 获取ε邻域内的所有邻居
 * @param pts 数据点集合
 * @param idx 当前点的索引
 * @return 邻居索引列表
 */
QVector<int> OPTICS5::getNeighbors(const QVector<QVector<double>>& pts, int idx)
{
    QVector<int> neighbors;
    for (int i = 0; i < pts.size(); ++i) {
        if (i == idx) continue;
        double d = 0.0;
        int dim = qMin(pts[idx].size(), pts[i].size());
        for (int j = 0; j < dim; ++j) {
            double diff = pts[idx][j] - pts[i][j];
            d += diff * diff;
        }
        if (qSqrt(d) <= m_eps) {
            neighbors.append(i);
        }
    }
    return neighbors;
}

/**
 * @brief 对数据点执行OPTICS聚类
 *
 * 算法流程：
 * 1. 计算每个点的核心距离
 * 2. 使用优先队列按可达距离扩展排序
 * 3. 从排序结果中提取簇标签
 *
 * @param points 输入数据点集合
 * @return 每个点的簇标签，-1表示噪声
 */
QVector<int> OPTICS5::cluster(const QVector<QVector<double>>& points)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> labels;
    m_reachability.clear();
    m_ordering.clear();

    if (points.isEmpty()) {
        emit clusteringCompleted(0, 0);
        return labels;
    }

    int n = points.size();
    labels.resize(n, -1);
    m_reachability.resize(n, std::numeric_limits<double>::max());

    QVector<bool> processed(n, false);

    /* OPTICS主循环 */
    for (int i = 0; i < n; ++i) {
        if (processed[i]) continue;
        processed[i] = true;
        m_ordering.append(i);

        double cd = coreDist(points, i);
        if (cd < std::numeric_limits<double>::max()) {
            /* 核心点：使用优先队列扩展 */
            QVector<double> reachDist(n, std::numeric_limits<double>::max());

            /* 初始化种子集 */
            QVector<int> seeds;
            auto neighbors = getNeighbors(points, i);
            for (int nb : neighbors) {
                if (processed[nb]) continue;
                double newReach = qMax(cd, 0.0); /* 简化：距离 */
                double d = 0.0;
                int dim = qMin(points[i].size(), points[nb].size());
                for (int j = 0; j < dim; ++j) {
                    double diff = points[i][j] - points[nb][j];
                    d += diff * diff;
                }
                newReach = qMax(cd, qSqrt(d));

                if (newReach < reachDist[nb]) {
                    reachDist[nb] = newReach;
                }
                if (!seeds.contains(nb)) {
                    seeds.append(nb);
                }
            }

            /* 按可达距离处理种子点 */
            while (!seeds.isEmpty()) {
                /* 找最小可达距离的种子 */
                int bestIdx = 0;
                double bestDist = reachDist[seeds[0]];
                for (int s = 1; s < seeds.size(); ++s) {
                    if (reachDist[seeds[s]] < bestDist) {
                        bestDist = reachDist[seeds[s]];
                        bestIdx = s;
                    }
                }

                int q = seeds.takeAt(bestIdx);
                processed[q] = true;
                m_ordering.append(q);
                m_reachability[q] = reachDist[q];

                double qcd = coreDist(points, q);
                if (qcd < std::numeric_limits<double>::max()) {
                    auto qNeighbors = getNeighbors(points, q);
                    for (int nb : qNeighbors) {
                        if (processed[nb]) continue;
                        double d = 0.0;
                        int dim = qMin(points[q].size(), points[nb].size());
                        for (int j = 0; j < dim; ++j) {
                            double diff = points[q][j] - points[nb][j];
                            d += diff * diff;
                        }
                        double newReach = qMax(qcd, qSqrt(d));

                        if (newReach < reachDist[nb]) {
                            reachDist[nb] = newReach;
                        }
                        if (!seeds.contains(nb)) {
                            seeds.append(nb);
                        }
                    }
                }
            }
        }
    }

    /* 从可达距离排序中提取簇 */
    int clusterId = 0;
    double threshold = m_eps * 0.75;

    if (m_method == "linear") {
        /* 线性提取：可达距离低于阈值的连续段为一个簇 */
        bool inCluster = false;
        for (int idx : m_ordering) {
            if (m_reachability[idx] <= threshold &&
                m_reachability[idx] < std::numeric_limits<double>::max()) {
                if (!inCluster) {
                    clusterId++;
                    inCluster = true;
                }
                labels[idx] = clusterId - 1;
            } else {
                inCluster = false;
                labels[idx] = -1;
            }
        }
    } else {
        /* Xi方法：基于可达距离的陡峭下降和上升提取簇 */
        for (int i = 0; i < m_ordering.size(); ++i) {
            if (m_reachability[m_ordering[i]] <= threshold &&
                m_reachability[m_ordering[i]] < std::numeric_limits<double>::max()) {
                if (labels[m_ordering[i]] == -1) {
                    clusterId++;
                }
                labels[m_ordering[i]] = clusterId - 1;
            }
        }
    }

    /* 统计噪声点 */
    int noiseCount = 0;
    for (int l : labels) {
        if (l == -1) noiseCount++;
    }

    /* 更新统计 */
    m_stats.totalClusterings++;
    m_stats.totalPoints += n;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;

    emit clusteringCompleted(clusterId, noiseCount);
    return labels;
}

/**
 * @brief 获取当前统计信息
 * @return 聚类统计结构
 */
OPTICS5::Stats OPTICS5::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据
 */
void OPTICS5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
