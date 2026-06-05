#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class HierarchicalCluster5 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalClusterings = 0; int totalPoints = 0; double avgProcessingTimeMs = 0.0; };
    explicit HierarchicalCluster5(QObject* parent = nullptr);
    void setLinkage(const QString& method);
    void setNumClusters(int k);
    QVector<int> fit(const QVector<QVector<double>>& data);
    QVector<QPair<int,int>> dendrogram() const { return m_dendrogram; }
    QVector<double> heights() const { return m_heights; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void clusteringCompleted(int points, int clusters);
private:
    QString m_linkage = "ward"; int m_k = 2;
    QVector<QPair<int,int>> m_dendrogram; QVector<double> m_heights;
    Stats m_stats; double m_timeSum = 0.0;
};
