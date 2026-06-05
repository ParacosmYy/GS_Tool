#include "HierarchicalCluster11.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化层次聚类引擎
 * @param parent 父对象指针
 */
HierarchicalCluster11::HierarchicalCluster11(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void HierarchicalCluster11::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 执行层次聚类
 *
 * 自底向上合并最近的两个簇，直到只剩一个簇。
 * 支持三种链接策略：单链接(0)、全链接(1)、平均链接(2)。
 * 每次合并记录合并步骤，最终形成完整的树状图。
 *
 * @param distanceMatrix n×n距离矩阵（对称）
 * @param linkageType 链接策略: 0=单链接 1=全链接 2=平均链接
 * @return 合并步骤序列（树状图）
 */
QVector<HierarchicalCluster11::MergeStep> HierarchicalCluster11::fit(
    const QVector<QVector<double>>& distanceMatrix, int linkageType)
{
    QElapsedTimer timer;
    timer.start();

    QVector<MergeStep> mergeSteps;
    const int n = distanceMatrix.size();
    if (n == 0) {
        emit clusteringCompleted(0);
        return mergeSteps;
    }

    /* 初始化活跃簇集合 */
    QVector<QVector<int>> clusters(n);
    for (int i = 0; i < n; ++i) {
        clusters[i] = {i};
    }

    QVector<bool> active(n, true);
    m_mergeDistances.clear();

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
        MergeStep ms;
        ms.clusterA = mergeA;
        ms.clusterB = mergeB;
        ms.distance = minDist;
        ms.newSize = clusters[mergeA].size() + clusters[mergeB].size();
        mergeSteps.append(ms);
        m_mergeDistances.append(minDist);

        /* 合并B到A */
        clusters[mergeA].append(clusters[mergeB]);
        active[mergeB] = false;

        /* 更新距离矩阵 */
        for (int k = 0; k < n; ++k) {
            if (!active[k] || k == mergeA) continue;
            double newDist = 0.0;
            if (linkageType == 0) {
                /* 单链接：最小距离 */
                newDist = qMin(dist[mergeA][k], dist[mergeB][k]);
            } else if (linkageType == 1) {
                /* 全链接：最大距离 */
                newDist = qMax(dist[mergeA][k], dist[mergeB][k]);
            } else {
                /* 平均链接：簇间平均距离 */
                double sumA = 0.0, sumB = 0.0;
                for (int ci : clusters[mergeA]) {
                    for (int cj : clusters[k]) {
                        if (ci < distanceMatrix.size() && cj < distanceMatrix.size())
                            sumA += distanceMatrix[ci][cj];
                    }
                }
                sumA /= (clusters[mergeA].size() * clusters[k].size());
                for (int ci : clusters[mergeB]) {
                    for (int cj : clusters[k]) {
                        if (ci < distanceMatrix.size() && cj < distanceMatrix.size())
                            sumB += distanceMatrix[ci][cj];
                    }
                }
                sumB /= (clusters[mergeB].size() * clusters[k].size());
                newDist = (sumA * clusters[mergeA].size() + sumB * clusters[mergeB].size())
                          / (clusters[mergeA].size() + clusters[mergeB].size());
            }
            dist[mergeA][k] = newDist;
            dist[k][mergeA] = newDist;
        }
    }

    m_stats.totalMergeSteps += mergeSteps.size();

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalClusteringRuns++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusteringRuns;

    emit clusteringCompleted(n - mergeSteps.size() + 1);
    return mergeSteps;
}

/**
 * @brief 从树状图切割指定数量的簇
 *
 * 在第(n-k)次合并后截断，得到k个簇。
 *
 * @param mergeSteps 合并步骤序列
 * @param k 目标簇数
 * @return 每个样本的簇标签
 */
QVector<int> HierarchicalCluster11::cutTree(const QVector<MergeStep>& mergeSteps, int k) const
{
    const int n = mergeSteps.size() + 1;
    if (k <= 0 || k > n) return {};

    /* 使用并查集跟踪合并 */
    QVector<int> parent(n);
    for (int i = 0; i < n; ++i) parent[i] = i;

    /* 只执行前n-k步合并 */
    int stepsToExecute = n - k;
    for (int i = 0; i < stepsToExecute && i < mergeSteps.size(); ++i) {
        int a = mergeSteps[i].clusterA;
        int b = mergeSteps[i].clusterB;
        /* 将b的根指向a的根 */
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
        if (!rootToLabel.contains(root)) {
            rootToLabel[root] = labelIdx++;
        }
        labels[i] = rootToLabel[root];
    }
    return labels;
}

/**
 * @brief 计算轮廓系数评估聚类质量
 *
 * 轮廓系数衡量样本与其所属簇的紧密度和与其他簇的分离度。
 * 值域[-1, 1]，越大表示聚类效果越好。
 *
 * @param labels 簇标签
 * @param distanceMatrix 距离矩阵
 * @return 平均轮廓系数
 */
double HierarchicalCluster11::silhouetteScore(const QVector<int>& labels,
                                               const QVector<QVector<double>>& distanceMatrix) const
{
    const int n = labels.size();
    if (n == 0 || n != distanceMatrix.size()) return 0.0;

    double totalScore = 0.0;
    for (int i = 0; i < n; ++i) {
        /* 计算簇内平均距离a(i) */
        double aSum = 0.0;
        int aCount = 0;
        /* 计算最近其他簇平均距离b(i) */
        QMap<int, double> clusterDist;
        QMap<int, int> clusterCount;

        for (int j = 0; j < n; ++j) {
            if (i == j) continue;
            double d = (j < distanceMatrix[i].size()) ? distanceMatrix[i][j] : 0.0;
            if (labels[j] == labels[i]) {
                aSum += d;
                aCount++;
            } else {
                clusterDist[labels[j]] += d;
                clusterCount[labels[j]]++;
            }
        }

        double a = (aCount > 0) ? aSum / aCount : 0.0;
        double b = 1e18;
        for (auto it = clusterDist.begin(); it != clusterDist.end(); ++it) {
            int cnt = clusterCount[it.key()];
            if (cnt > 0) b = qMin(b, it.value() / cnt);
        }
        if (b > 1e17) b = 0.0;

        double denom = qMax(a, b);
        totalScore += (denom > 0) ? (b - a) / denom : 0.0;
    }
    return totalScore / n;
}
