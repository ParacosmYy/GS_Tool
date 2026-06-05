#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class MeanShift3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalShifts = 0; int totalPoints = 0; double avgProcessingTimeMs = 0.0; };
    explicit MeanShift3(QObject* parent = nullptr);
    void setBandwidth(double bw);
    void setMaxIterations(int iter);
    QVector<int> cluster(const QVector<QVector<double>>& points);
    QVector<QVector<double>> modes() const { return m_modes; }
    int numClusters() const { return m_modes.size(); }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void clusteringCompleted(int clusters, int iterations);
private:
    double m_bandwidth = 1.0; int m_maxIter = 100;
    QVector<QVector<double>> m_modes;
    QVector<double> shiftPoint(const QVector<double>& pt, const QVector<QVector<double>>& all);
    double gaussianKernel(double dist) const;
    Stats m_stats; double m_timeSum = 0.0;
};
