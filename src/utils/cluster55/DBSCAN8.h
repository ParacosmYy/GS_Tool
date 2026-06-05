#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class DBSCAN8 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalClusterings = 0; int totalPoints = 0; double avgProcessingTimeMs = 0.0; };
    explicit DBSCAN8(QObject* parent = nullptr);
    void setEpsilon(double eps);
    void setMinPoints(int minPts);
    void setDistanceMetric(const QString& metric);
    QVector<int> cluster(const QVector<QVector<double>>& points);
    int numClusters() const { return m_numClusters; }
    int numNoisePoints() const { return m_noiseCount; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void clusteringCompleted(int clusters, int noise);
private:
    double m_eps = 0.5; int m_minPts = 5; QString m_metric = "euclidean";
    int m_numClusters = 0; int m_noiseCount = 0;
    QVector<int> regionQuery(const QVector<QVector<double>>& pts, int idx);
    double distance(const QVector<double>& a, const QVector<double>& b) const;
    Stats m_stats; double m_timeSum = 0.0;
};
