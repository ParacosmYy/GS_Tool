#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class KMeans8 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalClusterings = 0; int totalPoints = 0; double avgProcessingTimeMs = 0.0; };
    explicit KMeans8(QObject* parent = nullptr);
    void setNumClusters(int k);
    void setMaxIterations(int iter);
    void setDistanceMetric(const QString& metric);
    QVector<int> cluster(const QVector<QVector<double>>& points);
    QVector<QVector<double>> centroids() const { return m_centroids; }
    double inertia() const { return m_inertia; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void clusteringCompleted(int k, double inertia);
private:
    int m_k = 3; int m_maxIter = 300; QString m_metric = "euclidean";
    QVector<QVector<double>> m_centroids; double m_inertia = 0.0;
    double distance(const QVector<double>& a, const QVector<double>& b) const;
    Stats m_stats; double m_timeSum = 0.0;
};
