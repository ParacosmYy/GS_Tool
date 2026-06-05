/**
 * @file DBSCAN6.h
 * @brief DBSCAN6 — 并行密度聚类+网格加速
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

class DBSCAN6 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalClusterings = 0;
        int totalPointsProcessed = 0;
        int totalNeighborhoodQueries = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DBSCAN6(QObject* parent = nullptr);

    void setEpsilon(double epsilon);
    void setMinPoints(int minPts);
    void setGridAcceleration(bool enable);
    QVector<int> fit(const QVector<QVector<double>>& data);
    int numClusters() const { return m_numClusters; }
    int numNoisePoints() const { return m_numNoise; }

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int points, int clusters);

private:
    double m_epsilon = 0.5;
    int m_minPts = 5;
    bool m_gridEnabled = true;
    int m_numClusters = 0;
    int m_numNoise = 0;
    double m_cellSize = 0.0;

    QVector<int> m_labels;
    QVector<QVector<double>> m_data;

    QVector<int> gridNeighbors(int pointIdx) const;
    QVector<int> bruteNeighbors(int pointIdx) const;
    void buildGrid();
    int gridCell(const QVector<double>& point) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
