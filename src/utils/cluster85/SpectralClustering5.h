#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 谱聚类算法
 *
 * 基于图拉普拉斯矩阵特征分解的聚类方法。
 */
class SpectralClustering5 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalClusterings = 0;
        int totalEigenDecomps = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SpectralClustering5(QObject* parent = nullptr);

    /** @brief 执行谱聚类 */
    QVector<int> fit(const QVector<QVector<double>>& affinity, int k);

    /** @brief 构建亲和矩阵(高斯核) */
    QVector<QVector<double>> buildAffinity(const QVector<QVector<double>>& data,
                                            double sigma) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int k, double eigengap);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
};
