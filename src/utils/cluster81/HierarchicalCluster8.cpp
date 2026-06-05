#include "HierarchicalCluster8.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化层次聚类器
 * @param parent 父QObject对象指针
 */
HierarchicalCluster8::HierarchicalCluster8(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 执行凝聚式层次聚类
 *
 * 自底向上逐步合并最近的两个簇，直到只剩一个簇。
 * 支持三种链接准则：
 * - "single": 单链接，取两个簇间最小距离
 * - "complete": 完全链接，取两个簇间最大距离
 * - "ward": Ward方法，最小化簇内方差增量
 *
 * 返回合并历史（每步合并的两个簇编号）供cutDendrogram使用。
 *
 * @param data 输入数据，每行为一个样本
 * @param linkage 链接准则名称
 * @return 合并历史，每对表示被合并的两个簇编号
 */
QVector<QPair<int, int>> HierarchicalCluster8::fit(const QVector<QVector<double>>& data,
                                                     const QString& linkage)
{
    QElapsedTimer timer;
    timer.start();

    const int n = data.size();
    if (n <= 1) return {};

    /// 计算距离矩阵
    QVector<QVector<double>> distMatrix(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double d = 0.0;
            const int dim = qMin(data[i].size(), data[j].size());
            for (int k = 0; k < dim; ++k) {
                double diff = data[i][k] - data[j][k];
                d += diff * diff;
            }
            d = std::sqrt(d);
            distMatrix[i][j] = d;
            distMatrix[j][i] = d;
        }
    }

    /// 初始化：每个样本为一个独立簇
    QVector<QVector<int>> clusters(n);
    QVector<bool> active(n, true);
    for (int i = 0; i < n; ++i) clusters[i] = {i};

    m_mergeHistory.clear();
    int totalMerges = 0;

    /// 凝聚聚类主循环
    for (int step = 0; step < n - 1; ++step) {
        double minDist = 1e30;
        int mergeI = -1, mergeJ = -1;

        /// 寻找距离最近的两个活跃簇
        for (int i = 0; i < n; ++i) {
            if (!active[i]) continue;
            for (int j = i + 1; j < n; ++j) {
                if (!active[j]) continue;

                double clusterDist = 0.0;
                if (linkage == QStringLiteral("single")) {
                    /// 单链接：最小距离
                    clusterDist = 1e30;
                    for (int a : clusters[i]) {
                        for (int b : clusters[j]) {
                            clusterDist = qMin(clusterDist, distMatrix[a][b]);
                        }
                    }
                } else if (linkage == QStringLiteral("complete")) {
                    /// 完全链接：最大距离
                    clusterDist = 0.0;
                    for (int a : clusters[i]) {
                        for (int b : clusters[j]) {
                            clusterDist = qMax(clusterDist, distMatrix[a][b]);
                        }
                    }
                } else {
                    /// Ward方法：方差增量
                    double sum = 0.0;
                    int count = 0;
                    for (int a : clusters[i]) {
                        for (int b : clusters[j]) {
                            sum += distMatrix[a][b];
                            ++count;
                        }
                    }
                    clusterDist = sum / qMax(1, count);
                }

                if (clusterDist < minDist) {
                    minDist = clusterDist;
                    mergeI = i;
                    mergeJ = j;
                }
            }
        }

        if (mergeI < 0) break;

        /// 合并两个簇
        clusters[mergeI].append(clusters[mergeJ]);
        active[mergeJ] = false;
        m_mergeHistory.append(qMakePair(mergeI, mergeJ));
        ++totalMerges;

        emit mergeCompleted(mergeI, mergeJ, minDist);
    }

    /// 更新统计信息
    m_stats.totalClustersBuilt++;
    m_stats.totalMergesPerformed += totalMerges;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClustersBuilt;

    return m_mergeHistory;
}

/**
 * @brief 根据距离阈值切割树状图获取簇标签
 *
 * 遍历合并历史，在合并距离超过阈值时停止合并，
 * 此时的活跃簇即为最终聚类结果。
 *
 * @param threshold 距离阈值，超过此值的合并不执行
 * @return 每个样本的簇标签
 */
QVector<int> HierarchicalCluster8::cutDendrogram(double threshold) const
{
    const int n = m_mergeHistory.size() + 1;  ///< 原始样本数
    if (n <= 1) return {0};

    /// 初始化并查集
    QVector<int> parent(n);
    for (int i = 0; i < n; ++i) parent[i] = i;

    /// 执行不超过阈值的合并
    int numMerges = qMin(static_cast<int>(threshold), static_cast<int>(m_mergeHistory.size()));
    for (int i = 0; i < numMerges; ++i) {
        int a = m_mergeHistory[i].first;
        int b = m_mergeHistory[i].second;
        /// 简化：将b的根指向a的根
        parent[b] = a;
    }

    /// 分配标签
    QVector<int> labels(n);
    int label = 0;
    for (int i = 0; i < n; ++i) {
        int root = i;
        while (parent[root] != root) root = parent[root];
        labels[i] = root;
    }

    return labels;
}

/**
 * @brief 获取当前统计数据
 * @return 包含聚类次数、合并操作数和平均耗时的Stats结构
 */
HierarchicalCluster8::Stats HierarchicalCluster8::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据为零值，清除合并历史
 */
void HierarchicalCluster8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_mergeHistory.clear();
}
