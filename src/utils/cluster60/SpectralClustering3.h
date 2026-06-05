#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class SpectralClustering3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalClusterings = 0; int totalPoints = 0; double avgProcessingTimeMs = 0.0; };
    explicit SpectralClustering3(QObject* parent = nullptr);
    void setNumClusters(int k);
    void setKernelType(const QString& type);
    void setKernelParam(double param);
    QVector<int> cluster(const QVector<QVector<double>>& points);
    QVector<QVector<double>> embedding() const { return m_embedding; }
    double silhouette() const { return m_silhouette; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void clusteringCompleted(int k, double sil);
private:
    int m_k = 3; QString m_kernel = "rbf"; double m_param = 1.0;
    QVector<QVector<double>> m_embedding; double m_silhouette = 0.0;
    QVector<QVector<double>> buildKernelMatrix(const QVector<QVector<double>>& pts);
    QVector<int> discretize(const QVector<QVector<double>>& emb, int k);
    Stats m_stats; double m_timeSum = 0.0;
};
