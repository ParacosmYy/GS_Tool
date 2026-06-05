/**
 * @file DBSCAN8.cpp
 * @brief DBSCAN8 实现
 *
 * 实现DBSCAN密度聚类：HNSW索引构建、epsilon邻域查询、
 * 核心点/边界点/噪声点分类、簇递归扩展。
 */

#include "utils/cluster159/DBSCAN8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>
#include <limits>

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
DBSCAN8::DBSCAN8(QObject* parent)
    : QObject(parent)
{
}

void DBSCAN8::setEpsilon(double eps)
{
    m_eps = qMax(1e-10, eps);
}

void DBSCAN8::setMinPoints(int minPts)
{
    m_minPts = qMax(2, minPts);
}

void DBSCAN8::setHnswM(int m)
{
    m_hnswM = qMax(4, m);
}

/**
 * @brief 构建HNSW近似最近邻图
 *
 * 每个节点最多连接M个邻居，通过贪心搜索插入。
 * 生成随机层数用于层次结构。
 */
void DBSCAN8::buildHnsw(const QVector<QVector<double>>& data)
{
    const int n = data.size();
    m_hnswNodes.resize(n);
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> levelDist(0.0, 1.0);
    const double mL = 1.0 / qLn(m_hnswM);

    for (int i = 0; i < n; ++i) {
        m_hnswNodes[i].index = i;

        if (i == 0) continue;

        /* 贪心搜索：从已有点中找最近邻 */
        int entryPoint = 0;
        double bestDist = euclidean(data[i], data[0]);
        int bestIdx = 0;

        /* 多轮搜索扩大邻居覆盖 */
        for (int probe = 0; probe < qMin(i, m_hnswM * 2); ++probe) {
            int candidate = (probe < i) ? probe : rng() % i;
            double d = euclidean(data[i], data[candidate]);
            if (d < bestDist) {
                bestDist = d;
                bestIdx = candidate;
            }

            /* 从候选点扩展搜索 */
            for (int nb : m_hnswNodes[candidate].neighbors) {
                double nd = euclidean(data[i], data[nb]);
                if (nd < bestDist) {
                    bestDist = nd;
                    bestIdx = nb;
                }
            }
        }

        /* 双向连接 */
        m_hnswNodes[i].neighbors.append(bestIdx);
        m_hnswNodes[bestIdx].neighbors.append(i);

        /* 限制每个节点的邻居数 */
        if (m_hnswNodes[bestIdx].neighbors.size() > m_hnswM * 2) {
            /* 保留最近的M*2个邻居 */
            QVector<QPair<double, int>> distIdx;
            for (int nb : m_hnswNodes[bestIdx].neighbors) {
                distIdx.append({euclidean(data[bestIdx], data[nb]), nb});
            }
            std::sort(distIdx.begin(), distIdx.end());
            m_hnswNodes[bestIdx].neighbors.clear();
            for (int k = 0; k < qMin(m_hnswM * 2, distIdx.size()); ++k) {
                m_hnswNodes[bestIdx].neighbors.append(distIdx[k].second);
            }
        }
    }
}

/**
 * @brief 使用HNSW索引进行epsilon范围查询
 *
 * 从查询点出发，沿HNSW图边扩展搜索，收集距离 < eps 的所有点。
 */
QSet<int> DBSCAN8::rangeQuery(int pointIdx, const QVector<QVector<double>>& data) const
{
    QSet<int> result;
    const int n = data.size();
    const double epsSq = m_eps * m_eps;

    /* 使用HNSW邻居作为种子扩展 */
    QSet<int> visited;
    QVector<int> queue;
    queue.append(pointIdx);
    visited.insert(pointIdx);

    while (!queue.isEmpty()) {
        int current = queue.takeLast();
        double dSq = 0.0;
        const int dim = qMin(data[pointIdx].size(), data[current].size());
        for (int d = 0; d < dim; ++d) {
            double diff = data[pointIdx][d] - data[current][d];
            dSq += diff * diff;
        }

        if (dSq <= epsSq) {
            result.insert(current);

            /* 从HNSW邻居中扩展搜索 */
            for (int nb : m_hnswNodes[current].neighbors) {
                if (!visited.contains(nb)) {
                    visited.insert(nb);
                    queue.append(nb);
                }
            }
        }
    }

    /* 暴力补充：对HNSW可能遗漏的点做距离检查 */
    /* 只在小规模时执行 */
    if (n <= 2000) {
        for (int i = 0; i < n; ++i) {
            if (result.contains(i)) continue;
            double dSq = 0.0;
            const int dim = qMin(data[pointIdx].size(), data[i].size());
            for (int d = 0; d < dim; ++d) {
                double diff = data[pointIdx][d] - data[i][d];
                dSq += diff * diff;
            }
            if (dSq <= epsSq) {
                result.insert(i);
            }
        }
    }

    return result;
}

