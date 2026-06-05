#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief Mean Shift均值漂移聚类
 *
 * 基于核密度估计的非参数聚类算法，自动确定簇数。
 */
class MeanShift6 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalPointsShifted = 0;
        int totalModesFound = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MeanShift6(QObject* parent = nullptr);

    /** @brief 执行Mean Shift聚类 */
    QVector<int> fit(const QVector<QVector<double>>& data, double bandwidth, int maxIter = 300);

    /** @brief 获取收敛的簇中心(模式点) */
    QVector<QVector<double>> modes() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void shiftCompleted(int iteration, double totalShift);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QVector<QVector<double>> m_modes;
};
