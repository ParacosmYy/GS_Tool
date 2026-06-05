#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Mean Shift均值漂移聚类实现 (9参数配置)
 *
 * 基于核密度估计的聚类算法，自动发现簇数量，适用于任意形状的簇分布。
 */
class MeanShift9 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构
    struct Stats {
        int totalClusteringRuns = 0;   ///< 总聚类运行次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
        int clustersFound = 0;         ///< 发现的簇数量
    };

    explicit MeanShift9(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 执行Mean Shift聚类
     * @param data 输入数据点
     * @param bandwidth 核带宽参数
     * @return 每个样本的簇标签
     */
    QVector<int> fit(const QVector<QVector<double>>& data, double bandwidth);

    /**
     * @brief 自动估计最佳带宽
     * @param data 输入数据点
     * @return 建议的带宽值
     */
    double estimateBandwidth(const QVector<QVector<double>>& data) const;

    /**
     * @brief 获取聚类中心点
     * @return 各簇中心坐标
     */
    QVector<QVector<double>> clusterCenters() const { return m_centers; }

    /**
     * @brief 预测新样本所属簇
     * @param sample 输入样本
     * @return 簇标签
     */
    int predict(const QVector<double>& sample) const;

signals:
    /// 聚类完成信号
    void clusteringCompleted(int clusterCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QVector<QVector<double>> m_centers;
    double m_bandwidth = 1.0;
};
