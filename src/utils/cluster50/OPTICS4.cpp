/**
 * @file OPTICS4.cpp
 * @brief OPTICS聚类算法实现
 *
 * OPTICS（Ordering Points To Identify the Clustering Structure）算法，
 * 是DBSCAN的改进版本，不直接产生聚类结果，而是生成可达距离排序，
 * 通过可达距离图（reachability plot）可视化数据的聚类结构。
 *
 * 与DBSCAN相比，OPTICS的优势:
 * 1. 无需精确设置epsilon参数（使用生成距离代替）
 * 2. 能发现不同密度的簇
 * 3. 可达距离图提供更丰富的聚类结构信息
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/cluster50/OPTICS4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject对象指针
 */
OPTICS4::OPTICS4(QObject* parent)
    : QObject(parent)
    , m_eps(1.0)
    , m_minPts(5)
    , m_timeSum(0.0)
{
}

/**
 * @brief 设置邻域半径上限
 * @param eps 最大邻域半径，用于限制邻域查询范围
 */
void OPTICS4::setEpsilon(double eps)
{
    m_eps = qMax(1e-10, eps);
}

/**
 * @brief 设置最小点数阈值
 * @param minPts 核心点所需的最小邻域点数，必须 >= 2
 */
void OPTICS4::setMinPoints(int minPts)
{
    m_minPts = qMax(2, minPts);
}

/**
 * @brief 执行OPTICS聚类，生成可达距离排序
 *
 * 主算法流程:
 * 1. 初始化所有点的可达距离为无穷大
 * 2. 按顺序处理每个未访问点
 * 3. 对核心点，计算其邻域内各点的可达距离并更新
 * 4. 使用优先队列按可达距离排序提取下一个处理点
 *
 * 返回的标签数组中，同一簇的点具有相同标签（>= 0），
 * 噪声点标记为 -1。同时可通过 reachabilityPlot() 获取可达距离图。
 *
 * @param data 输入数据集，每个元素为一个多维样本
 * @return 聚类标签数组，长度与输入数据相同
 */
QVector<int> OPTICS4::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    const int n = data.size();
    QVector<int> labels(n, -1);  ///< 默认标记为噪声
    m_reachability.resize(n);
    m_reachability.fill(std::numeric_limits<double>::infinity());

    if (n == 0) {
        emit clusteringCompleted(0);
        return labels;
    }

    QVector<bool> visited(n, false);
    QVector<int> order;  ///< 处理顺序
    order.reserve(n);

    /* 主循环: 按顺序处理每个未访问点 */
    for (int i = 0; i < n; ++i) {
        if (visited[i]) {
            continue;
        }

        visited[i] = true;
        order.append(i);

        /* 查询当前点的邻域 */
        QVector<int> neighbors = rangeQuery(data, i);

        if (neighbors.size() >= m_minPts) {
            /* 核心点: 计算核心距离 */
            double coreDist = computeCoreDist(data, i, neighbors);

            /* 使用优先队列按可达距离排序处理 */
            QList<int> seeds;
            updateSeeds(data, i, neighbors, coreDist, visited, seeds, labels);

            while (!seeds.isEmpty()) {
                /* 找到可达距离最小的种子点 */
                int bestIdx = 0;
                double bestReach = m_reachability[seeds[0]];
                for (int s = 1; s < seeds.size(); ++s) {
                    if (m_reachability[seeds[s]] < bestReach) {
                        bestReach = m_reachability[seeds[s]];
                        bestIdx = s;
                    }
                }

                int current = seeds.takeAt(bestIdx);
                visited[current] = true;
                order.append(current);

                QVector<int> currentNeighbors = rangeQuery(data, current);

                if (currentNeighbors.size() >= m_minPts) {
                    double curCoreDist = computeCoreDist(data, current, currentNeighbors);
                    updateSeeds(data, current, currentNeighbors, curCoreDist,
                               visited, seeds, labels);
                }
            }
        }
    }

    /* 基于可达距离的聚类提取（陡峭下降法简化版） */
    int clusterId = 0;
    double threshold = m_eps * 0.5;  ///< 阈值用于区分簇边界

    for (int i = 0; i < order.size(); ++i) {
        int idx = order[i];
        if (m_reachability[idx] == std::numeric_limits<double>::infinity()) {
            /* 检查是否为核心点 */
            QVector<int> nbrs = rangeQuery(data, idx);
            if (nbrs.size() < m_minPts) {
                labels[idx] = -1;  ///< 噪声点
                continue;
            }
        }

        if (i == 0 || m_reachability[idx] > threshold) {
            if (labels[idx] == -1) {
                clusterId++;
            }
        }

        if (m_reachability[idx] < threshold || i == 0) {
            labels[idx] = clusterId;
        } else if (labels[idx] == -1) {
            /* 根据最近邻分配 */
            labels[idx] = clusterId;
        }
    }

    /* 更新统计信息 */
    double elapsed = timer.elapsed();
    m_stats.totalClusterings++;
    m_stats.totalPoints += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;

    emit clusteringCompleted(n);
    return labels;
}

