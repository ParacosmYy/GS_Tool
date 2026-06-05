#pragma once
#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

class DBSCAN7 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalClusterings = 0; int totalPoints = 0; double avgProcessingTimeMs = 0.0; };
    explicit DBSCAN7(QObject* parent = nullptr);
    void setEpsilon(double eps);
    void setMinPoints(int minPts);
    QVector<int> fit(const QVector<QVector<double>>& data);
    int numClusters() const { return m_numClusters; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void clusteringCompleted(int points, int clusters);
private:
    double m_eps = 0.5; int m_minPts = 5; int m_numClusters = 0;
    QVector<int> expandCluster(const QVector<QVector<double>>& data, int pt,
                                QVector<int>& labels, int cluster);
    QVector<int> rangeQuery(const QVector<QVector<double>>& data, int pt) const;
    Stats m_stats; double m_timeSum = 0.0;
};
