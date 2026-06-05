#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class KMedoids4 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalClusterings = 0; int totalPoints = 0; double avgProcessingTimeMs = 0.0; };
    explicit KMedoids4(QObject* parent = nullptr);
    void setNumClusters(int k);
    void setMaxIterations(int iter);
    void setDistanceMetric(const QString& metric);
    QVector<int> cluster(const QVector<QVector<double>>& points);
    QVector<int> medoidIndices() const { return m_medoids; }
    double totalCost() const { return m_totalCost; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void clusteringCompleted(int clusters, double cost);
private:
    int m_k = 3; int m_maxIter = 100; QString m_metric = "euclidean";
    QVector<int> m_medoids; double m_totalCost = 0.0;
    double distance(const QVector<double>& a, const QVector<double>& b) const;
    double assignCost(const QVector<QVector<double>>& pts, const QVector<int>& meds);
    Stats m_stats; double m_timeSum = 0.0;
};
