/**
 * @file BirchClustering3.h
 * @brief BIRCH聚类3 — CF树动态调参+内存感知
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

class BirchClustering3 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalInsertions = 0;
        int totalPointsProcessed = 0;
        int totalMerges = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BirchClustering3(QObject* parent = nullptr);

    void setThreshold(double threshold);
    void setBranchingFactor(int b);
    void setMaxMemory(int maxEntries);
    void insert(const QVector<double>& point);
    void insertBatch(const QVector<QVector<double>>& points);
    QVector<QVector<int>> cluster(int k);
    void rebuild();

    int treeHeight() const;
    int leafCount() const;
    double memoryUsage() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void insertionCompleted(int totalPoints, int leafEntries);
    void clusteringCompleted(int k, int iterations);

private:
    struct CFEntry {
        int n = 0;
        QVector<double> linearSum;
        QVector<double> squareSum;
        QVector<int> pointIndices;
    };

    double m_threshold = 0.5;
    int m_branchingFactor = 50;
    int m_maxEntries = 100000;
    int m_dim = 0;
    QList<CFEntry> m_leafEntries;
    QVector<QVector<double>> m_points;

    double entryRadius(const CFEntry& e) const;
    double entryDistance(const CFEntry& a, const CFEntry& b) const;
    void mergeEntries(CFEntry& target, const CFEntry& source);

    Stats m_stats;
    double m_timeSum = 0.0;
};
