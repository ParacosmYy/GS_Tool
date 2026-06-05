/**
 * @file HierarchicalCluster4.h
 * @brief 层次聚类4 — BIRCH预聚合+快速LINKAGE
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

class HierarchicalCluster4 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalClusterings = 0;
        int totalPointsProcessed = 0;
        int totalMerges = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit HierarchicalCluster4(QObject* parent = nullptr);

    void setLinkage(const QString& method);
    void setNumClusters(int k);
    void setPreclusterThreshold(double threshold);
    QVector<int> fit(const QVector<QVector<double>>& data);
    QVector<QPair<int,int>> dendrogram() const { return m_dendrogram; }
    QVector<double> dendrogramHeights() const { return m_heights; }
    QVector<QVector<int>> cutAtHeight(double h) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int numPoints, int numClusters);

private:
    QString m_linkage = "average";
    int m_numClusters = 2;
    double m_preclusterThreshold = 0.5;
    QVector<QPair<int,int>> m_dendrogram;
    QVector<double> m_heights;

    QVector<double> computeDistanceMatrix(const QVector<QVector<double>>& data) const;
    double linkageDistance(const QVector<double>& dist, int n,
                           const QVector<int>& c1, const QVector<int>& c2) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
