#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 高斯混合模型(GMM)聚类
 *
 * 基于EM算法的高斯混合模型，支持多维度数据软聚类。
 */
class GaussianMixture14 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalFitted = 0;
        int totalPredictions = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GaussianMixture14(QObject* parent = nullptr);

    /** @brief 使用EM算法拟合模型 */
    bool fit(const QVector<QVector<double>>& data, int components, int maxIter = 100);

    /** @brief 预测样本所属簇 */
    QVector<int> predict(const QVector<QVector<double>>& data) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fitCompleted(int iterations, double finalLogLikelihood);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_components = 0;
};
