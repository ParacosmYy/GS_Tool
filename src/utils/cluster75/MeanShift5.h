#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief MeanShift5 - 均值漂移聚类算法
 *
 * 基于核密度估计的非参数聚类，自动发现簇数量，
 * 通过均值漂移迭代收敛到密度极大值。
 */
class MeanShift5 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalShifts = 0;
        int totalClusters = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MeanShift5(QObject* parent = nullptr);

    /** @brief 设置带宽参数(核函数半径) */
    void setBandwidth(double bandwidth);

    /** @brief 设置核函数类型: gaussian/flat */
    void setKernel(const QString& kernelType);

    /** @brief 执行均值漂移聚类 */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief 获取各簇的模态点(密度极大值) */
    QVector<QVector<double>> clusterModes() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int clusterCount, int iterations);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_bandwidth = 1.0;
    QString m_kernelType = "gaussian";
    QVector<QVector<double>> m_modes;
};
