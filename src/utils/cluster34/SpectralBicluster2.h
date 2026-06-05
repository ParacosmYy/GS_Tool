#pragma once
#include <QObject>
#include <QVector>
#include <QPair>
class SpectralBicluster2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalClusterings = 0; int totalElementsProcessed = 0; double avgProcessingTimeMs = 0.0; };
    explicit SpectralBicluster2(QObject* parent = nullptr);
    void setNumRowClusters(int k); void setNumColClusters(int l);
    QPair<QVector<int>,QVector<int>> fit(const QVector<QVector<double>>& matrix);
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void clusteringComplete(int rowClusters, int colClusters);
private:
    int m_k = 2, m_l = 2;
    Stats m_stats; double m_timeSum = 0.0;
};
