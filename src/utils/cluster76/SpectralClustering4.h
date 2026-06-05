#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief SpectralClustering4 - 谱聚类算法
 *
 * 基于图拉普拉斯矩阵特征分解的聚类方法，
 * 适用于非凸形状数据集，支持归一化和非归一化切割。
 */
class SpectralClustering4 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalClusterings = 0;
        int totalEigenvalues = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SpectralClustering4(QObject* parent = nullptr);

    /** @brief 设置相似度矩阵构建方式: rbf/knn */
    void setAffinity(const QString& type, double param = 1.0);

    /** @brief 执行谱聚类，返回簇标签 */
    QVector<int> fit(const QVector<QVector<double>>& data, int numClusters);

    /** @brief 获取图拉普拉斯矩阵的前k个特征向量 */
    QVector<QVector<double>> spectralEmbedding(int dimensions) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int clusters, double cutValue);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QString m_affinityType = "rbf";
    double m_affinityParam = 1.0;
    QVector<QVector<double>> m_embedding;
};
