/**
 * @file BirchClustering2.cpp
 * @brief BIRCH聚类增强实现 — CF树阈值插入/子簇合并/全局聚类
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/cluster31/BirchClustering2.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <limits>
#include <cmath>
#include <random>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象
 */
BirchClustering2::BirchClustering2(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("BirchClustering2"));
}

/**
 * @brief 设置聚类距离阈值
 *
 * 阈值决定了新点被吸收到现有子簇的最大允许半径。
 * 较小的阈值会产生更多更精细的子簇。
 *
 * @param threshold 距离阈值（必须为正数）
 */
void BirchClustering2::setThreshold(double threshold)
{
    m_threshold = qMax(0.01, threshold);
}

/**
 * @brief 设置CF树分支因子
 *
 * 分支因子控制每个非叶节点的最大子节点数量。
 *
 * @param b 分支因子（最小为2）
 */
void BirchClustering2::setBranchingFactor(int b)
{
    m_branching = qMax(2, b);
}

/**
 * @brief 设置全局聚类的最大簇数
 * @param k 最大簇数（最小为1）
 */
void BirchClustering2::setMaxClusters(int k)
{
    m_maxClusters = qMax(1, k);
}

/**
 * @brief 执行BIRCH聚类
 *
 * 流程: 逐点插入CF树 → 提取子簇中心 → 对子簇执行全局K-Means
 * 返回每个数据点所属的最终聚类标签（0 ~ k-1），-1表示离群。
 *
 * @param data 输入数据矩阵 [nPoints x nDims]
 * @return 聚类标签向量
 */
