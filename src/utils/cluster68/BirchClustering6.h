#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class BirchClustering6 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalClusterings = 0; int totalPoints = 0; double avgProcessingTimeMs = 0.0; };
    explicit BirchClustering6(QObject* parent = nullptr);
    void setThreshold(double t);
    void setBranchFactor(int b);
    QVector<int> cluster(const QVector<QVector<double>>& points);
    int treeSize() const { return m_treeSize; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void clusteringCompleted(int clusters, int treeSize);
private:
    double m_threshold = 0.5; int m_branch = 50; int m_treeSize = 0;
    Stats m_stats; double m_timeSum = 0.0;
};
