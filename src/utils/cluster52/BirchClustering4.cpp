/**
 * @file BirchClustering4.cpp
 * @brief BIRCH聚类算法实现，支持大规模数据的增量式聚类
 *
 * BIRCH(Balanced Iterative Reducing and Clustering using Hierarchies)
 * 通过聚类特征(CF)向量增量式处理数据点，适合大规模数据集。
 * 两个阶段：
 * 1. 构建CF树：将数据点逐步插入叶节点
 * 2. 全局聚类：对叶节点执行K-Means聚类
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/cluster52/BirchClustering4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认阈值和分支因子
 * @param parent 父QObject对象指针
 */
BirchClustering4::BirchClustering4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置聚类半径阈值
 * @param t 阈值，控制叶节点中样本的半径上限，值越小聚类越细
 */
void BirchClustering4::setThreshold(double t)
{
    m_threshold = qMax(0.001, t);
}

/**
 * @brief 设置分支因子（每个节点的最大子节点数）
 * @param b 分支因子，默认50
 */
void BirchClustering4::setBranching(int b)
{
    m_branch = qMax(2, b);
}

/**
 * @brief 插入一个数据点到CF树中
 *
 * 算法流程：
 * 1. 计算新点到各叶节点CF中心的距离
 * 2. 找到最近的叶节点CF
 * 3. 如果距离小于阈值，吸收到该CF中
 * 4. 否则创建新的叶节点CF
 *
 * @param point 待插入的数据点
 */
void BirchClustering4::insert(const QVector<double>& point)
{
    QElapsedTimer timer;
    timer.start();

    int dim = point.size();
    m_points.append(point);

    if (m_leaves.isEmpty()) {
        /* 第一个点：创建新叶节点 */
        CF cf;
        cf.n = 1;
        cf.ls = point;
        cf.ss.resize(dim);
        for (int d = 0; d < dim; ++d)
            cf.ss[d] = point[d] * point[d];
        m_leaves.append(cf);
    } else {
        /* 寻找最近的叶节点 */
        int bestIdx = 0;
        double bestDist = 1e18;

        for (int i = 0; i < m_leaves.size(); ++i) {
            double dist = 0.0;
            for (int d = 0; d < dim; ++d) {
                double center = m_leaves[i].ls[d] / m_leaves[i].n;
                double diff = point[d] - center;
                dist += diff * diff;
            }
            dist = qSqrt(dist);
            if (dist < bestDist) {
                bestDist = dist;
                bestIdx = i;
            }
        }

        if (bestDist <= m_threshold && m_leaves[bestIdx].n < m_branch) {
            /* 吸收到最近的叶节点 */
            CF& cf = m_leaves[bestIdx];
            cf.n += 1;
            for (int d = 0; d < dim; ++d) {
                cf.ls[d] += point[d];
                cf.ss[d] += point[d] * point[d];
            }
        } else {
            /* 创建新的叶节点 */
            CF cf;
            cf.n = 1;
            cf.ls = point;
            cf.ss.resize(dim);
            for (int d = 0; d < dim; ++d)
                cf.ss[d] = point[d] * point[d];
            m_leaves.append(cf);
        }
    }

    /* 更新统计 */
    m_stats.totalInsertions++;
    m_stats.totalPoints++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalInsertions;
}

/**
 * @brief 对CF树叶节点执行K-Means聚类
 *
 * 使用叶节点的CF中心作为K-Means的初始数据点，
 * 大幅减少计算量。标准K-Means迭代直至收敛。
 *
 * @param k 目标聚类数量
 * @return 每个聚类包含的原始数据点索引列表
 */
QVector<QVector<int>> BirchClustering4::cluster(int k)
{
    QElapsedTimer timer;
    timer.start();

    k = qMin(k, m_leaves.size());
    k = qMax(1, k);

    int dim = (m_points.isEmpty()) ? 0 : m_points[0].size();
    if (dim == 0 || m_leaves.isEmpty()) return {};

    /* 计算每个叶节点的中心 */
    QVector<QVector<double>> centers(k);
    for (int i = 0; i < k; ++i) {
        centers[i] = m_leaves[i].ls;
        for (int d = 0; d < dim; ++d)
            centers[i][d] /= m_leaves[i].n;
    }

    /* K-Means迭代 */
    QVector<int> leafAssign(m_leaves.size(), 0);

    for (int iter = 0; iter < 50; ++iter) {
        bool changed = false;

        /* 分配步骤：每个叶节点分配到最近的聚类中心 */
        for (int i = 0; i < m_leaves.size(); ++i) {
            double bestDist = 1e18;
            int bestC = 0;
            for (int c = 0; c < k; ++c) {
                double dist = 0.0;
                for (int d = 0; d < dim; ++d) {
                    double center = m_leaves[i].ls[d] / m_leaves[i].n;
                    double diff = center - centers[c][d];
                    dist += diff * diff;
                }
                if (dist < bestDist) {
                    bestDist = dist;
                    bestC = c;
                }
            }
            if (leafAssign[i] != bestC) {
                leafAssign[i] = bestC;
                changed = true;
            }
        }

        if (!changed) break;

        /* 更新步骤：重新计算聚类中心 */
        for (int c = 0; c < k; ++c) {
            QVector<double> sum(dim, 0.0);
            int count = 0;
            for (int i = 0; i < m_leaves.size(); ++i) {
                if (leafAssign[i] == c) {
                    for (int d = 0; d < dim; ++d)
                        sum[d] += m_leaves[i].ls[d];
                    count += m_leaves[i].n;
                }
            }
            if (count > 0) {
                for (int d = 0; d < dim; ++d)
                    centers[c][d] = sum[d] / count;
            }
        }
    }

    /* 将原始数据点映射到聚类 */
    /* 首先确定每个点属于哪个叶节点 */
    QVector<int> pointToLeaf(m_points.size(), 0);
    for (int i = 0; i < m_points.size(); ++i) {
        double bestDist = 1e18;
        for (int j = 0; j < m_leaves.size(); ++j) {
            double dist = 0.0;
            for (int d = 0; d < dim; ++d) {
                double center = m_leaves[j].ls[d] / m_leaves[j].n;
                double diff = m_points[i][d] - center;
                dist += diff * diff;
            }
            if (dist < bestDist) {
                bestDist = dist;
                pointToLeaf[i] = j;
            }
        }
    }

    /* 构建最终聚类结果 */
    QVector<QVector<int>> clusters(k);
    for (int i = 0; i < m_points.size(); ++i) {
        int cluster = leafAssign[pointToLeaf[i]];
        clusters[cluster].append(i);
    }

    /* 更新统计 */
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalInsertions);

    emit clusteringCompleted(k);
    return clusters;
}

/**
 * @brief 获取当前叶节点数量
 * @return CF树叶节点数
 */
int BirchClustering4::leafCount() const
{
    return m_leaves.size();
}

/**
 * @brief 重置所有统计数据和聚类状态
 */
void BirchClustering4::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
    m_leaves.clear();
    m_points.clear();
}
