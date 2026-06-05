#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 层次聚类分析器
 *
 * 实现凝聚型层次聚类，支持多种链接策略和距离度量，
 * 输出树状图结构的聚类层次。
 */
class HierarchicalCluster10 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats {
        int totalPoints = 0;         ///< 已处理数据点数
        int totalMerges = 0;         ///< 合并操作总数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit HierarchicalCluster10(QObject* parent = nullptr);

    /** @brief 设置链接策略(single/complete/average/ward) */
    void setLinkage(const QString& method);
    /** @brief 设置距离度量(euclidean/manhattan/cosine) */
    void setMetric(const QString& metric);
    /** @brief 拟合数据，执行层次聚类 */
    void fit(const QVector<QVector<double>>& data);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 聚类完成，返回最终簇数 */
    void clusteringCompleted(int clusterCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QString m_linkage = "average";
    QString m_metric = "euclidean";
};