/**
 * @brief 计算核心距离
 *
 * 核心距离定义为: 使点p成为核心点所需的最小邻域半径。
 * 即第 minPts-1 近邻的距离。
 *
 * @param data 数据集
 * @param pt 目标点索引
 * @param neighbors 邻域点索引列表
 * @return 核心距离值
 */
double OPTICS4::computeCoreDist(const QVector<QVector<double>>& data,
                                 int pt,
                                 const QVector<int>& neighbors)
{
    if (neighbors.size() < m_minPts) {
        return std::numeric_limits<double>::infinity();
    }

    /* 计算所有邻域距离并排序 */
    QVector<double> distances;
    distances.reserve(neighbors.size());
    for (int nb : neighbors) {
        distances.append(dist(data[pt], data[nb]));
    }

    std::sort(distances.begin(), distances.end());

    /* 第 minPts-1 小的距离即为核心距离 */
    return distances[qMin(m_minPts - 1, distances.size() - 1)];
}

/**
 * @brief 更新种子集合的可达距离
 *
 * 对于核心点的每个未访问邻域点，计算新的可达距离:
 * reachability[q] = max(coreDist, dist(p, q))
 * 若新的可达距离更小，则更新。
 *
 * @param data 数据集
 * @param pt 当前核心点索引
 * @param neighbors 邻域点列表
 * @param coreDist 当前点的核心距离
 * @param visited 访问标记数组
 * @param seeds 种子点列表（会被修改）
 * @param labels 聚类标签数组
 */
void OPTICS4::updateSeeds(const QVector<QVector<double>>& data,
                           int pt,
                           const QVector<int>& neighbors,
                           double coreDist,
                           const QVector<bool>& visited,
                           QList<int>& seeds,
                           QVector<int>& labels)
{
    for (int nb : neighbors) {
        if (visited[nb]) {
            continue;
        }

        double newReach = qMax(coreDist, dist(data[pt], data[nb]));

        /* 更新可达距离（取较小值） */
        if (newReach < m_reachability[nb]) {
            m_reachability[nb] = newReach;

            /* 加入种子集合（若尚未在列表中） */
            if (!seeds.contains(nb)) {
                seeds.append(nb);
            }
        }
    }
}

/**
 * @brief 查询epsilon邻域内的所有点
 *
 * 计算目标点与数据集中所有点的欧几里得距离，
 * 返回距离小于等于epsilon的点索引列表。
 *
 * @param data 数据集
 * @param pt 目标点索引
 * @return 邻域点索引列表
 */
QVector<int> OPTICS4::rangeQuery(const QVector<QVector<double>>& data, int pt) const
{
    QVector<int> neighbors;
    const QVector<double>& target = data[pt];
    const int dims = target.size();

    for (int i = 0; i < data.size(); ++i) {
        double d = dist(target, data[i]);
        if (d <= m_eps) {
            neighbors.append(i);
        }
    }

    return neighbors;
}

/**
 * @brief 计算两个向量之间的欧几里得距离
 * @param a 第一个向量
 * @param b 第二个向量
 * @return 欧几里得距离
 */
double OPTICS4::dist(const QVector<double>& a, const QVector<double>& b) const
{
    double sumSq = 0.0;
    int dims = qMin(a.size(), b.size());

    for (int i = 0; i < dims; ++i) {
        double diff = a[i] - b[i];
        sumSq += diff * diff;
    }

    /* 处理维度不匹配 */
    for (int i = dims; i < a.size(); ++i) {
        sumSq += a[i] * a[i];
    }
    for (int i = dims; i < b.size(); ++i) {
        sumSq += b[i] * b[i];
    }

    return qSqrt(sumSq);
}

/**
 * @brief 重置所有统计计数器
 */
void OPTICS4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
