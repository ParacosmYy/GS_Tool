/**
 * @file DBSCAN9.cpp
 * @brief DBSCAN密度聚类算法实现（第9版）
 *
 * 基于密度的空间聚类应用（DBSCAN）算法，通过ε邻域和最小点数约束
 * 自动发现任意形状的簇，同时识别噪声点。使用欧氏距离度量。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/cluster62/DBSCAN9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化DBSCAN聚类器
 * @param parent 父QObject对象指针
 */
DBSCAN9::DBSCAN9(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置ε邻域半径
 * @param eps 邻域半径值，必须为正数
 */
void DBSCAN9::setEpsilon(double eps)
{
    m_eps = qMax(0.001, eps);
}

/**
 * @brief 设置最小邻域点数阈值
 * @param minPts 核心点所需的最小邻居数，至少为1
 */
void DBSCAN9::setMinPoints(int minPts)
{
    m_minPts = qMax(1, minPts);
}

/**
 * @brief 对给定数据点集合执行DBSCAN聚类
 *
 * 算法流程：
 * 1. 遍历所有未访问的点
 * 2. 查找其ε邻域内的所有邻居
 * 3. 若邻居数≥minPts，标记为核心点并扩展簇
 * 4. 否则暂时标记为噪声
 *
 * @param points 二维数据点集合，每个点为一个特征向量
 * @return 每个点的簇标签，-1表示噪声点
 */
QVector<int> DBSCAN9::cluster(const QVector<QVector<double>>& points)
{
    QElapsedTimer timer;
    timer.start();

    const int n = points.size();
    QVector<int> labels(n, -1);
    QVector<bool> visited(n, false);
    m_corePoints.clear();
    m_numClusters = 0;
    m_noiseCount = 0;

    if (n == 0) {
        emit clusteringCompleted(0, 0);
        return labels;
    }

    int clusterId = 0;

    /* 第一遍：寻找所有核心点及其邻域 */
    QVector<QVector<int>> neighbors(n);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i != j && dist(points[i], points[j]) <= m_eps) {
                neighbors[i].append(j);
            }
        }
    }

    /* 第二遍：基于核心点扩展簇 */
    for (int i = 0; i < n; ++i) {
        if (visited[i]) continue;
        visited[i] = true;

        /* 判断是否为核心点 */
        if (neighbors[i].size() < m_minPts) {
            labels[i] = -1; /* 暂时标记为噪声 */
            continue;
        }

        /* 核心点，开始扩展新簇 */
        m_corePoints.append(i);
        labels[i] = clusterId;
        QVector<int> seeds = neighbors[i];

        /* BFS扩展簇 */
        int idx = 0;
        while (idx < seeds.size()) {
            int q = seeds[idx++];
            if (!visited[q]) {
                visited[q] = true;
                if (neighbors[q].size() >= m_minPts) {
                    m_corePoints.append(q);
                    /* 将q的邻居加入种子集 */
                    for (int nb : neighbors[q]) {
                        if (!visited[nb] && !seeds.contains(nb)) {
                            seeds.append(nb);
                        }
                    }
                }
            }
            if (labels[q] == -1) {
                labels[q] = clusterId;
            }
        }
        clusterId++;
    }

    m_numClusters = clusterId;
    for (int i = 0; i < n; ++i) {
        if (labels[i] == -1) m_noiseCount++;
    }

    /* 更新统计信息 */
    m_stats.totalClusterings++;
    m_stats.totalPoints += n;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;

    emit clusteringCompleted(m_numClusters, m_noiseCount);
    return labels;
}

/**
 * @brief 计算两点之间的欧氏距离
 * @param a 第一个点
 * @param b 第二个点
 * @return 欧氏距离值
 */
double DBSCAN9::dist(const QVector<double>& a, const QVector<double>& b) const
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
 * @brief 获取当前统计信息
 * @return 包含聚类次数、总点数和平均处理时间的统计结构
 */
DBSCAN9::Stats DBSCAN9::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据
 */
void DBSCAN9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 获取噪声点占总点数的比例
 *
 * 计算最近一次聚类中噪声点所占百分比。
 * 高噪声比可能表明ε值设置过小或minPts值过大。
 *
 * @return 噪声比例（0.0~1.0），无数据时返回0
 */
double DBSCAN9::noiseRatio() const
{
    if (m_stats.totalPoints == 0) return 0.0;
    return static_cast<double>(m_noiseCount) /
        static_cast<double>(m_stats.totalPoints);
}

/**
 * @brief 获取核心点占总点数的比例
 *
 * 核心点比例反映了数据集的密度分布。
 * 比例过低说明大部分区域为稀疏区域。
 *
 * @return 核心点比例（0.0~1.0）
 */
double DBSCAN9::corePointRatio() const
{
    if (m_stats.totalPoints == 0) return 0.0;
    return static_cast<double>(m_corePoints.size()) /
        static_cast<double>(m_stats.totalPoints);
}

/**
 * @brief 验证聚类结果的内部一致性
 *
 * 检查所有非噪声点是否都被分配到有效的簇，
 * 以及簇编号是否从0连续递增。
 *
 * @return 结果是否一致
 */
bool DBSCAN9::validateResult(const QVector<int>& labels) const
{
    if (labels.isEmpty()) return true;

    /* 检查簇编号范围 */
    int maxLabel = 0;
    for (int l : labels) {
        if (l >= maxLabel) maxLabel = l;
    }

    /* 非噪声标签应该从0连续 */
    QSet<int> usedLabels;
    for (int l : labels) {
        if (l >= 0) usedLabels.insert(l);
    }

    for (int i = 0; i < maxLabel; ++i) {
        if (!usedLabels.contains(i)) return false;
    }

    return true;
}
