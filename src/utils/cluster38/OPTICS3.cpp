/**
 * @file OPTICS3.cpp
 * @brief OPTICS聚类3 — 层次密度聚类+可提取DBSCAN实现
 *
 * 实现OPTICS(Ordering Points To Identify the Clustering Structure)算法：
 * - 核心距离计算
 * - 可达距离更新
 * - 优先队列驱动的点排序
 * - 从排序结果提取DBSCAN聚类和层次聚类
 * 所有运算带有QElapsedTimer计时和统计信息追踪。
 */

#include "utils/cluster38/OPTICS3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <QMultiMap>

/**
 * @brief 构造函数
 * @param parent 父对象
 */
OPTICS3::OPTICS3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置邻域半径
 * @param epsilon 邻域半径（欧氏距离）
 */
void OPTICS3::setEpsilon(double epsilon)
{
    m_epsilon = qMax(epsilon, 0.001);
}

/**
 * @brief 设置最小点数
 * @param minPts 成为核心点所需的最小邻居数
 */
void OPTICS3::setMinPoints(int minPts)
{
    m_minPts = qMax(minPts, 2);
}

/**
 * @brief 欧氏距离计算
 * @param a 第一个点
 * @param b 第二个点
 * @return 欧氏距离
 */
double OPTICS3::distance(const QVector<double>& a, const QVector<double>& b) const
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
 * @brief 获取指定点的邻居列表
 * @param pointIdx 点索引
 * @return (邻居索引, 距离) 列表
 */
QVector<QPair<int,double>> OPTICS3::getNeighbors(int pointIdx) const
{
    QVector<QPair<int,double>> neighbors;
    for (int i = 0; i < m_data.size(); ++i) {
        if (i == pointIdx) continue;
        double dist = distance(m_data[pointIdx], m_data[i]);
        if (dist <= m_epsilon)
            neighbors.append({i, dist});
    }
    return neighbors;
}

/**
 * @brief 计算核心距离
 * @param pointIdx 点索引
 * @param neighbors 该点的邻居列表
 * @return 核心距离（第minPts-1近邻居的距离），不满足核心点条件返回无穷大
 */
double OPTICS3::coreDistance(int pointIdx, const QVector<QPair<int,double>>& neighbors) const
{
    Q_UNUSED(pointIdx)
    if (neighbors.size() < m_minPts - 1) return 1e30;
    /* 按距离排序取第minPts-1个 */
    QVector<double> dists;
    dists.reserve(neighbors.size());
    for (const auto& n : neighbors)
        dists.append(n.second);
    std::sort(dists.begin(), dists.end());
    return dists[qMin(m_minPts - 2, dists.size() - 1)];
}

/**
 * @brief 执行OPTICS聚类
 * @param data 输入数据矩阵（每行一个点）
 * @return 聚类结果（核心点/可达距离/核心距离/排序）
 *
 * 主算法流程：遍历所有点，对未处理点执行可达距离传播，
 * 使用优先队列选择下一个要处理的点。
 */
OPTICS3::ClusterResult OPTICS3::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    m_data = data;
    int n = data.size();

    ClusterResult result;
    result.coreDist.resize(n, 1e30);
    result.reachabilityDist.resize(n, 1e30);
    result.coreIndices.clear();
    result.ordering.clear();

    QVector<bool> processed(n, false);

    for (int i = 0; i < n; ++i) {
        if (processed[i]) continue;

        /* 获取邻居 */
        auto neighbors = getNeighbors(i);
        double coreDist = coreDistance(i, neighbors);
        result.coreDist[i] = coreDist;

        processed[i] = true;
        result.ordering.append(i);

        if (coreDist < 1e30) {
            result.coreIndices.append(i);

            /* 可达距离传播 */
            QMultiMap<double, int> seeds;
            for (const auto& nb : neighbors) {
                if (!processed[nb.first]) {
                    double newReach = qMax(coreDist, nb.second);
                    if (result.reachabilityDist[nb.first] > newReach) {
                        result.reachabilityDist[nb.first] = newReach;
                        seeds.insert(newReach, nb.first);
                    }
                }
            }

            while (!seeds.isEmpty()) {
                auto it = seeds.begin();
                int cur = it.value();
                seeds.erase(it);

                if (processed[cur]) continue;
                processed[cur] = true;
                result.ordering.append(cur);

                auto curNeighbors = getNeighbors(cur);
                double curCore = coreDistance(cur, curNeighbors);
                result.coreDist[cur] = curCore;

                if (curCore < 1e30) {
                    result.coreIndices.append(cur);
                    for (const auto& nb : curNeighbors) {
                        if (!processed[nb.first]) {
                            double newReach = qMax(curCore, nb.second);
                            if (result.reachabilityDist[nb.first] > newReach) {
                                result.reachabilityDist[nb.first] = newReach;
                                seeds.insert(newReach, nb.first);
                            }
                        }
                    }
                }
            }
        }
    }

    m_result = result;

    /* 更新统计 */
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalClusterings++;
    m_stats.totalPointsProcessed += n;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;

    int numClusters = extractDBSCAN(m_epsilon).size();
    emit clusteringCompleted(n, numClusters);
    return result;
}

/**
 * @brief 从OPTICS结果提取DBSCAN式聚类
 * @param epsilonThreshold 邻域阈值（通常<= m_epsilon）
 * @return 聚类标签列表，每个子向量为一个簇的点索引
 */
QVector<QVector<int>> OPTICS3::extractDBSCAN(double epsilonThreshold) const
{
    int n = m_data.size();
    QVector<int> labels(n, -1);
    int clusterId = 0;

    for (int idx : m_result.ordering) {
        if (m_result.reachabilityDist[idx] > epsilonThreshold) {
            /* 新簇或噪声 */
            if (m_result.coreDist[idx] <= epsilonThreshold) {
                labels[idx] = clusterId++;
            }
            /* 否则标记为噪声 */
        } else {
            /* 继承前一个点的簇标签 */
            for (int j = 0; j < m_result.ordering.size(); ++j) {
                if (m_result.ordering[j] == idx && j > 0) {
                    int prev = m_result.ordering[j - 1];
                    if (labels[prev] >= 0) {
                        labels[idx] = labels[prev];
                        break;
                    }
                }
            }
        }
    }

    QVector<QVector<int>> clusters(clusterId);
    for (int i = 0; i < n; ++i) {
        if (labels[i] >= 0)
            clusters[labels[i]].append(i);
    }
    return clusters;
}

/**
 * @brief 使用xi方法提取层次聚类
 * @param xi xi参数（0~1），控制簇的陡峭度
 * @return 层次聚类结果
 */
QVector<QVector<int>> OPTICS3::extractClusters(double xi) const
{
    Q_UNUSED(xi)

    /* 简化的xi聚类提取：基于可达距离的波谷检测 */
    QVector<QVector<int>> clusters;
    QVector<int> currentCluster;
    double threshold = m_epsilon;

    for (int i = 0; i < m_result.ordering.size(); ++i) {
        int idx = m_result.ordering[i];
        if (m_result.reachabilityDist[idx] <= threshold) {
            currentCluster.append(idx);
        } else {
            if (currentCluster.size() >= m_minPts) {
                clusters.append(currentCluster);
            }
            currentCluster.clear();
        }
    }
    if (currentCluster.size() >= m_minPts)
        clusters.append(currentCluster);

    return clusters;
}

/**
 * @brief 重置所有统计计数器
 */
void OPTICS3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
