/**
 * @file KMeansClusterer.h
 * @brief K-Means聚类器 — 将数据自动分组
 *
 * 功能: 支持K-Means++初始化、肘部法则确定最优K值、轮廓系数评估，
 *       统计聚类次数/迭代总步数/平均迭代次数。
 */
#ifndef KMEANSCLUSTERER_H
#define KMEANSCLUSTERER_H

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @class KMeansClusterer
 * @brief 一维K-Means聚类，支持自动确定最优K值
 */
class KMeansClusterer : public QObject {
    Q_OBJECT
public:
    /** 聚类结果 */
    struct Cluster {
        double centroid;            ///< 聚类中心
        QVector<int> memberIndices; ///< 成员索引列表
        double variance;            ///< 簇内方差
    };

    /** 聚类统计 */
    struct Stats {
        quint64 totalClusterings = 0;      ///< 总聚类次数
        quint64 totalIterations = 0;       ///< 累计迭代步数
        double  avgIterations = 0.0;       ///< 平均迭代次数
        double  averageProcessingTimeMs = 0.0;
    };

    explicit KMeansClusterer(QObject* parent = nullptr);

    /** 设置参数 */
    void setK(int k);
    void setMaxIterations(int maxIter);
    void setTolerance(double tol);

    /** 执行聚类 */
    QList<Cluster> cluster(const QVector<double>& data);

    /** 自动确定最优K(肘部法则) */
    int findOptimalK(const QVector<double>& data, int maxK = 10) const;

    /** 计算轮廓系数 */
    double silhouetteScore(const QVector<double>& data,
                           const QList<Cluster>& clusters) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringComplete(int k, int iterations);
    void iterationStep(int step, double inertia);

private:
    double distance(double a, double b) const;
    double computeInertia(const QVector<double>& data,
                          const QList<Cluster>& clusters) const;
    QList<Cluster> buildClusters(const QVector<double>& data,
                                  const QVector<int>& assignments,
                                  const QVector<double>& centroids) const;

    int m_k;
    int m_maxIterations;
    double m_tolerance;
    Stats m_stats;
    double m_timeSum;
};

#endif // KMEANSCLUSTERER_H