double DBSCAN8::euclidean(const QVector<double>& a, const QVector<double>& b) const
{
    double sum = 0.0;
    const int dim = qMin(a.size(), b.size());
    for (int i = 0; i < dim; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return qSqrt(sum);
}

/**
 * @brief 递归扩展簇
 *
 * 从核心点出发，将密度可达的所有点归入同一簇。
 */
void DBSCAN8::expandCluster(int pointIdx, int clusterId,
                            const QVector<QVector<double>>& data,
                            QVector<int>& clusterLabels)
{
    QSet<int> seeds = rangeQuery(pointIdx, data);

    if (seeds.size() < static_cast<size_t>(m_minPts)) {
        /* 不是核心点，不扩展 */
        return;
    }

    QVector<int> queue;
    for (int s : seeds) {
        if (clusterLabels[s] == -1 || m_labels[s] == PointLabel::Undefined) {
            clusterLabels[s] = clusterId;
            m_labels[s] = PointLabel::Border;
            queue.append(s);
        }
    }

    /* BFS扩展 */
    while (!queue.isEmpty()) {
        int current = queue.takeFirst();
        QSet<int> currentNeighbors = rangeQuery(current, data);

        if (currentNeighbors.size() >= static_cast<size_t>(m_minPts)) {
            m_labels[current] = PointLabel::Core;
            for (int nb : currentNeighbors) {
                if (m_labels[nb] == PointLabel::Undefined ||
                    clusterLabels[nb] == -1) {
                    clusterLabels[nb] = clusterId;
                    m_labels[nb] = PointLabel::Border;
                    queue.append(nb);
                }
            }
        }
    }
}

/**
 * @brief 执行DBSCAN聚类
 *
 * 1) 构建HNSW近似最近邻索引
 * 2) 遍历每个未访问点，执行epsilon范围查询
 * 3) 若邻域内点数 >= MinPts，标记为核心点并扩展簇
 * 4) 否则暂时标记为噪声
 */
QVector<int> DBSCAN8::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    const int n = data.size();
    if (n == 0) return QVector<int>();

    /* 构建HNSW索引 */
    buildHnsw(data);

    QVector<int> clusterLabels(n, -1);
    m_labels.resize(n);
    for (int i = 0; i < n; ++i) {
        m_labels[i] = PointLabel::Undefined;
    }

    int clusterId = 0;

    for (int i = 0; i < n; ++i) {
        if (m_labels[i] != PointLabel::Undefined) continue;

        QSet<int> neighbors = rangeQuery(i, data);

        if (neighbors.size() < static_cast<size_t>(m_minPts)) {
            /* 暂时标记为噪声 */
            m_labels[i] = PointLabel::Noise;
            continue;
        }

        /* 核心点，创建新簇 */
        m_labels[i] = PointLabel::Core;
        clusterLabels[i] = clusterId;

        /* 扩展簇 */
        expandCluster(i, clusterId, data, clusterLabels);

        clusterId++;
    }

    /* 统计 */
    int noiseCount = 0;
    int coreCount = 0;
    for (int i = 0; i < n; ++i) {
        if (m_labels[i] == PointLabel::Noise) noiseCount++;
        if (m_labels[i] == PointLabel::Core) coreCount++;
    }

    m_stats.noisePointCount = noiseCount;
    m_stats.corePointCount = coreCount;
    m_stats.totalClusterOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalClusterOps > 0)
        ? m_timeSum / m_stats.totalClusterOps : 0.0;

    emit fitCompleted(clusterId, noiseCount);
    return clusterLabels;
}

void DBSCAN8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
