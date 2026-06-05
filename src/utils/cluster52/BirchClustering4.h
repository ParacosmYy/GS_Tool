#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class BirchClustering4 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalInsertions = 0; int totalPoints = 0; double avgProcessingTimeMs = 0.0; };
    explicit BirchClustering4(QObject* parent = nullptr);
    void setThreshold(double t);
    void setBranching(int b);
    void insert(const QVector<double>& point);
    QVector<QVector<int>> cluster(int k);
    int leafCount() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void clusteringCompleted(int k);
private:
    double m_threshold = 0.5; int m_branch = 50;
    struct CF { int n = 0; QVector<double> ls; QVector<double> ss; };
    QList<CF> m_leaves; QVector<QVector<double>> m_points;
    Stats m_stats; double m_timeSum = 0.0;
};
