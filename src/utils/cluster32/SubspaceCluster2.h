/**
 * @file SubspaceCluster2.h
 * @brief 子空间聚类增强 — CLIQUE/MAFIA/自适应网格/子空间搜索
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>
class SubspaceCluster2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalClusterings = 0; int totalPointsProcessed = 0; double avgProcessingTimeMs = 0.0; };
    explicit SubspaceCluster2(QObject* parent = nullptr);
    void setGridResolution(int bins);
    void setDensityThreshold(double threshold);
    void setMinDimensions(int minDims);
    QVector<int> fit(const QVector<QVector<double>>& data);
    QList<QVector<int>> findSubspaces(const QVector<QVector<double>>& data);
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void clusteringComplete(int numClusters, int numSubspaces);
private:
    int m_bins = 10; double m_threshold = 0.2; int m_minDims = 2;
    Stats m_stats; double m_timeSum = 0.0;
};
