/**
 * @file AgglomerativeCluster2.h
 * @brief Agglomerative clustering enhanced
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QPair>
class AgglomerativeCluster2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalClusterings = 0; int totalPointsProcessed = 0; double avgProcessingTimeMs = 0.0; };
    enum Linkage { Single, Complete, Average, Ward };
    explicit AgglomerativeCluster2(QObject* parent = nullptr);
    void setLinkage(Linkage method);
    void setDistanceThreshold(double threshold);
    QVector<int> fit(const QVector<QVector<double>>& data, int k);
    QVector<QPair<int,int>> dendrogram() const;
    QVector<int> cutTree(int k) const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void clusteringComplete(int numClusters);
private:
    double clusterDistance(const QVector<int>& a, const QVector<int>& b,
                          const QVector<QVector<double>>& data) const;
    Linkage m_linkage = Ward; double m_threshold = 0.0;
    QVector<QPair<int,int>> m_merges; QVector<double> m_mergeDistances;
    Stats m_stats; double m_timeSum = 0.0;
};
