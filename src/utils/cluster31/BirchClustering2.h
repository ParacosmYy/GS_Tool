/**
 * @file BirchClustering2.h
 * @brief BIRCH clustering enhanced - CF tree/bulk loading/global clustering
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QPair>
class BirchClustering2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalClusterings = 0; int totalPointsProcessed = 0; double avgProcessingTimeMs = 0.0; };
    explicit BirchClustering2(QObject* parent = nullptr);
    void setThreshold(double threshold);
    void setBranchingFactor(int b);
    void setMaxClusters(int k);
    QVector<int> fit(const QVector<QVector<double>>& data);
    QVector<QVector<double>> subclusterCenters() const;
    int numSubclusters() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void clusteringComplete(int numClusters);
private:
    double m_threshold = 0.5; int m_branching = 50; int m_maxClusters = 10;
    QVector<QVector<double>> m_centers; QVector<int> m_counts;
    Stats m_stats; double m_timeSum = 0.0;
};
