/**
 * @file DBSCAN2.cpp
 * @brief DBSCAN密度聚类引擎实现 — KD-tree加速邻域查询
 */

#include "utils/cluster7/DBSCAN2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <queue>

/** @brief 构造函数 @param parent 父对象 */
DBSCAN2::DBSCAN2(QObject* parent)
    : QObject(parent)
    , m_kdRoot(nullptr)
    , m_dataRef(nullptr)
{
}

/** @brief 设置epsilon邻域半径 @param eps 半径 */
void DBSCAN2::setEpsilon(double eps)
{
    m_eps = qMax(1e-10, eps);
}

/** @brief 设置最小点数 @param minPts 成为核心点所需最小邻居数 */
void DBSCAN2::setMinPoints(int minPts)
{
    m_minPts = qMax(2, minPts);
}

/** @brief 执行DBSCAN聚类 @param data 输入数据 @return 聚类结果 */
DBSCAN2::ClusteringResult DBSCAN2::fit(const QList<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    ClusteringResult result;
    int n = data.size();
    if (n == 0) return result;

    m_dimensions = data[0].size();
    m_dataRef = &data;

    /* 构建KD-tree */
    QList<int> indices;
    for (int i = 0; i < n; ++i) indices.append(i);
    freeKdTree(m_kdRoot);
    m_kdRoot = buildKdTree(data, indices, 0);

    /* 初始化标签 */
    QVector<PointLabel> labels(n, PointLabel::Undefined);
    QVector<int> clusterAssign(n, -1);

    int clusterId = 0;

    for (int i = 0; i < n; ++i) {
        if (labels[i] != PointLabel::Undefined) continue;

        /* 查找epsilon邻域 */
        QList<int> neighbors = rangeQuery(data, i);
        m_stats.totalNeighborQueries++;

        if (neighbors.size() < m_minPts) {
            labels[i] = PointLabel::Noise;
            continue;
        }

        /* 新簇 */
        labels[i] = PointLabel::Core;
        clusterAssign[i] = clusterId;

        /* BFS扩展 */
        expandCluster(data, i, clusterId, labels, clusterAssign, neighbors);

        clusterId++;
        emit progress(i + 1, n);
    }

    /* 收集结果 */
    QMap<int, ClusterResult> clusterMap;
    for (int i = 0; i < n; ++i) {
        int cid = clusterAssign[i];
        if (cid >= 0) {
            clusterMap[cid].indices.append(i);
            clusterMap[cid].points.append(data[i]);
        } else if (labels[i] == PointLabel::Noise) {
            result.noiseIndices.append(i);
        }
    }

    /* 计算簇中心和半径 */
    for (auto it = clusterMap.begin(); it != clusterMap.end(); ++it) {
        ClusterResult& cr = it.value();
        cr.centroid.resize(m_dimensions, 0.0);

        for (const auto& pt : cr.points) {
            for (int d = 0; d < m_dimensions; ++d) {
                cr.centroid[d] += pt[d];
            }
        }
        for (int d = 0; d < m_dimensions; ++d) {
            cr.centroid[d] /= cr.points.size();
        }

        double maxDist = 0.0;
        for (const auto& pt : cr.points) {
            double dist = euclideanDistance(pt, cr.centroid);
            if (dist > maxDist) maxDist = dist;
        }
        cr.centroidRadius = maxDist;

        int coreCount = 0;
        for (int idx : cr.indices) {
            if (labels[idx] == PointLabel::Core) coreCount++;
        }
        cr.label = (coreCount > 0) ? PointLabel::Core : PointLabel::Border;

        result.clusters.append(cr);
        result.coreCount += coreCount;
    }

    result.totalPoints = n;
    result.noiseCount = result.noiseIndices.size();
    result.borderCount = n - result.coreCount - result.noiseCount;

    /* 更新统计 */
    m_stats.totalClusterings++;
    m_stats.totalPointsProcessed += n;
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalClusterings);

    freeKdTree(m_kdRoot);
    m_kdRoot = nullptr;
    m_dataRef = nullptr;

    emit clusteringComplete(result.clusters.size(), result.noiseCount);
    return result;
}

/** @brief 查询指定点的epsilon邻域 @param data 数据集 @param pointIndex 查询点索引 @return 邻域内点索引 */
QList<int> DBSCAN2::rangeQuery(const QList<QVector<double>>& data,
                                int pointIndex) const
{
    QList<int> result;
    if (pointIndex < 0 || pointIndex >= data.size()) return result;

    const auto& query = data[pointIndex];

    /* 使用KD-tree加速 */
    if (m_kdRoot && m_dataRef) {
        kdRangeQuery(m_kdRoot, query, data, m_eps, result);
    } else {
        /* 暴力搜索 */
        for (int i = 0; i < data.size(); ++i) {
            if (euclideanDistance(query, data[i]) <= m_eps) {
                result.append(i);
            }
        }
    }
    return result;
}

