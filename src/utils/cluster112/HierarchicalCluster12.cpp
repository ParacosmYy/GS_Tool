#include "HierarchicalCluster12.h"
#include <QElapsedTimer>
#include <QMap>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化层次聚类引擎
 * @param parent 父对象指针
 */
HierarchicalCluster12::HierarchicalCluster12(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void HierarchicalCluster12::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 执行凝聚式层次聚类
 *
 * 自底向上逐步合并最近的两个簇，支持四种链接准则：
 * - single: 单链接，取簇间最小距离
 * - complete: 全链接，取簇间最大距离
 * - average: 平均链接，取簇间平均距离
 * - ward: Ward法，最小化簇内方差增量
 *
 * @param distanceMatrix n×n距离矩阵（对称）
 * @param linkage 链接准则名称
 * @return 每步合并记录 ((簇A索引, 簇B索引), 合并距离)
 */
QVector<QPair<QPair<int, int>, double>> HierarchicalCluster12::agglomerative(
    const QVector<QVector<double>>& distanceMatrix, const QString& linkage)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<QPair<int, int>, double>> mergeSteps;
    const int n = distanceMatrix.size();
    if (n == 0) {
        emit clusteringCompleted(0);
        return mergeSteps;
    }

    /* 初始化活跃簇集合 */
    QVector<QVector<int>> clusters(n);
    for (int i = 0; i < n; ++i)
        clusters[i] = {i};

    QVector<bool> active(n, true);
    QVector<int> clusterSize(n, 1);

    /* 工作距离矩阵 */
    QVector<QVector<double>> dist = distanceMatrix;

    for (int step = 0; step < n - 1; ++step) {
        /* 查找最近的两个活跃簇 */
        double minDist = 1e18;
        int mergeA = -1, mergeB = -1;

        for (int i = 0; i < n; ++i) {
            if (!active[i]) continue;
            for (int j = i + 1; j < n; ++j) {
                if (!active[j]) continue;
                if (dist[i][j] < minDist) {
                    minDist = dist[i][j];
                    mergeA = i;
                    mergeB = j;
                }
            }
        }

        if (mergeA < 0 || mergeB < 0) break;

        /* 记录合并步骤 */
        mergeSteps.append({{mergeA, mergeB}, minDist});

        /* 合并B到A */
        clusters[mergeA].append(clusters[mergeB]);
        active[mergeB] = false;

        /* 更新距离矩阵 */
        for (int k = 0; k < n; ++k) {
            if (!active[k] || k == mergeA) continue;
            double newDist = 0.0;

            if (linkage == QStringLiteral("single")) {
                /* 单链接：最小距离 */
                newDist = qMin(dist[mergeA][k], dist[mergeB][k]);
            } else if (linkage == QStringLiteral("complete")) {
                /* 全链接：最大距离 */
                newDist = qMax(dist[mergeA][k], dist[mergeB][k]);
            } else if (linkage == QStringLiteral("average")) {
                /* 平均链接：按簇大小加权平均 */
                double dA = dist[mergeA][k] * clusterSize[mergeA];
                double dB = dist[mergeB][k] * clusterSize[mergeB];
                int totalSize = clusterSize[mergeA] + clusterSize[mergeB];
                newDist = (dA + dB) / totalSize;
            } else {
                /* Ward法：最小化合并后的方差增量 */
                double dAK = dist[mergeA][k];
                double dBK = dist[mergeB][k];
                double dAB = dist[mergeA][mergeB];
                int nA = clusterSize[mergeA];
                int nB = clusterSize[mergeB];
                int nK = clusterSize[k];
                int total = nA + nB + nK;
                newDist = qSqrt(
                    ((nA + nK) * dAK * dAK + (nB + nK) * dBK * dBK
                     - nK * dAB * dAB) / total);
            }

            dist[mergeA][k] = newDist;
            dist[k][mergeA] = newDist;
        }

        clusterSize[mergeA] += clusterSize[mergeB];
    }

    m_stats.totalMergeOps += mergeSteps.size();

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalMergeOps > 0 ? 1 : 1);

    emit clusteringCompleted(n - mergeSteps.size() + 1);
    return mergeSteps;
}

