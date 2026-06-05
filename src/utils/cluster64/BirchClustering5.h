#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class BirchClustering5 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalClusterings = 0; int totalPoints = 0; double avgProcessingTimeMs = 0.0; };
    explicit BirchClustering5(QObject* parent = nullptr);
    void setThreshold(double t);
    void setBranchFactor(int b);
    void setNumClusters(int k);
    QVector<int> cluster(const QVector<QVector<double>>& points);
    int numLeafEntries() const { return m_leafCount; }
    int treeHeight() const { return m_height; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void clusteringCompleted(int clusters, int leaves);
private:
    double m_threshold = 0.5; int m_branch = 50; int m_k = 3;
    int m_leafCount = 0; int m_height = 0;
    struct CFEntry { int n; QVector<double> ls; double ss; };
    QVector<CFEntry> m_leafEntries;
    double entryDist(const CFEntry& a, const CFEntry& b) const;
    Stats m_stats; double m_timeSum = 0.0;
};
