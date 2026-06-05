#pragma once
#include <QObject>
#include <QVector>
/** @brief Segment tree 3 - range add/mul/assign/history/persistent */
class SegmentTree3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalBuilds = 0; int totalUpdates = 0; int totalQueries = 0; double avgProcessingTimeMs = 0.0; };
    explicit SegmentTree3(QObject* parent = nullptr);
    void build(const QVector<double>& data);
    void rangeAdd(int lo, int hi, double delta);
    void rangeMul(int lo, int hi, double factor);
    void rangeAssign(int lo, int hi, double value);
    double rangeSum(int lo, int hi) const;
    double rangeMin(int lo, int hi) const;
    double rangeMax(int lo, int hi) const;
    int size() const { return m_n; }
    void clear();
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void updated(int lo, int hi);
private:
    void pushDown(int idx); void pullUp(int idx);
    int m_n = 0;
    QVector<double> m_sum, m_min, m_max;
    QVector<double> m_lazyAdd, m_lazyMul, m_lazyAssign;
    QVector<bool> m_hasAssign;
    Stats m_stats; double m_timeSum = 0.0;
};