/**
 * @brief 根据距离阈值截断树状图获取聚类标签
 *
 * 遍历合并记录，在合并距离超过阈值时停止合并，
 * 使用并查集确定最终簇归属。
 *
 * @param mergeSteps 合并步骤记录
 * @param cutDistance 截断距离阈值
 * @return 各样本的聚类标签
 */
QVector<int> HierarchicalCluster12::cutDendrogram(
    const QVector<QPair<QPair<int, int>, double>>& mergeSteps, double cutDistance)
{
    if (mergeSteps.isEmpty()) return {};

    /* 推断总样本数 */
    int maxIdx = 0;
    for (const auto& step : mergeSteps) {
        maxIdx = qMax(maxIdx, qMax(step.first.first, step.first.second));
    }
    const int n = maxIdx + 1;

    /* 并查集 */
    QVector<int> parent(n);
    for (int i = 0; i < n; ++i)
        parent[i] = i;

    /* 只合并距离小于阈值的步骤 */
    for (const auto& step : mergeSteps) {
        if (step.second > cutDistance) break;

        int a = step.first.first;
        int b = step.first.second;
        if (a < 0 || a >= n || b < 0 || b >= n) continue;

        /* 路径压缩查找根 */
        int rootA = a, rootB = b;
        while (parent[rootA] != rootA) rootA = parent[rootA];
        while (parent[rootB] != rootB) rootB = parent[rootB];
        parent[rootB] = rootA;
    }

    /* 提取簇标签 */
    QVector<int> labels(n);
    QMap<int, int> rootToLabel;
    int labelIdx = 0;
    for (int i = 0; i < n; ++i) {
        int root = i;
        while (parent[root] != root) root = parent[root];
        if (!rootToLabel.contains(root))
            rootToLabel[root] = labelIdx++;
        labels[i] = rootToLabel[root];
    }
    return labels;
}

/**
 * @brief 计算簇间Ward距离（方差最小化准则）
 *
 * Ward距离衡量合并两个簇后簇内方差的增量。
 * 公式: d(A,B) = |A|*|B| / (|A|+|B|) * ||cA - cB||^2
 *
 * @param clusterA 簇A中的数据点索引
 * @param clusterB 簇B中的数据点索引
 * @param centroids 各数据点的坐标（质心向量）
 * @return Ward距离值
 */
double HierarchicalCluster12::wardDistance(const QVector<int>& clusterA,
                                           const QVector<int>& clusterB,
                                           const QVector<QVector<double>>& centroids)
{
    if (clusterA.isEmpty() || clusterB.isEmpty()) return 0.0;

    /* 计算两个簇各自的质心 */
    int dims = 0;
    for (int idx : clusterA) {
        if (idx >= 0 && idx < centroids.size()) {
            dims = qMax(dims, centroids[idx].size());
            break;
        }
    }
    if (dims == 0) return 0.0;

    QVector<double> centroidA(dims, 0.0);
    QVector<double> centroidB(dims, 0.0);

    for (int idx : clusterA) {
        if (idx < 0 || idx >= centroids.size()) continue;
        for (int d = 0; d < qMin(dims, centroids[idx].size()); ++d)
            centroidA[d] += centroids[idx][d];
    }
    for (int d = 0; d < dims; ++d)
        centroidA[d] /= clusterA.size();

    for (int idx : clusterB) {
        if (idx < 0 || idx >= centroids.size()) continue;
        for (int d = 0; d < qMin(dims, centroids[idx].size()); ++d)
            centroidB[d] += centroids[idx][d];
    }
    for (int d = 0; d < dims; ++d)
        centroidB[d] /= clusterB.size();

    /* 计算质心间欧氏距离平方 */
    double sqDist = 0.0;
    for (int d = 0; d < dims; ++d) {
        double diff = centroidA[d] - centroidB[d];
        sqDist += diff * diff;
    }

    /* Ward距离公式 */
    int nA = clusterA.size();
    int nB = clusterB.size();
    return (static_cast<double>(nA * nB) / (nA + nB)) * sqDist;
}
