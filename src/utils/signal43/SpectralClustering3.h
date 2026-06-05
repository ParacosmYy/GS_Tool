/**
 * @file SpectralClustering3.h
 * @brief 谱聚类3 — 归一化割+Krylov加速
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

class SpectralClustering3 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalClusterings = 0;
        int totalPointsProcessed = 0;
        int totalEigenComputations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SpectralClustering3(QObject* parent = nullptr);

    void setNumClusters(int k);
    void setSigma(double sigma);
    void setKNN(int knn);
    void setMaxIterations(int maxIter);
    QVector<int> fit(const QVector<QVector<double>>& data);
    QVector<double> eigenvalues() const { return m_eigenvalues; }

    int numClusters() const { return m_k; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int k, int iterations);

private:
    int m_k = 3;
    double m_sigma = 1.0;
    int m_knn = 10;
    int m_maxIterations = 100;
    QVector<double> m_eigenvalues;

    QVector<double> computeLaplacian(const QVector<QVector<double>>& aff,
                                      int n) const;
    QVector<double> lanczosEigen(const QVector<double>& mat, int n,
                                  int numEigen) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
