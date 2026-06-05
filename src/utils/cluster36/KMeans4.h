#pragma once
#include <QObject>
#include <QVector>
#include <QPair>
class KMeans4 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalFits = 0; int totalPointsProcessed = 0; double avgProcessingTimeMs = 0.0; };
    explicit KMeans4(QObject* parent = nullptr);
    void setK(int k); void setMaxIterations(int maxIter);
    void setInitMethod(int method);
    QVector<int> fit(const QVector<QVector<double>>& data);
    QVector<QVector<double>> centroids() const;
    double inertia() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void fitComplete(int k, double inertia);
private:
    int m_k = 3; int m_maxIter = 300; int m_initMethod = 0;
    QVector<QVector<double>> m_centroids; double m_inertia = 0.0;
    Stats m_stats; double m_timeSum = 0.0;
};
