#pragma once
#include <QObject>
#include <QVector>
#include <QPair>
class DBSCAN5 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalClusterings = 0; int totalPointsProcessed = 0; double avgProcessingTimeMs = 0.0; };
    explicit DBSCAN5(QObject* parent = nullptr);
    void setMinPoints(int pts); void setEpsilon(double eps);
    void setVariableDensity(bool enable);
    QVector<int> fit(const QVector<QVector<double>>& data);
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void clusteringComplete(int clusters, int noise);
private:
    int m_minPts = 5; double m_eps = 0.5; bool m_varDensity = false;
    Stats m_stats; double m_timeSum = 0.0;
};
