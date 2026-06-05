#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class SegmentTree5 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalUpdates = 0; int totalQueries = 0; double avgProcessingTimeMs = 0.0; };
    explicit SegmentTree5(QObject* parent = nullptr);
    void build(const QVector<double>& data);
    void update(int pos, double value);
    double query(int lo, int hi) const;
    double queryMin(int lo, int hi) const;
    double queryMax(int lo, int hi) const;
    int size() const { return m_n; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void updated(int pos);
private:
    int m_n = 0;
    QVector<double> m_sumTree; QVector<double> m_minTree; QVector<double> m_maxTree;
    void buildTree(QVector<double>& tree, const QVector<double>& data, int node, int lo, int hi);
    void updateTree(QVector<double>& tree, int node, int lo, int hi, int pos, double val);
    double querySum(int node, int lo, int hi, int ql, int qr) const;
    double queryMin(int node, int lo, int hi, int ql, int qr) const;
    double queryMax(int node, int lo, int hi, int ql, int qr) const;
    Stats m_stats; double m_timeSum = 0.0;
};
