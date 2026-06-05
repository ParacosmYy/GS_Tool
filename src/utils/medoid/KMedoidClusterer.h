/**
 * @file KMedoidClusterer.h
 * @brief K-Medoid聚类器 — 基于中心点的鲁棒聚类
 *
 * 功能: K-Medoids(PAM)聚类，比K-Means对离群值更鲁棒，
 *       统计聚类次数/迭代步数/轮廓系数。
 */
#ifndef KMEDOIDCLUSTERER_H
#define KMEDOIDCLUSTERER_H

#include <QObject>
#include <QVector>
#include <QList>

/**
 * @class KMedoidClusterer
 * @brief K-Medoids聚类，使用实际数据点作为中心
 */
class KMedoidClusterer : public QObject {
    Q_OBJECT
public:
    /** 聚类结果 */
    struct Cluster {
        int medoidIndex;                ///< 中心点索引
        QVector<int> memberIndices;     ///< 成员索引
        double totalDistance;            ///< 簇内总距离
    };

    /** 聚类统计 */
    struct Stats {
        quint64 totalClusterings = 0;
        quint64 totalSwaps = 0;
        double  avgIterations = 0.0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit KMedoidClusterer(QObject* parent = nullptr);

    void setK(int k);
    void setMaxIterations(int maxIter);

    /** 执行聚类 */
    QList<Cluster> cluster(const QVector<double>& data);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringComplete(int k, int iterations);

private:
    double totalCost(const QVector<double>& data, const QVector<int>& medoids,
                     QVector<int>& assignments) const;

    int m_k;
    int m_maxIterations;
    Stats m_stats;
    double m_timeSum;
};

#endif // KMEDOIDCLUSTERER_H
