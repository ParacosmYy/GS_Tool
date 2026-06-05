#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief K-Means聚类算法
 *
 * 经典K-Means++初始化的均值聚类，支持多种距离度量。
 */
class KMeans9 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalFitted = 0;
        int totalPredictions = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit KMeans9(QObject* parent = nullptr);

    /** @brief 拟合K-Means模型 */
    bool fit(const QVector<QVector<double>>& data, int k, int maxIter = 300);

    /** @brief 预测样本所属簇 */
    QVector<int> predict(const QVector<QVector<double>>& data) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void iterationCompleted(int iteration, double inertia);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QVector<QVector<double>> m_centroids;
};
