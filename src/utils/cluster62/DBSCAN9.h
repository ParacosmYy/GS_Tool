#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class DBSCAN9 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalClusterings = 0; int totalPoints = 0; double avgProcessingTimeMs = 0.0; };
    explicit DBSCAN9(QObject* parent = nullptr);
    void setEpsilon(double eps);
    void setMinPoints(int minPts);
    QVector<int> cluster(const QVector<QVector<double>>& points);
    int numClusters() const { return m_numClusters; }
    int numNoise() const { return m_noiseCount; }
    QVector<int> corePointIndices() const { return m_corePoints; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void clusteringCompleted(int clusters, int noise);
private:
    double m_eps = 0.5; int m_minPts = 5;
    int m_numClusters = 0; int m_noiseCount = 0;
    QVector<int> m_corePoints;
    double dist(const QVector<double>& a, const QVector<double>& b) const;
    Stats m_stats; double m_timeSum = 0.0;
};
