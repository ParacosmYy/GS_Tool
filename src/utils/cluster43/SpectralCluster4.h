/**
 * @file SpectralCluster4.h
 * @brief 谱聚类4 — 多流形+自适应核宽
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

class SpectralCluster4 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalClusterings = 0;
        int totalPointsProcessed = 0;
        int totalEigenDecomps = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SpectralCluster4(QObject* parent = nullptr);

    void setNumClusters(int k);
    void setAdaptiveBandwidth(bool adaptive);
    void setManifoldCount(int manifolds);
    QVector<int> fit(const QVector<QVector<double>>& data);
    QVector<QVector<double>> affinityMatrix() const { return m_affinity; }

    int numClusters() const { return m_k; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int k, int points);

private:
    int m_k = 3;
    bool m_adaptive = true;
    int m_manifolds = 1;
    QVector<QVector<double>> m_affinity;

    QVector<double> adaptiveSigma(const QVector<QVector<double>>& data,
                                   int pointIdx, int knn) const;
    void buildAdaptiveAffinity(const QVector<QVector<double>>& data);

    Stats m_stats;
    double m_timeSum = 0.0;
};
