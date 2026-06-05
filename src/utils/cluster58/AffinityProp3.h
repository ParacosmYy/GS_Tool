#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class AffinityProp3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalClusterings = 0; int totalPoints = 0; double avgProcessingTimeMs = 0.0; };
    explicit AffinityProp3(QObject* parent = nullptr);
    void setDamping(double damp);
    void setMaxIterations(int iter);
    void setConvergence(int convIter);
    QVector<int> cluster(const QVector<QVector<double>>& similarities);
    QVector<int> exemplarIndices() const { return m_exemplars; }
    int numClusters() const { return m_exemplars.size(); }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void clusteringCompleted(int clusters, int iterations);
private:
    double m_damping = 0.5; int m_maxIter = 200; int m_convIter = 10;
    QVector<int> m_exemplars;
    Stats m_stats; double m_timeSum = 0.0;
};
