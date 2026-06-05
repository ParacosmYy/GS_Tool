/**
 * @file AgglomerativeClusterer.cpp
 * @brief 层次凝聚聚类实现 — 自底向上合并 + 四种链接策略
 */

#include "utils/cluster2/AgglomerativeClusterer.h"

#include <QElapsedTimer>
#include <QtMath>
#include <limits>

/**
 * @brief 构造函数
 * @param numClusters 目标簇数
 * @param parent 父对象
 */
AgglomerativeClusterer::AgglomerativeClusterer(int numClusters,
                                                 QObject* parent)
    : QObject(parent)
    , m_numClusters(qMax(1, numClusters))
    , m_linkage(Linkage::Average)
    , m_timeSum(0.0)
{
}

/**
 * @brief 执行层次凝聚聚类
 * @param data 数据点集
 * @return 每个点的簇标签
 *
 * 算法:
 * 1. 每个点初始为一个簇
 * 2. 计算所有簇对的距离矩阵
 * 3. 找到距离最小的两个簇合并
 * 4. 更新距离矩阵(Lance-Williams公式)
 * 5. 重复直到达到目标簇数
 */
QVector<int> AgglomerativeClusterer::fit(
    const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    m_dendrogram.clear();
    int n = data.size();
    QVector<int> labels(n, 0);

    if (n <= m_numClusters) {
        /* 点数不足，每个点独立标记 */
        for (int i = 0; i < n; ++i) labels[i] = i;
        double elapsed = static_cast<double>(timer.elapsed());
        ++m_stats.totalClusterings;
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs =
            m_timeSum / static_cast<double>(m_stats.totalClusterings);
        emit clusteringCompleted(n, n);
        return labels;
    }

    /* 初始化: 每个点一个簇 */
    QVector<int> clusterIds(n);
    QVector<int> clusterSizes(n, 1);
    for (int i = 0; i < n; ++i) clusterIds[i] = i;

    /* 初始距离矩阵 */
    QVector<QVector<double>> dist = distanceMatrix(data);
    /* 用大值填充对角线避免自合并 */
    double inf = std::numeric_limits<double>::max();
    for (int i = 0; i < n; ++i) dist[i][i] = inf;

    int activeClusters = n;
    /* 簇映射: 记录每个原始点最终属于哪个活跃簇 */
    QVector<int> pointToCluster(n);
    for (int i = 0; i < n; ++i) pointToCluster[i] = i;

    /* 被合并掉的簇索引集合 */
    QVector<bool> merged(n, false);

    while (activeClusters > m_numClusters) {
        /* 找距离最小的簇对 */
        double minDist = inf;
        int mergeA = -1, mergeB = -1;

        for (int i = 0; i < n; ++i) {
            if (merged[i]) continue;
            for (int j = i + 1; j < n; ++j) {
                if (merged[j]) continue;
                if (dist[i][j] < minDist) {
                    minDist = dist[i][j];
                    mergeA = i;
                    mergeB = j;
                }
            }
        }

        if (mergeA < 0) break;

        /* 记录合并历史 */
        m_dendrogram.append({clusterIds[mergeA], clusterIds[mergeB]});

        /* 合并B到A: 更新A与所有其他活跃簇的距离 */
        for (int k = 0; k < n; ++k) {
            if (k == mergeA || k == mergeB || merged[k]) continue;
            dist[mergeA][k] = linkageDistance(
                dist[mergeA][k], dist[mergeB][k],
                minDist,
                clusterSizes[mergeA], clusterSizes[mergeB],
                clusterSizes[k]);
            dist[k][mergeA] = dist[mergeA][k];
        }

        clusterSizes[mergeA] += clusterSizes[mergeB];
        merged[mergeB] = true;

        /* 更新点->簇映射 */
        for (int i = 0; i < n; ++i) {
            if (pointToCluster[i] == mergeB) {
                pointToCluster[i] = mergeA;
            }
        }

        --activeClusters;
    }

    /* 分配连续标签(0 ~ numClusters-1) */
    QMap<int, int> remap;
    int nextLabel = 0;
    for (int i = 0; i < n; ++i) {
        int cid = pointToCluster[i];
        if (!remap.contains(cid)) {
            remap[cid] = nextLabel++;
        }
        labels[i] = remap[cid];
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalClusterings;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs =
        m_timeSum / static_cast<double>(m_stats.totalClusterings);

    emit clusteringCompleted(m_numClusters, n);
    return labels;
}

/**
 * @brief 获取树状图合并顺序
 * @return 合并序列
 */
QVector<QPair<int, int>> AgglomerativeClusterer::dendrogram() const
{
    return m_dendrogram;
}

/**
 * @brief 计算距离矩阵
 * @param data 数据点集
 * @return 对称距离矩阵
 */
QVector<QVector<double>> AgglomerativeClusterer::distanceMatrix(
    const QVector<QVector<double>>& data) const
{
    int n = data.size();
    QVector<QVector<double>> dist(n, QVector<double>(n, 0.0));

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double d = euclidean(data[i], data[j]);
            dist[i][j] = d;
            dist[j][i] = d;
        }
    }
    return dist;
}

/**
 * @brief 欧氏距离
 * @param a 点A
 * @param b 点B
 * @return 距离值
 */
double AgglomerativeClusterer::euclidean(
    const QVector<double>& a, const QVector<double>& b) const
{
    double sum = 0.0;
    int dim = qMin(a.size(), b.size());
    for (int i = 0; i < dim; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return qSqrt(sum);
}

/**
 * @brief Lance-Williams链接距离公式
 * @param dAC 簇A-C距离
 * @param dBC 簇B-C距离
 * @param dAB 簇A-B距离
 * @param sizeA 簇A大小
 * @param sizeB 簇B大小
 * @param sizeC 簇C大小
 * @return 合并簇(A+B)与C的距离
 */
double AgglomerativeClusterer::linkageDistance(
    double dAC, double dBC, double dAB,
    int sizeA, int sizeB, int sizeC) const
{
    double nA = static_cast<double>(sizeA);
    double nB = static_cast<double>(sizeB);
    double nC = static_cast<double>(sizeC);
    double total = nA + nB;

    switch (m_linkage) {
    case Linkage::Single:
        /* min(dAC, dBC) */
        return qMin(dAC, dBC);

    case Linkage::Complete:
        /* max(dAC, dBC) */
        return qMax(dAC, dBC);

    case Linkage::Average:
        /* 加权平均 */
        return (nA * dAC + nB * dBC) / total;

    case Linkage::Ward: {
        /* Ward方差最小化链接 */
        double alphaA = (nA + nC) / (total + nC);
        double alphaB = (nB + nC) / (total + nC);
        double beta   = nC / (total + nC);
        return qSqrt(alphaA * dAC * dAC + alphaB * dBC * dBC
                      - beta * dAB * dAB);
    }
    }
    return (nA * dAC + nB * dBC) / total;
}

/** @brief 设置链接策略 @param type 链接类型 */
void AgglomerativeClusterer::setLinkage(Linkage type)
{
    m_linkage = type;
}

/** @brief 设置目标簇数 @param k 簇数 */
void AgglomerativeClusterer::setNumClusters(int k)
{
    m_numClusters = qMax(1, k);
}

/** @brief 重置统计 */
void AgglomerativeClusterer::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_dendrogram.clear();
}
