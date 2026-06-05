#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class SegmentTree6 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalUpdates = 0; int totalQueries = 0; double avgProcessingTimeMs = 0.0; };
    explicit SegmentTree6(QObject* parent = nullptr);
    void build(const QVector<double>& data);
    void updateRange(int lo, int hi, double delta);
    double querySum(int lo, int hi) const;
    double queryMax(int lo, int hi) const;
    int size() const { return m_n; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void rangeUpdated(int lo, int hi);
private:
    int m_n = 0;
    QVector<double> m_sumTree; QVector<double> m_maxTree; QVector<double> m_lazy;
    void pushDown(int node, int lo, int hi);
    void updateRange(int node, int lo, int hi, int ql, int qr, double delta);
    double querySum(int node, int lo, int hi, int ql, int qr) const;
    double queryMax(int node, int lo, int hi, int ql, int qr) const;
    Stats m_stats; double m_timeSum = 0.0;
};
