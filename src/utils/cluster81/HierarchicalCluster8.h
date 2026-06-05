#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 层次聚类算法
 *
 * 自底向上凝聚式层次聚类，支持多种链接准则。
 */
class HierarchicalCluster8 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalClustersBuilt = 0;
        int totalMergesPerformed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit HierarchicalCluster8(QObject* parent = nullptr);

    /** @brief 执行层次聚类，返回合并历史 */
    QVector<QPair<int, int>> fit(const QVector<QVector<double>>& data,
                                  const QString& linkage = "ward");

    /** @brief 根据距离阈值切割树状图获取簇标签 */
    QVector<int> cutDendrogram(double threshold) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void mergeCompleted(int clusterA, int clusterB, double distance);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QVector<QPair<int, int>> m_mergeHistory;
};
