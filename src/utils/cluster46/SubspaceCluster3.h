/**
 * @file SubspaceCluster3.h
 * @brief 子空间聚类3 — PROCLUS+维选择
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

class SubspaceCluster3 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalClusterings = 0;
        int totalPointsProcessed = 0;
        int totalSubspaces = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SubspaceCluster3(QObject* parent = nullptr);

    void setNumClusters(int k);
    void setAverageDimensions(int l);
    void setMinDimensions(int minDim);
    QVector<int> fit(const QVector<QVector<double>>& data);
    QVector<QVector<int>> subspaceDimensions() const { return m_subspaces; }

    int numClusters() const { return m_k; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int k, int subspaces);

private:
    int m_k = 3;
    int m_l = 3;
    int m_minDim = 2;
    QVector<QVector<int>> m_subspaces;

    QVector<int> selectMedoids(const QVector<QVector<double>>& data) const;
    QVector<int> findRelevantDims(const QVector<QVector<double>>& data,
                                   int medoid, int numDim) const;
    double computeSparsity(const QVector<QVector<double>>& data,
                            int point, int dim) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
