#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class OPTICS5 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalClusterings = 0; int totalPoints = 0; double avgProcessingTimeMs = 0.0; };
    explicit OPTICS5(QObject* parent = nullptr);
    void setEpsilon(double eps);
    void setMinPoints(int minPts);
    void setExtractMethod(const QString& method);
    QVector<int> cluster(const QVector<QVector<double>>& points);
    QVector<double> reachabilityPlot() const { return m_reachability; }
    QVector<int> ordering() const { return m_ordering; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void clusteringCompleted(int clusters, int noise);
private:
    double m_eps = 0.5; int m_minPts = 5; QString m_method = "xi";
    QVector<double> m_reachability; QVector<int> m_ordering;
    double coreDist(const QVector<QVector<double>>& pts, int idx);
    QVector<int> getNeighbors(const QVector<QVector<double>>& pts, int idx);
    Stats m_stats; double m_timeSum = 0.0;
};