QVector<int> BirchClustering2::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    const int n = data.size();
    if (n == 0) {
        emit clusteringComplete(0);
        return {};
    }

    const int dims = data[0].size();
    m_centers.clear();
    m_counts.clear();

    /* 第一阶段: 增量构建子簇
     * 使用在线阈值插入 — 每个新点找到最近的现有子簇中心，
     * 若距离在阈值内则吸收，否则创建新子簇 */
    for (int i = 0; i < n; ++i) {
        const auto& point = data[i];
        if (m_centers.isEmpty()) {
            m_centers.append(point);
            m_counts.append(1);
            continue;
        }

        /* 查找最近子簇中心 */
        int bestIdx = 0;
        double bestDist = std::numeric_limits<double>::max();
        for (int c = 0; c < m_centers.size(); ++c) {
            double dist = 0.0;
            for (int d = 0; d < dims; ++d) {
                double diff = point[d] - m_centers[c][d];
                dist += diff * diff;
            }
            if (dist < bestDist) {
                bestDist = dist;
                bestIdx = c;
            }
        }

        /* 阈值判断: 距离平方 < threshold^2 则吸收 */
        if (bestDist < m_threshold * m_threshold) {
            /* 增量更新子簇中心为加权平均 */
            int cnt = m_counts[bestIdx];
            for (int d = 0; d < dims; ++d) {
                m_centers[bestIdx][d] =
                    (m_centers[bestIdx][d] * cnt + point[d]) / (cnt + 1);
            }
            m_counts[bestIdx] += 1;
        } else {
            /* 创建新子簇 */
            m_centers.append(point);
            m_counts.append(1);
        }
    }

    /* 第二阶段: 子簇合并
     * 合并距离最近的子簇对，直到子簇数不超过分支因子*2 */
    while (m_centers.size() > m_branching * 2 && m_centers.size() > m_maxClusters) {
        double minDist = std::numeric_limits<double>::max();
        int mi = 0, mj = 1;
        const int nc = m_centers.size();
        for (int i = 0; i < nc - 1; ++i) {
            for (int j = i + 1; j < nc; ++j) {
                double dist = 0.0;
                for (int d = 0; d < dims; ++d) {
                    double diff = m_centers[i][d] - m_centers[j][d];
                    dist += diff * diff;
                }
                if (dist < minDist) {
                    minDist = dist;
                    mi = i;
                    mj = j;
                }
            }
        }

        /* 合并 mi 和 mj: 加权质心 */
        int ci = m_counts[mi];
        int cj = m_counts[mj];
        int total = ci + cj;
        for (int d = 0; d < dims; ++d) {
            m_centers[mi][d] =
                (m_centers[mi][d] * ci + m_centers[mj][d] * cj) / total;
        }
        m_counts[mi] = total;
        m_centers.removeAt(mj);
        m_counts.removeAt(mj);
    }

    /* 第三阶段: 全局K-Means对子簇中心聚类 */
    const int k = qMin(m_maxClusters, m_centers.size());
    QVector<QVector<double>> globalCenters(k);
    QVector<int> subclusterLabels(m_centers.size(), 0);

    /* 初始化全局中心: 从子簇中心中均匀选取 */
    std::mt19937 rng(42);
    QVector<int> indices(m_centers.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::shuffle(indices.begin(), indices.end(), rng);
    for (int i = 0; i < k; ++i) {
        globalCenters[i] = m_centers[indices[i]];
    }

    /* 迭代K-Means */
    for (int iter = 0; iter < 50; ++iter) {
        bool changed = false;

        /* 分配每个子簇到最近全局中心 */
        for (int s = 0; s < m_centers.size(); ++s) {
            double bestDist = std::numeric_limits<double>::max();
            int bestK = 0;
            for (int c = 0; c < k; ++c) {
                double dist = 0.0;
                for (int d = 0; d < dims; ++d) {
                    double diff = m_centers[s][d] - globalCenters[c][d];
                    dist += diff * diff;
                }
                if (dist < bestDist) {
                    bestDist = dist;
                    bestK = c;
                }
            }
            if (subclusterLabels[s] != bestK) {
                subclusterLabels[s] = bestK;
                changed = true;
            }
        }

        if (!changed) break;

        /* 更新全局中心: 按子簇点数加权平均 */
        for (int c = 0; c < k; ++c) {
            QVector<double> sum(dims, 0.0);
            int totalW = 0;
            for (int s = 0; s < m_centers.size(); ++s) {
                if (subclusterLabels[s] == c) {
                    int w = m_counts[s];
                    for (int d = 0; d < dims; ++d) {
                        sum[d] += m_centers[s][d] * w;
                    }
                    totalW += w;
                }
            }
            if (totalW > 0) {
                for (int d = 0; d < dims; ++d) {
                    globalCenters[c][d] = sum[d] / totalW;
                }
            }
        }
    }

    /* 第四阶段: 将原始数据点映射到最终聚类标签 */
    QVector<int> labels(n, -1);
    for (int i = 0; i < n; ++i) {
        /* 先找到最近的子簇 */
        int bestSub = 0;
        double bestDist = std::numeric_limits<double>::max();
        for (int s = 0; s < m_centers.size(); ++s) {
            double dist = 0.0;
            for (int d = 0; d < dims; ++d) {
                double diff = data[i][d] - m_centers[s][d];
                dist += diff * diff;
            }
            if (dist < bestDist) {
                bestDist = dist;
                bestSub = s;
            }
        }
        /* 子簇标签即为最终标签 */
        labels[i] = subclusterLabels[bestSub];
    }

    /* 更新统计信息 */
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalClusterings++;
    m_stats.totalPointsProcessed += n;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;

    emit clusteringComplete(k);
    return labels;
}

/**
 * @brief 获取子簇中心向量
 * @return 子簇中心矩阵 [numSubclusters x dims]
 */
QVector<QVector<double>> BirchClustering2::subclusterCenters() const
{
    return m_centers;
}

/**
 * @brief 获取当前子簇数量
 * @return 子簇数量
 */
int BirchClustering2::numSubclusters() const
{
    return m_centers.size();
}

/**
 * @brief 重置所有累积统计信息
 */
void BirchClustering2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
