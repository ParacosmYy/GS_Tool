#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class SpectralClustering2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalClusterings = 0; int totalPoints = 0; double avgProcessingTimeMs = 0.0; };
    explicit SpectralClustering2(QObject* parent = nullptr);
    void setNumClusters(int k);
    void setSigma(double sigma);
    QVector<int> cluster(const QVector<QVector<double>>& points);
    QVector<QVector<double>> eigenvectors() const { return m_eigvecs; }
    int numClusters() const { return m_k; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void clusteringCompleted(int clusters, double silhouette);
private:
    int m_k = 3; double m_sigma = 1.0;
    QVector<QVector<double>> m_eigvecs;
    QVector<QVector<double>> buildSimilarityMatrix(const QVector<QVector<double>>& pts);
    QVector<int> kMeansOnEmbedding(const QVector<QVector<double>>& emb, int k);
    Stats m_stats; double m_timeSum = 0.0;
};
