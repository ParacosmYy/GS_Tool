/**
 * @file IntervalTree3.h
 * @brief Interval tree enhanced - weighted/stabbing query/max overlap
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>
class IntervalTree3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalInsertions = 0; int totalQueries = 0; double avgProcessingTimeMs = 0.0; };
    explicit IntervalTree3(QObject* parent = nullptr);
    void insert(double lo, double hi, int value);
    QList<QPair<QPair<double,double>,int>> queryPoint(double point) const;
    QList<QPair<QPair<double,double>,int>> queryRange(double lo, double hi) const;
    int maxOverlap() const;
    QPair<double,double> maxOverlapRange() const;
    void clear();
    int size() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void intervalInserted(double lo, double hi);
private:
    struct Interval { double lo, hi; int value; };
    struct Node { double center; QList<Interval> byLow, byHigh; Node *left, *right; };
    Node* build(QList<Interval>& intervals);
    void queryPoint(Node* n, double p, QList<QPair<QPair<double,double>,int>>& result) const;
    void destroyTree(Node* n);
    Node* m_root = nullptr; int m_size = 0;
    Stats m_stats; double m_timeSum = 0.0;
};
