/**
 * @file OPTICS3.h
 * @brief OPTICS聚类3 — 层次密度聚类+可提取DBSCAN
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

class OPTICS3 : public QObject
{
    Q_OBJECT

public:
    struct ClusterResult {
        QVector<int> coreIndices;
        QVector<double> reachabilityDist;
        QVector<double> coreDist;
        QVector<int> ordering;
    };

    struct Stats {
        int totalClusterings = 0;
        int totalPointsProcessed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit OPTICS3(QObject* parent = nullptr);

    void setEpsilon(double epsilon);
    void setMinPoints(int minPts);
    ClusterResult fit(const QVector<QVector<double>>& data);
    QVector<QVector<int>> extractDBSCAN(double epsilonThreshold) const;
    QVector<QVector<int>> extractClusters(double xi = 0.05) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int numPoints, int numClusters);

private:
    double m_epsilon = 1.0;
    int m_minPts = 5;
    ClusterResult m_result;
    QVector<QVector<double>> m_data;

    double distance(const QVector<double>& a, const QVector<double>& b) const;
    QVector<QPair<int,double>> getNeighbors(int pointIdx) const;
    double coreDistance(int pointIdx, const QVector<QPair<int,double>>& neighbors) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
