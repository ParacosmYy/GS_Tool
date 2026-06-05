#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief HierarchicalCluster7 - 层次聚类算法
 *
 * 支持凝聚和分裂策略的层次聚类，提供多种
 * 链接准则(单链接/全链接/平均链接/Ward)。
 */
class HierarchicalCluster7 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalMerges = 0;
        int totalClusters = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit HierarchicalCluster7(QObject* parent = nullptr);

    /** @brief 设置链接准则: single/complete/average/ward */
    void setLinkage(const QString& linkage);

    /** @brief 执行层次聚类，返回合并历史 */
    QVector<QPair<int,int>> fit(const QVector<QVector<double>>& data, int targetClusters);

    /** @brief 获取指定截断高度的簇标签 */
    QVector<int> getLabels(int numClusters) const;

    /** @brief 获取树状图的合并高度序列 */
    QVector<double> mergeHeights() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int clusterCount, double maxHeight);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QString m_linkage = "ward";
    QVector<QPair<int,int>> m_mergeHistory;
    QVector<double> m_mergeHeights;
};
