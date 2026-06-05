/**
 * @file HierarchicalCluster5.cpp
 * @brief 层次聚类算法实现
 *
 * 实现凝聚式(Agglomerative)层次聚类算法，支持多种链接准则：
 * single(单链接)、complete(全链接)、average(平均链接)、ward(Ward方差最小化)。
 * 输出完整树状图(dendrogram)和指定簇数的分割结果。
 * 使用QElapsedTimer计时并累积统计信息。
 */

#include "utils/cluster48/HierarchicalCluster5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <limits>

/**
 * @class HierarchicalCluster5
 * @brief 凝聚式层次聚类器
 *
 * 算法流程：
 * 1. 初始化：每个数据点为一个簇
 * 2. 迭代：合并距离最近的两个簇，记录合并高度
 * 3. 重复直到所有点合并为一个簇
 * 4. 根据目标簇数k截断树状图得到最终聚类
 *
 * 距离矩阵采用完全存储，适合中小规模数据集。
 */

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject指针
 */
HierarchicalCluster5::HierarchicalCluster5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置链接方法
 * @param method 链接方法名称："single"/"complete"/"average"/"ward"
 */
void HierarchicalCluster5::setLinkage(const QString& method)
{
    if (method == "single" || method == "complete" ||
        method == "average" || method == "ward") {
        m_linkage = method;
    }
}

/**
 * @brief 设置目标簇数
 * @param k 簇的数量，必须 >= 1
 */
void HierarchicalCluster5::setNumClusters(int k)
{
    m_k = qMax(1, k);
}

/**
 * @brief 执行层次聚类
 *
 * 1. 计算所有点对之间的欧氏距离矩阵
 * 2. 迭代合并最近的两个簇，更新距离矩阵
 * 3. 记录每次合并的簇对和合并高度
 * 4. 根据目标簇数截断树状图
 *
 * @param data 输入数据矩阵，每个元素为特征向量
 * @return 每个样本的簇标签（0 到 k-1）
 */
QVector<int> HierarchicalCluster5::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    const int n = data.size();
    QVector<int> labels(n, 0);
    m_dendrogram.clear();
    m_heights.clear();

    if (n <= 1) {
        m_stats.totalClusterings++;
        m_stats.totalPoints += n;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = (m_stats.totalClusterings > 0)
            ? m_timeSum / m_stats.totalClusterings : 0.0;
        return labels;
    }

    /* 初始化：每个点为一个簇 */
    QVector<int> clusterId(n);
    for (int i = 0; i < n; ++i) clusterId[i] = i;
    int nextId = n;
    QVector<int> clusterSize(n, 1);
    int activeClusters = n;

    /* 计算初始距离矩阵（对称矩阵，只存上三角） */
    QVector<double> distMatrix(n * n, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double d = 0.0;
            const int dim = qMin(data[i].size(), data[j].size());
            for (int k = 0; k < dim; ++k) {
                double diff = data[i][k] - data[j][k];
                d += diff * diff;
            }
            distMatrix[i * n + j] = qSqrt(d);
        }
    }

    /* 跟踪每个簇的当前索引映射 */
    QVector<int> active(n);
    for (int i = 0; i < n; ++i) active[i] = i;

    /* 迭代合并 */
    while (activeClusters > 1) {
        /* 找到距离最近的两个活跃簇 */
        double minDist = std::numeric_limits<double>::max();
        int mergeI = -1, mergeJ = -1;

        for (int i = 0; i < activeClusters; ++i) {
            for (int j = i + 1; j < activeClusters; ++j) {
                int ai = active[i];
                int aj = active[j];
                int row = qMin(ai, aj);
                int col = qMax(ai, aj);
                double d = distMatrix[row * n + col];
                if (d < minDist) {
                    minDist = d;
                    mergeI = i;
                    mergeJ = j;
                }
            }
        }

        if (mergeI < 0) break;

        /* 记录合并到树状图 */
        int c1 = active[mergeI];
        int c2 = active[mergeJ];
        m_dendrogram.append({c1, c2});
        m_heights.append(minDist);

        /* 更新距离矩阵：用新簇(nextId)替换c1 */
        int size1 = clusterSize[c1];
        int size2 = clusterSize[c2];
        int newSize = size1 + size2;

        for (int k = 0; k < activeClusters; ++k) {
            if (k == mergeI || k == mergeJ) continue;
            int ak = active[k];
            double d1 = distMatrix[qMin(c1, ak) * n + qMax(c1, ak)];
            double d2 = distMatrix[qMin(c2, ak) * n + qMax(c2, ak)];

            double newDist;
            if (m_linkage == "single") {
                newDist = qMin(d1, d2);
            } else if (m_linkage == "complete") {
                newDist = qMax(d1, d2);
            } else if (m_linkage == "average") {
                newDist = (d1 * size1 + d2 * size2) / newSize;
            } else {
                /* Ward链接：基于方差增量 */
                int sk = clusterSize[ak];
                newDist = qSqrt((sk * size1 * d1 * d1 + sk * size2 * d2 * d2
                    - size1 * size2 * d1 * d1 * d2 * d2 / (size1 * size2))
                    / ((sk + newSize) * newSize));
                if (std::isnan(newDist) || std::isinf(newDist)) {
                    newDist = qMax(d1, d2);
                }
            }
            distMatrix[qMin(c1, ak) * n + qMax(c1, ak)] = newDist;
        }

        clusterSize[c1] = newSize;
        clusterId[c1] = nextId++;

        /* 移除c2（用最后一个活跃簇覆盖） */
        active[mergeJ] = active[activeClusters - 1];
        activeClusters--;
        if (mergeI >= activeClusters) {
            /* mergeI被覆盖了，调整 */
            active[mergeI] = active[mergeJ];
        }

        /* 更新所有点的簇标签 */
        for (int i = 0; i < n; ++i) {
            if (clusterId[i] == c2 || clusterId[i] == c1) {
                clusterId[i] = c1;
            }
        }
    }

    /* 根据目标簇数k截断树状图 */
    if (m_k >= n) {
        for (int i = 0; i < n; ++i) labels[i] = i;
    } else {
        /* 从树状图倒推，保留最后(n-k)次合并 */
        QVector<int> unionFind(n);
        for (int i = 0; i < n; ++i) unionFind[i] = i;

        for (int i = 0; i < n - m_k; ++i) {
            if (i >= m_dendrogram.size()) break;
            int a = m_dendrogram[i].first;
            int b = m_dendrogram[i].second;
            /* 将b的标签改为a */
            int labelA = unionFind[a];
            int labelB = unionFind[b];
            for (int j = 0; j < n; ++j) {
                if (unionFind[j] == labelB) unionFind[j] = labelA;
            }
        }

        /* 重新编号为连续整数 */
        QMap<int, int> remap;
        int nextLabel = 0;
        for (int i = 0; i < n; ++i) {
            if (!remap.contains(unionFind[i])) {
                remap[unionFind[i]] = nextLabel++;
            }
            labels[i] = remap[unionFind[i]];
        }
    }

    m_stats.totalClusterings++;
    m_stats.totalPoints += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalClusterings > 0)
        ? m_timeSum / m_stats.totalClusterings : 0.0;

    emit clusteringCompleted(n, m_k);
    return labels;
}

/**
 * @brief 重置统计数据
 */
void HierarchicalCluster5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
