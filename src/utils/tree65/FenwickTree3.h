#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class FenwickTree3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalUpdates = 0; int totalQueries = 0; double avgProcessingTimeMs = 0.0; };
    explicit FenwickTree3(QObject* parent = nullptr);
    void build(const QVector<double>& data);
    void update(int idx, double delta);
    double prefixSum(int idx) const;
    double rangeSum(int lo, int hi) const;
    int lowerBound(double target) const;
    int size() const { return m_n; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void updated(int idx, double delta);
private:
    int m_n = 0;
    QVector<double> m_tree;
    int lsb(int x) const { return x & (-x); }
    Stats m_stats; double m_timeSum = 0.0;
};
