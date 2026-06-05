#include "DBSCAN12.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化DBSCAN聚类器
 * @param parent 父QObject对象指针
 */
DBSCAN12::DBSCAN12(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 执行DBSCAN密度聚类
 *
 * 遍历所有数据点，对未访问的点执行区域查询。
 * 若邻域内点数达到minPts则创建新簇并递归扩展；
 * 否则标记为噪声点（可能后续被其他簇吸收）。
 *
 * @param points 输入数据点集合，每个点为多维坐标向量
 * @param eps 邻域半径阈值，决定两点是否相邻
 * @param minPts 核心点所需的最小邻域点数
 * @return 每个点的簇编号，-1表示噪声点
 */
QVector<int> DBSCAN12::fit(const QVector<QVector<double>>& points, double eps, int minPts)
{
    QElapsedTimer timer;
    timer.start();

    const int n = points.size();
    QVector<int> labels(n, -2);  ///< -2=未访问, -1=噪声, >=0=簇号
    m_coreIndices.clear();
    int clusterId = 0;

    /// 计算欧氏距离的辅助lambda
    auto distance = [&](int i, int j) -> double {
        double sum = 0.0;
        const auto& a = points[i];
        const auto& b = points[j];
        const int dim = qMin(a.size(), b.size());
        for (int d = 0; d < dim; ++d) {
            double diff = a[d] - b[d];
            sum += diff * diff;
        }
        return std::sqrt(sum);
    };

    /// 查询eps邻域内所有点的索引
    auto regionQuery = [&](int idx) -> QVector<int> {
        QVector<int> neighbors;
        for (int i = 0; i < n; ++i) {
            if (distance(idx, i) <= eps) {
                neighbors.append(i);
            }
        }
        return neighbors;
    };

    /// 对每个未访问的点进行聚类处理
    for (int i = 0; i < n; ++i) {
        if (labels[i] != -2) continue;  ///< 已处理则跳过

        QVector<int> neighbors = regionQuery(i);
        if (neighbors.size() < minPts) {
            labels[i] = -1;  ///< 标记为噪声
            continue;
        }

        /// 创建新簇并扩展
        m_coreIndices.append(i);
        labels[i] = clusterId;
        QVector<int> seedSet = neighbors;
        int seedIdx = 0;

        while (seedIdx < seedSet.size()) {
            int current = seedSet[seedIdx];
            if (labels[current] == -1) {
                labels[current] = clusterId;  ///< 噪声点被吸收
            }
            if (labels[current] != -2) {
                ++seedIdx;
                continue;  ///< 已分配则跳过
            }

            labels[current] = clusterId;
            QVector<int> currentNeighbors = regionQuery(current);
            if (currentNeighbors.size() >= minPts) {
                m_coreIndices.append(current);
                for (int nb : currentNeighbors) {
                    if (!seedSet.contains(nb)) {
                        seedSet.append(nb);
                    }
                }
            }
            ++seedIdx;
        }
        ++clusterId;
    }

    /// 更新统计信息
    m_stats.totalPointsProcessed += n;
    m_stats.totalClustersFound += clusterId;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalClustersFound > 0
                                                ? m_stats.totalClustersFound : 1);

    int noiseCount = std::count(labels.begin(), labels.end(), -1);
    emit clusterCompleted(clusterId, noiseCount);
    return labels;
}

/**
 * @brief 获取聚类结果中的核心点索引列表
 *
 * 核心点是指在eps邻域内拥有至少minPts个邻居的数据点，
 * 它们是簇的核心骨架，决定了簇的形状和密度。
 *
 * @return 核心点索引的向量
 */
QVector<int> DBSCAN12::coreIndices() const
{
    return m_coreIndices;
}

/**
 * @brief 获取当前统计数据
 * @return 包含处理点数、簇数和平均耗时的Stats结构
 */
DBSCAN12::Stats DBSCAN12::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据为零值
 */
void DBSCAN12::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_coreIndices.clear();
}
