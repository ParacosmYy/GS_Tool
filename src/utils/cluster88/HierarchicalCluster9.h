#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 层次聚类工具类
 *
 * 提供层次聚类分析功能，支持设置链接策略(single/complete/average)，
 * 可生成聚类树和距离矩阵。
 */
class HierarchicalCluster9 : public QObject {
    Q_OBJECT
public:
    /// 聚类统计信息
    struct Stats {
        int totalClusterings = 0;   ///< 总聚类次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit HierarchicalCluster9(QObject* parent = nullptr);

    /** @brief 设置链接策略名称 */
    void setLinkage(const QString& linkage);

    /** @brief 对输入数据集执行层次聚类 */
    void fit(const QVector<QVector<double>>& data);

    /** @brief 获取距离矩阵 */
    QVector<QVector<double>> distanceMatrix() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 聚类完成信号，返回样本数量 */
    void clusteringCompleted(int sampleCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QString m_linkage = "average";
    QVector<QVector<double>> m_distMatrix;
};
