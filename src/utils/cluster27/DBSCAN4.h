/**
 * @file DBSCAN4.h
 * @brief 密度聚类增强 — 自适应epsilon/HDBSCAN*层次/稳定簇提取
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QPair>
class DBSCAN4 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalClusterings = 0;
        int totalPointsProcessed = 0;
        double avgProcessingTimeMs = 0.0;
    };
    explicit DBSCAN4(QObject* parent = nullptr);
    void setMinPoints(int minPts);
    void setAutoEpsilon(bool enable);
    QVector<int> fit(const QVector<QVector<double>>& data);
    QVector<QPair<double,int>> computeCoreDistances(const QVector<QVector<double>>& data);
    QVector<int> extractStableClusters(const QVector<QPair<double,int>>& condensedTree,
                                       double minClusterSize);
    int clusterCount() const;
    QVector<double> clusterSizes() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void clusteringComplete(int numClusters, int noisePoints);
private:
    double euclideanDist(const QVector<double>& a, const QVector<double>& b) const;
    double estimateEpsilon(const QVector<QVector<double>>& data) const;
    QVector<int> regionQuery(const QVector<QVector<double>>& data, int idx, double eps) const;
    int m_minPts = 5;
    bool m_autoEpsilon = true;
    double m_epsilon = 0.0;
    QVector<int> m_labels;
    Stats m_stats;
    double m_timeSum = 0.0;
};
