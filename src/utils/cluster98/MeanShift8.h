#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 均值漂移聚类器
 *
 * 基于核密度估计的非参数聚类算法，自动发现聚类数，
 * 通过迭代漂移到密度极大值实现聚类。
 */
class MeanShift8 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats {
        int totalPoints = 0;         ///< 已处理数据点数
        int totalIterations = 0;     ///< 迭代总次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit MeanShift8(QObject* parent = nullptr);

    /** @brief 设置核带宽参数 */
    void setBandwidth(double bandwidth);
    /** @brief 设置核函数类型(gaussian/flat) */
    void setKernel(const QString& kernel);
    /** @brief 拟合数据，执行均值漂移聚类 */
    void fit(const QVector<QVector<double>>& data);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 聚类完成，返回发现的簇数 */
    void clusteringCompleted(int clusterCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_bandwidth = 1.0;
    QString m_kernel = "gaussian";
};
