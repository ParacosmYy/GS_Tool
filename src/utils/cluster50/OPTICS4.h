#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class OPTICS4 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalClusterings = 0; int totalPoints = 0; double avgProcessingTimeMs = 0.0; };
    explicit OPTICS4(QObject* parent = nullptr);
    void setEpsilon(double eps);
    void setMinPoints(int minPts);
    QVector<int> fit(const QVector<QVector<double>>& data);
    QVector<double> reachabilityPlot() const { return m_reachability; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void clusteringCompleted(int points);
private:
    double m_eps = 1.0; int m_minPts = 5;
    QVector<double> m_reachability;
    double dist(const QVector<double>& a, const QVector<double>& b) const;
    Stats m_stats; double m_timeSum = 0.0;
};
