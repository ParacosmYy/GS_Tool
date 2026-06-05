/**
 * @file SubspaceCluster.cpp
 * @brief 子空间聚类引擎实现 — PROCLUS算法/维度选择/质量评估
 */

#include "utils/cluster17/SubspaceCluster.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

/** @brief 构造函数 @param parent 父对象 */
SubspaceCluster::SubspaceCluster(QObject* parent)
    : QObject(parent)
{
}

/** @brief PROCLUS子空间聚类
 *  @param data 数据矩阵 [nPoints x nDims]
 *  @param k 目标聚类数
 *  @param l 平均子空间维度数
 *  @return 聚类结果列表
 */
QList<SubspaceCluster::Cluster> SubspaceCluster::proclus(
    const QVector<QVector<double>>& data, int k, int l)
{
    QElapsedTimer timer;
    timer.start();

    QList<Cluster> result;
    int n = data.size();
    if (n < k || k <= 0 || l <= 0 || data.isEmpty()) {
        return result;
    }
    int dims = data[0].size();

    /* 第1步: 随机选取初始medoid候选 */
    QVector<int> medoids = findMedoids(data, k);

    /* 第2步: 为每个medoid选择最佳子空间维度 */
    QList<QVector<int>> subspaces;
    for (int i = 0; i < medoids.size(); ++i) {
        QVector<int> selDims = selectDimensions(data, medoids[i], l);
        subspaces.append(selDims);
    }

    /* 第3步: 迭代分配点到最近medoid */
    QVector<int> assignment(n, -1);
    for (int iter = 0; iter < 20; ++iter) {
        bool changed = false;

        /* 分配阶段: 每个点分配到子空间距离最近的medoid */
        for (int p = 0; p < n; ++p) {
            double bestDist = 1e18;
            int bestMed = 0;
            for (int m = 0; m < medoids.size(); ++m) {
                double dist = 0.0;
                for (int d : subspaces[m]) {
                    if (d < dims) {
                        double diff = data[p][d] - data[medoids[m]][d];
                        dist += diff * diff;
                    }
                }
                if (dist < bestDist) {
                    bestDist = dist;
                    bestMed = m;
                }
            }
            if (assignment[p] != bestMed) {
                assignment[p] = bestMed;
                changed = true;
            }
        }

        if (!changed) break;

        /* 更新medoid: 每个簇中选择使SSE最小的点作为新medoid */
        for (int m = 0; m < medoids.size(); ++m) {
            QVector<int> members;
            for (int p = 0; p < n; ++p) {
                if (assignment[p] == m) members.append(p);
            }
            if (members.isEmpty()) continue;

            double bestSSE = 1e18;
            int bestIdx = medoids[m];
            for (int cand : members) {
                double sse = 0.0;
                for (int mem : members) {
                    for (int d : subspaces[m]) {
                        if (d < dims) {
                            double diff = data[mem][d] - data[cand][d];
                            sse += diff * diff;
                        }
                    }
                }
                if (sse < bestSSE) {
                    bestSSE = sse;
                    bestIdx = cand;
                }
            }
            medoids[m] = bestIdx;

            /* 重新选择子空间维度 */
            subspaces[m] = selectDimensions(data, medoids[m], l);
        }
    }

    /* 第4步: 构建最终聚类结果 */
    for (int m = 0; m < medoids.size(); ++m) {
        Cluster c;
        c.selectedDims = subspaces[m];
        for (int p = 0; p < n; ++p) {
            if (assignment[p] == m) {
                c.pointIndices.append(p);
            }
        }
        /* 计算中心 */
        if (!c.pointIndices.isEmpty()) {
            c.centroid.resize(dims, 0.0);
            for (int idx : c.pointIndices) {
                for (int d = 0; d < dims; ++d) {
                    c.centroid[d] += data[idx][d];
                }
            }
            double inv = 1.0 / c.pointIndices.size();
            for (int d = 0; d < dims; ++d) {
                c.centroid[d] *= inv;
            }
        }
        c.quality = clusterSSE(data, c);
        result.append(c);
    }

    /* 更新统计 */
    double elapsed = timer.elapsed();
    ++m_stats.totalClusterOps;
    m_stats.totalPointsProcessed += n;
    m_stats.totalDimensionsAnalyzed += dims;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalClusterOps);

    double overallQuality = 0.0;
    for (const auto& c : result) {
        overallQuality += c.quality;
    }
    if (m_stats.bestQuality == 0 || overallQuality < m_stats.bestQuality) {
        m_stats.bestQuality = overallQuality;
    }

    emit clusteringComplete(result.size(), overallQuality);
    return result;
}

/** @brief 为medoid选择子空间维度
 *  @param data 数据矩阵
 *  @param medoidIdx 中心点索引
 *  @param l 目标维度数
 *  @return 选中的维度索引列表
 */
QVector<int> SubspaceCluster::selectDimensions(
    const QVector<QVector<double>>& data, int medoidIdx, int l)
{
    int n = data.size();
    if (n == 0 || data[0].isEmpty()) return {};

    int dims = data[0].size();

    /* 计算每个维度的方差(到medoid的距离方差) */
    QVector<QPair<double, int>> dimScores;
    for (int d = 0; d < dims; ++d) {
        double var = computeDimVar(data, d, medoidIdx);
        dimScores.append({var, d});
    }

    /* 选择方差最小的l个维度(数据最集中的维度) */
    std::sort(dimScores.begin(), dimScores.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });

    QVector<int> selected;
    int count = qMin(l, dims);
    for (int i = 0; i < count; ++i) {
        selected.append(dimScores[i].second);
    }

    emit dimensionsSelected(selected, dimScores.isEmpty() ? 0.0
                          : dimScores[0].first);
    return selected;
}

