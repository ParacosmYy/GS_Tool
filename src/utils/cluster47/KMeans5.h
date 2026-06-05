#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class KMeans5 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalFits = 0; int totalPredictions = 0; double avgProcessingTimeMs = 0.0; };
    explicit KMeans5(QObject* parent = nullptr);
    void setComponents(int k);
    void setMaxIterations(int maxIter);
    void setInitialization(const QString& method);
    QVector<int> fit(const QVector<QVector<double>>& data);
    QVector<int> predict(const QVector<QVector<double>>& data) const;
    QVector<QVector<double>> centroids() const { return m_centroids; }
    double inertia() const { return m_inertia; }
    int components() const { return m_k; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void fitCompleted(int k, int iterations, double inertia);
private:
    int m_k = 3; int m_maxIter = 300; QString m_init = "kmeans++";
    QVector<QVector<double>> m_centroids; double m_inertia = 0.0;
    void initRandom(const QVector<QVector<double>>& data);
    void initKMeansPlusPlus(const QVector<QVector<double>>& data);
    double distance(const QVector<double>& a, const QVector<double>& b) const;
    Stats m_stats; double m_timeSum = 0.0;
};
