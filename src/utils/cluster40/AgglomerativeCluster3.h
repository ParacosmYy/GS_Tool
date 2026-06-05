/**
 * @file AgglomerativeCluster3.h
 * @brief 层次聚类3 — Ward方差最小+树状图剪枝
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

class AgglomerativeCluster3 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalClusterings = 0;
        int totalPointsProcessed = 0;
        int totalMerges = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit AgglomerativeCluster3(QObject* parent = nullptr);

    void setLinkage(const QString& linkage);
    void setNumClusters(int k);
    QVector<int> fit(const QVector<QVector<double>>& data);
    QVector<QPair<int,int>> mergeHistory() const { return m_merges; }
    QVector<double> mergeDistances() const { return m_mergeDist; }
    QVector<QVector<int>> cutTree(int k) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int numPoints, int numClusters);

private:
    QString m_linkage = "ward";
    int m_numClusters = 2;
    QVector<QPair<int,int>> m_merges;
    QVector<double> m_mergeDist;

    double wardDistance(const QVector<QVector<double>>& data,
                        const QVector<int>& c1, const QVector<int>& c2) const;
    double singleLink(const QVector<QVector<double>>& data,
                       const QVector<int>& c1, const QVector<int>& c2) const;
    double completeLink(const QVector<QVector<double>>& data,
                         const QVector<int>& c1, const QVector<int>& c2) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
