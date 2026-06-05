#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class KMeans7 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalClusterings = 0; int totalPoints = 0; double avgProcessingTimeMs = 0.0; };
    explicit KMeans7(QObject* parent = nullptr);
    void setNumClusters(int k);
    void setMaxIterations(int iter);
    void setMiniBatchSize(int size);
    QVector<int> cluster(const QVector<QVector<double>>& points);
    QVector<int> predict(const QVector<QVector<double>>& points) const;
    QVector<QVector<double>> centroids() const { return m_centroids; }
    double inertia() const { return m_inertia; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void clusteringCompleted(int k, double inertia);
private:
    int m_k = 3; int m_maxIter = 300; int m_batchSize = 0;
    QVector<QVector<double>> m_centroids; double m_inertia = 0.0;
    double distance(const QVector<double>& a, const QVector<double>& b) const;
    Stats m_stats; double m_timeSum = 0.0;
};
