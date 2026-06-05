/**
 * @file SubspaceCluster3.cpp
 * @brief 子空间聚类3 — PROCLUS+维选择 实现
 *
 * 实现 PROCLUS 风格的子空间聚类算法，通过中位点选择与
 * 稀疏度度量自动发现高维数据中的相关子空间。
 * 支持参数配置、统计追踪和计时记录。
 */

#include "utils/cluster46/SubspaceCluster3.h"

#include <QElapsedTimer>
#include <QSet>
#include <QtMath>

#include <algorithm>
#include <random>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject
 */
SubspaceCluster3::SubspaceCluster3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置目标聚类数
 * @param k 聚类数，必须 >= 2
 */
void SubspaceCluster3::setNumClusters(int k)
{
    m_k = qMax(2, k);
}

/**
 * @brief 设置每个子空间平均维度数
 * @param l 平均维度数，必须 >= 1
 */
void SubspaceCluster3::setAverageDimensions(int l)
{
    m_l = qMax(1, l);
}

/**
 * @brief 设置子空间的最小维度数
 * @param minDim 最小维度数
 */
void SubspaceCluster3::setMinDimensions(int minDim)
{
    m_minDim = qMax(1, minDim);
}

/**
 * @brief 对输入数据执行子空间聚类
 *
 * 流程：中位点选择 -> 相关维度发现 -> 子空间划分 -> 点分配。
 * 使用 QElapsedTimer 计时，并更新统计信息。
 *
 * @param data 输入数据矩阵，每行为一个样本，每列为一个维度
 * @return 聚类标签向量，标签范围 [0, k-1]
 */
QVector<int> SubspaceCluster3::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    const int n = data.size();
    if (n == 0) {
        m_subspaces.clear();
        return {};
    }
    const int d = data[0].size();

    /* 统计更新 */
    m_stats.totalClusterings++;
    m_stats.totalPointsProcessed += n;

    /* 第一步：选择中位点候选集 */
    QVector<int> medoids = selectMedoids(data);
    const int actualK = qMin(m_k, medoids.size());

    /* 第二步：为每个中位点发现相关维度 */
    m_subspaces.clear();
    m_stats.totalSubspaces = 0;
    for (int i = 0; i < actualK; ++i) {
        QVector<int> dims = findRelevantDims(data, medoids[i], m_l);
        if (dims.size() < m_minDim) {
            /* 维度不足时补充前 m_minDim 个维度 */
            for (int dd = 0; dd < d && dims.size() < m_minDim; ++dd) {
                if (!dims.contains(dd)) {
                    dims.append(dd);
                }
            }
        }
        m_subspaces.append(dims);
        m_stats.totalSubspaces += dims.size();
    }

    /* 第三步：分配每个点到最近的子空间中位点 */
    QVector<int> labels(n, 0);
    for (int i = 0; i < n; ++i) {
        double bestDist = std::numeric_limits<double>::max();
        int bestLabel = 0;
        for (int c = 0; c < actualK; ++c) {
            const QVector<int>& dims = m_subspaces[c];
            double dist = 0.0;
            for (int dim : dims) {
                double diff = data[i][dim] - data[medoids[c]][dim];
                dist += diff * diff;
            }
            if (dist < bestDist) {
                bestDist = dist;
                bestLabel = c;
            }
        }
        labels[i] = bestLabel;
    }

    /* 计时与统计 */
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;

    emit clusteringCompleted(actualK, m_stats.totalSubspaces);
    return labels;
}

/**
 * @brief 选择 k 个中位点（基于距离最大化的贪心策略）
 *
 * 随机选择第一个中位点，后续中位点选择距离已选中位数集最远的点。
 *
 * @param data 输入数据矩阵
 * @return 被选中的中位点索引列表
 */
QVector<int> SubspaceCluster3::selectMedoids(const QVector<QVector<double>>& data) const
{
    const int n = data.size();
    const int d = data[0].size();
    const int actualK = qMin(m_k, n);

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, n - 1);

    QVector<int> medoids;
    QSet<int> used;
    int first = dist(rng);
    medoids.append(first);
    used.insert(first);

    /* 贪心选择剩余中位点 */
    for (int iter = 1; iter < actualK; ++iter) {
        double bestMinDist = -1.0;
        int bestIdx = 0;
        for (int i = 0; i < n; ++i) {
            if (used.contains(i)) continue;
            double minDist = std::numeric_limits<double>::max();
            for (int m : medoids) {
                double d2 = 0.0;
                for (int j = 0; j < d; ++j) {
                    double diff = data[i][j] - data[m][j];
                    d2 += diff * diff;
                }
                minDist = qMin(minDist, d2);
            }
            if (minDist > bestMinDist) {
                bestMinDist = minDist;
                bestIdx = i;
            }
        }
        medoids.append(bestIdx);
        used.insert(bestIdx);
    }

    return medoids;
}

/**
 * @brief 为指定中位点发现最相关的维度
 *
 * 计算该中位点周围邻近点在各维度上的稀疏度，
 * 选择稀疏度最低（数据最密集）的维度作为相关子空间。
 *
 * @param data 输入数据矩阵
 * @param medoid 中位点索引
 * @param numDim 需要选择的维度数量
 * @return 相关维度的索引列表
 */
QVector<int> SubspaceCluster3::findRelevantDims(const QVector<QVector<double>>& data,
                                                  int medoid, int numDim) const
{
    const int n = data.size();
    const int d = data[0].size();
    const int actualDim = qMin(numDim, d);

    /* 计算每个维度的稀疏度（中位点邻域的方差倒置） */
    QVector<QPair<double, int>> dimSparsity;
    for (int dim = 0; dim < d; ++dim) {
        double sparsity = computeSparsity(data, medoid, dim);
        dimSparsity.append(qMakePair(sparsity, dim));
    }

    /* 按稀疏度升序排列，选择最紧凑的维度 */
    std::sort(dimSparsity.begin(), dimSparsity.end(),
              [](const QPair<double, int>& a, const QPair<double, int>& b) {
                  return a.first < b.first;
              });

    QVector<int> result;
    for (int i = 0; i < actualDim && i < dimSparsity.size(); ++i) {
        result.append(dimSparsity[i].second);
    }
    return result;
}

/**
 * @brief 计算指定点和维度上的稀疏度度量
 *
 * 使用中位点邻域（最近的 sqrt(n) 个点）在指定维度上的
 * 平均曼哈顿距离作为稀疏度指标。
 *
 * @param data 输入数据矩阵
 * @param point 中心点索引
 * @param dim 目标维度
 * @return 稀疏度值，越低表示数据越密集
 */
double SubspaceCluster3::computeSparsity(const QVector<QVector<double>>& data,
                                           int point, int dim) const
{
    const int n = data.size();
    if (n <= 1) return 0.0;

    /* 邻域大小为 sqrt(n) */
    int neighborCount = qMax(2, static_cast<int>(qSqrt(static_cast<double>(n))));

    /* 收集所有点到目标点在指定维度上的距离 */
    QVector<double> distances;
    distances.reserve(n);
    for (int i = 0; i < n; ++i) {
        double diff = qAbs(data[i][dim] - data[point][dim]);
        distances.append(diff);
    }

    /* 排序取最近的 neighborCount 个的平均值 */
    std::sort(distances.begin(), distances.end());
    double sum = 0.0;
    int cnt = qMin(neighborCount, distances.size());
    for (int i = 0; i < cnt; ++i) {
        sum += distances[i];
    }
    return (cnt > 0) ? sum / cnt : 0.0;
}

/**
 * @brief 重置所有统计数据
 */
void SubspaceCluster3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