/** @brief 计算轮廓系数
 *  @param data 数据矩阵
 *  @param clusters 聚类结果
 *  @return 平均轮廓系数
 */
double SubspaceCluster::silhouetteScore(
    const QVector<QVector<double>>& data,
    const QList<Cluster>& clusters)
{
    if (clusters.size() < 2) return 0.0;
    int n = data.size();
    int dims = data.isEmpty() ? 0 : data[0].size();

    /* 建立点→聚类映射 */
    QVector<int> label(n, -1);
    for (int c = 0; c < clusters.size(); ++c) {
        for (int idx : clusters[c].pointIndices) {
            label[idx] = c;
        }
    }

    double totalSil = 0.0;
    int validCount = 0;

    for (int p = 0; p < n; ++p) {
        if (label[p] < 0) continue;

        /* 计算a(p): 到同簇其他点的平均距离 */
        double aDist = 0.0;
        int aCount = 0;
        for (int idx : clusters[label[p]].pointIndices) {
            if (idx != p) {
                aDist += manhattanDist(data[p], data[idx]);
                ++aCount;
            }
        }
        aDist = (aCount > 0) ? aDist / aCount : 0.0;

        /* 计算b(p): 到最近其他簇的平均距离 */
        double bDist = 1e18;
        for (int c = 0; c < clusters.size(); ++c) {
            if (c == label[p]) continue;
            double cDist = 0.0;
            int cCount = 0;
            for (int idx : clusters[c].pointIndices) {
                cDist += manhattanDist(data[p], data[idx]);
                ++cCount;
            }
            if (cCount > 0) {
                cDist /= cCount;
                if (cDist < bDist) bDist = cDist;
            }
        }

        if (bDist == 1e18) bDist = 0.0;
        double sil = (bDist + aDist > 0)
            ? (bDist - aDist) / qMax(bDist, aDist) : 0.0;
        totalSil += sil;
        ++validCount;
    }

    return (validCount > 0) ? totalSil / validCount : 0.0;
}

/** @brief 轴平行子空间投影
 *  @param data 数据矩阵
 *  @param dims 投影维度
 *  @return 投影后数据
 */
QVector<QVector<double>> SubspaceCluster::axisParallelProject(
    const QVector<QVector<double>>& data, const QVector<int>& dims)
{
    QVector<QVector<double>> projected;
    projected.reserve(data.size());

    for (const auto& point : data) {
        QVector<double> proj;
        proj.reserve(dims.size());
        for (int d : dims) {
            if (d < point.size()) {
                proj.append(point[d]);
            }
        }
        projected.append(proj);
    }
    return projected;
}

/** @brief 计算聚类SSE
 *  @param data 数据矩阵
 *  @param cluster 聚类
 *  @return SSE值
 */
double SubspaceCluster::clusterSSE(const QVector<QVector<double>>& data,
                                   const Cluster& cluster) const
{
    if (cluster.pointIndices.size() <= 1) return 0.0;
    double sse = 0.0;
    for (int idx : cluster.pointIndices) {
        for (double v : cluster.centroid) {
            /* 简化: 用点到中心的距离平方 */
            int d = 0;
            for (double cv : cluster.centroid) {
                if (d < data[idx].size()) {
                    double diff = data[idx][d] - cv;
                    sse += diff * diff;
                }
                ++d;
            }
            break;
        }
    }
    return sse;
}

/** @brief 重置统计 */
void SubspaceCluster::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 曼哈顿距离 @param a 向量A @param b 向量B @return 距离 */
double SubspaceCluster::manhattanDist(const QVector<double>& a,
                                      const QVector<double>& b) const
{
    double dist = 0.0;
    int len = qMin(a.size(), b.size());
    for (int i = 0; i < len; ++i) {
        dist += qAbs(a[i] - b[i]);
    }
    return dist;
}

/** @brief 随机选取medoid @param data 数据 @param k 个数 @return 索引列表 */
QVector<int> SubspaceCluster::findMedoids(
    const QVector<QVector<double>>& data, int k)
{
    int n = data.size();
    QVector<int> indices(n);
    for (int i = 0; i < n; ++i) indices[i] = i;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(indices.begin(), indices.end(), gen);

    QVector<int> medoids;
    for (int i = 0; i < qMin(k, n); ++i) {
        medoids.append(indices[i]);
    }
    return medoids;
}

/** @brief 计算维度到medoid的距离方差 @param data 数据 @param dim 维度 @param medoidIdx medoid */
double SubspaceCluster::computeDimVar(
    const QVector<QVector<double>>& data, int dim, int medoidIdx)
{
    int n = data.size();
    if (dim >= data[0].size()) return 1e18;

    double ref = data[medoidIdx][dim];
    double sumSq = 0.0, sum = 0.0;
    for (int i = 0; i < n; ++i) {
        double diff = data[i][dim] - ref;
        sum += diff;
        sumSq += diff * diff;
    }
    double mean = sum / n;
    return sumSq / n - mean * mean;
}
