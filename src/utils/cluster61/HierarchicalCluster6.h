#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class HierarchicalCluster6 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalClusterings = 0; int totalPoints = 0; double avgProcessingTimeMs = 0.0; };
    explicit HierarchicalCluster6(QObject* parent = nullptr);
    void setLinkage(const QString& type);
    void setNumClusters(int k);
    QVector<int> cluster(const QVector<QVector<double>>& points);
    QVector<QPair<int,int>> dendrogram() const { return m_dendrogram; }
    QVector<double> mergeDistances() const { return m_mergeDist; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void clusteringCompleted(int clusters, double height);
private:
    QString m_linkage = "ward"; int m_k = 3;
    QVector<QPair<int,int>> m_dendrogram; QVector<double> m_mergeDist;
    double clusterDistance(const QVector<QVector<double>>& pts, const QVector<int>& a, const QVector<int>& b);
    Stats m_stats; double m_timeSum = 0.0;
};
