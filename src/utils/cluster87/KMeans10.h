#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief K-Means聚类算法工具类
 *
 * 提供K-Means聚类分析功能，支持设置簇数量、拟合数据、
 * 预测样本归属簇以及获取聚类中心。
 */
class KMeans10 : public QObject {
    Q_OBJECT
public:
    /// 聚类统计信息
    struct Stats {
        int totalClusterings = 0;   ///< 总聚类次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit KMeans10(QObject* parent = nullptr);

    /** @brief 设置聚类簇数量 */
    void setClusterCount(int count);

    /** @brief 对输入数据集执行K-Means拟合 */
    void fit(const QVector<QVector<double>>& data);

    /** @brief 预测单个样本所属簇编号 */
    int predict(const QVector<double>& sample);

    /** @brief 获取当前聚类中心坐标 */
    QVector<QVector<double>> centroids() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 聚类完成信号，返回簇数量和迭代次数 */
    void clusteringCompleted(int clusterCount, int iterations);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_clusterCount = 3;
    QVector<QVector<double>> m_centroids;
};
