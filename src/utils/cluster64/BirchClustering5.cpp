/**
 * @file BirchClustering5.cpp
 * @brief BIRCH聚类算法实现（第5版）
 *
 * 实现平衡迭代规约和聚类层次树（BIRCH）算法。
 * 使用聚类特征（CF）三元组线性摘要数据点，
 * 两阶段聚类：先构建CF树，再对叶节点条目做全局聚类。
 * 适用于大规模数据集的增量式聚类。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/cluster64/BirchClustering5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <random>

/**
 * @brief 构造函数，初始化BIRCH聚类器
 * @param parent 父QObject对象指针
 */
BirchClustering5::BirchClustering5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置CF树阈值
 * @param t 半径阈值，控制叶节点条目的最大半径
 */
void BirchClustering5::setThreshold(double t)
{
    m_threshold = qMax(0.001, t);
}

/**
 * @brief 设置分支因子
 * @param b 每个非叶节点的最大子节点数
 */
void BirchClustering5::setBranchFactor(int b)
{
    m_branch = qMax(2, b);
}

/**
 * @brief 设置最终聚类的簇数
 * @param k 全局聚类阶段的目标簇数
 */
void BirchClustering5::setNumClusters(int k)
{
    m_k = qMax(1, k);
}

/**
 * @brief 计算两个CF条目之间的距离（质心欧氏距离）
 * @param a 第一个CF条目
 * @param b 第二个CF条目
 * @return 质心之间的欧氏距离
 */
double BirchClustering5::entryDist(const CFEntry& a, const CFEntry& b) const
{
    if (a.n == 0 || b.n == 0) return 0.0;
    int dim = a.ls.size();
    double sum = 0.0;
    for (int i = 0; i < dim; ++i) {
        double diff = (a.ls[i] / a.n) - (b.ls[i] / b.n);
        sum += diff * diff;
    }
    return qSqrt(sum);
}

/**
 * @brief 对数据点集合执行BIRCH聚类
 *
 * 两阶段流程：
 * 1. 增量构建CF树：逐点吸收到最近的叶条目，超阈值则分裂
 * 2. 全局聚类：对叶条目的质心执行K-Means得到最终簇划分
 *
 * @param points 输入数据点集合
 * @return 每个点的簇标签
 */
QVector<int> BirchClustering5::cluster(const QVector<QVector<double>>& points)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> labels;
    if (points.isEmpty()) {
        emit clusteringCompleted(0, 0);
        return labels;
    }

    int n = points.size();
    int dim = points[0].size();
    labels.resize(n, 0);

    /* ========== 阶段1：增量构建CF树 ========== */
    m_leafEntries.clear();
    m_leafCount = 0;
    m_height = 0;

    for (int i = 0; i < n; ++i) {
        const auto& pt = points[i];

        /* 计算点到所有叶条目质心的距离 */
        int bestEntry = -1;
        double bestDist = std::numeric_limits<double>::max();

        for (int e = 0; e < m_leafEntries.size(); ++e) {
            if (m_leafEntries[e].n == 0) continue;
            int d = m_leafEntries[e].ls.size();
            double dist = 0.0;
            for (int j = 0; j < qMin(d, dim); ++j) {
                double diff = m_leafEntries[e].ls[j] / m_leafEntries[e].n - pt[j];
                dist += diff * diff;
            }
            dist = qSqrt(dist);
            if (dist < bestDist) {
                bestDist = dist;
                bestEntry = e;
            }
        }

        if (bestEntry >= 0 && bestDist <= m_threshold) {
            /* 吸收到已有条目 */
            CFEntry& entry = m_leafEntries[bestEntry];
            entry.n++;
            for (int j = 0; j < dim; ++j) {
                entry.ls[j] += pt[j];
                entry.ss += pt[j] * pt[j];
            }
        } else {
            /* 创建新条目 */
            CFEntry newEntry;
            newEntry.n = 1;
            newEntry.ls = pt;
            newEntry.ss = 0.0;
            for (int j = 0; j < dim; ++j) {
                newEntry.ss += pt[j] * pt[j];
            }
            m_leafEntries.append(newEntry);
            m_leafCount++;
        }
    }

    /* 计算CF树高度（近似） */
    int leafCount = m_leafEntries.size();
    m_height = 0;
    int temp = leafCount;
    while (temp > 0) {
        temp /= m_branch;
        m_height++;
    }
    m_height = qMax(1, m_height);

    /* ========== 阶段2：对叶条目质心做全局聚类 ========== */
    int numEntries = m_leafEntries.size();
    int actualK = qMin(m_k, numEntries);

    /* 提取质心 */
    QVector<QVector<double>> centroids(numEntries);
    for (int e = 0; e < numEntries; ++e) {
        centroids[e].resize(dim);
        for (int j = 0; j < dim; ++j) {
            centroids[e][j] = m_leafEntries[e].ls[j] / m_leafEntries[e].n;
        }
    }

    /* 对质心做K-Means */
    QVector<int> entryLabels(numEntries, 0);
    if (numEntries <= actualK) {
        /* 条目数不超过k，每个条目一个簇 */
        for (int i = 0; i < numEntries; ++i) entryLabels[i] = i;
    } else {
        /* 初始化质心 */
        QVector<QVector<double>> kmCent(actualK);
        std::mt19937 gen(42);
        std::uniform_int_distribution<int> dist(0, numEntries - 1);
        for (int c = 0; c < actualK; ++c) {
            kmCent[c] = centroids[dist(gen)];
        }

        /* K-Means迭代 */
        for (int iter = 0; iter < 50; ++iter) {
            bool changed = false;
            for (int e = 0; e < numEntries; ++e) {
                double minD = std::numeric_limits<double>::max();
                int bestC = 0;
                for (int c = 0; c < actualK; ++c) {
                    double d = 0.0;
                    for (int j = 0; j < dim; ++j) {
                        double diff = centroids[e][j] - kmCent[c][j];
                        d += diff * diff;
                    }
                    if (d < minD) { minD = d; bestC = c; }
                }
                if (entryLabels[e] != bestC) { entryLabels[e] = bestC; changed = true; }
            }
            if (!changed) break;

            /* 更新质心 */
            for (int c = 0; c < actualK; ++c) {
                QVector<double> sum(dim, 0.0);
                int cnt = 0;
                for (int e = 0; e < numEntries; ++e) {
                    if (entryLabels[e] == c) {
                        for (int j = 0; j < dim; ++j) sum[j] += centroids[e][j];
                        cnt++;
                    }
                }
                if (cnt > 0) {
                    for (int j = 0; j < dim; ++j) kmCent[c][j] = sum[j] / cnt;
                }
            }
        }
    }

    /* 将条目标签映射回原始数据点 */
    for (int i = 0; i < n; ++i) {
        const auto& pt = points[i];
        int bestEntry = 0;
        double bestDist = std::numeric_limits<double>::max();
        for (int e = 0; e < numEntries; ++e) {
            double d = 0.0;
            for (int j = 0; j < dim; ++j) {
                double diff = m_leafEntries[e].ls[j] / m_leafEntries[e].n - pt[j];
                d += diff * diff;
            }
            if (d < bestDist) { bestDist = d; bestEntry = e; }
        }
        labels[i] = entryLabels[bestEntry];
    }

    /* 更新统计 */
    m_stats.totalClusterings++;
    m_stats.totalPoints += n;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;

    emit clusteringCompleted(actualK, m_leafCount);
    return labels;
}

/**
 * @brief 获取当前统计信息
 * @return 聚类统计结构
 */
BirchClustering5::Stats BirchClustering5::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据
 */
void BirchClustering5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