/** @brief 获取簇统计信息 @param cluster 簇 @return (均值, 标准差) */
QPair<QVector<double>, QVector<double>> DBSCAN2::clusterStatistics(
    const ClusterResult& cluster) const
{
    int n = cluster.points.size();
    int d = cluster.centroid.size();
    QVector<double> mean = cluster.centroid;
    QVector<double> stddev(d, 0.0);

    if (n <= 1) return {mean, stddev};

    for (const auto& pt : cluster.points) {
        for (int i = 0; i < d; ++i) {
            double diff = pt[i] - mean[i];
            stddev[i] += diff * diff;
        }
    }
    for (int i = 0; i < d; ++i) {
        stddev[i] = qSqrt(stddev[i] / static_cast<double>(n - 1));
    }
    return {mean, stddev};
}

/** @brief 计算Rand指数 @param r1 结果1 @param r2 结果2 @return Rand指数 */
double DBSCAN2::randIndex(const ClusteringResult& r1,
                           const ClusteringResult& r2) const
{
    int n = r1.totalPoints;
    if (n != r2.totalPoints || n < 2) return 0.0;

    /* 构建标签数组 */
    QVector<int> labels1(n, -1), labels2(n, -1);
    for (int c = 0; c < r1.clusters.size(); ++c) {
        for (int idx : r1.clusters[c].indices) labels1[idx] = c;
    }
    for (int c = 0; c < r2.clusters.size(); ++c) {
        for (int idx : r2.clusters[c].indices) labels2[idx] = c;
    }

    long long agree = 0;
    long long total = static_cast<long long>(n) * (n - 1) / 2;

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            bool same1 = (labels1[i] == labels1[j]);
            bool same2 = (labels2[i] == labels2[j]);
            if (same1 == same2) agree++;
        }
    }
    return static_cast<double>(agree) / static_cast<double>(total);
}

/** @brief 重置统计 */
void DBSCAN2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 构建KD-tree */
DBSCAN2::KdNode* DBSCAN2::buildKdTree(const QList<QVector<double>>& data,
                                        QList<int>& indices, int depth)
{
    if (indices.isEmpty()) return nullptr;

    int splitDim = depth % m_dimensions;

    /* 按splitDim排序 */
    std::sort(indices.begin(), indices.end(), [&](int a, int b) {
        return data[a][splitDim] < data[b][splitDim];
    });

    int mid = indices.size() / 2;
    KdNode* node = new KdNode;
    node->index = indices[mid];
    node->splitDim = splitDim;

    QList<int> leftIndices = indices.mid(0, mid);
    QList<int> rightIndices = indices.mid(mid + 1);

    node->left = buildKdTree(data, leftIndices, depth + 1);
    node->right = buildKdTree(data, rightIndices, depth + 1);

    return node;
}

/** @brief KD-tree范围查询 */
void DBSCAN2::kdRangeQuery(KdNode* node, const QVector<double>& query,
                             const QList<QVector<double>>& data,
                             double radius, QList<int>& result) const
{
    if (!node) return;

    double dist = euclideanDistance(query, data[node->index]);
    if (dist <= radius) {
        result.append(node->index);
    }

    double diff = query[node->splitDim] - data[node->index][node->splitDim];

    if (diff <= radius) {
        kdRangeQuery(node->right, query, data, radius, result);
    }
    if (diff >= -radius) {
        kdRangeQuery(node->left, query, data, radius, result);
    }
}

/** @brief 释放KD-tree */
void DBSCAN2::freeKdTree(KdNode* node)
{
    if (!node) return;
    freeKdTree(node->left);
    freeKdTree(node->right);
    delete node;
}

/** @brief 计算欧几里得距离 */
double DBSCAN2::euclideanDistance(const QVector<double>& a,
                                   const QVector<double>& b) const
{
    double sum = 0.0;
    int dim = qMin(a.size(), b.size());
    for (int i = 0; i < dim; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return qSqrt(sum);
}

/** @brief 扩展簇: BFS扩展核心点邻域 */
void DBSCAN2::expandCluster(const QList<QVector<double>>& data, int pointIndex,
                              int clusterId, QVector<PointLabel>& labels,
                              QVector<int>& clusterAssign,
                              const QList<int>& neighbors)
{
    std::queue<int> queue;
    for (int idx : neighbors) {
        if (idx != pointIndex) queue.push(idx);
    }

    while (!queue.empty()) {
        int current = queue.front();
        queue.pop();

        if (labels[current] == PointLabel::Noise) {
            labels[current] = PointLabel::Border;
            clusterAssign[current] = clusterId;
        }

        if (labels[current] != PointLabel::Undefined) continue;

        labels[current] = PointLabel::Core;
        clusterAssign[current] = clusterId;

        QList<int> currentNeighbors = rangeQuery(data, current);
        m_stats.totalNeighborQueries++;

        if (currentNeighbors.size() >= m_minPts) {
            for (int idx : currentNeighbors) {
                if (labels[idx] == PointLabel::Undefined ||
                    labels[idx] == PointLabel::Noise) {
                    queue.push(idx);
                }
            }
        }
    }
}
