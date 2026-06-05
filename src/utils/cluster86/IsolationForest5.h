#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief Isolation Forest异常检测
 *
 * 基于随机隔离的异常检测算法，对高维数据高效。
 */
class IsolationForest5 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalFitted = 0;
        int totalAnomaliesDetected = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit IsolationForest5(QObject* parent = nullptr);

    /** @brief 训练隔离森林 */
    void fit(const QVector<QVector<double>>& data, int numTrees = 100, int sampleSize = 256);

    /** @brief 预测异常分数(0~1，越大越异常) */
    QVector<double> score(const QVector<QVector<double>>& data) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void anomalyDetected(int sampleIndex, double score);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_numTrees = 100;
};
