#pragma once
#include <QObject>
#include <QVector>
class FenwickTree2D2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalUpdates = 0; int totalQueries = 0; double avgProcessingTimeMs = 0.0; };
    explicit FenwickTree2D2(int rows = 0, int cols = 0, QObject* parent = nullptr);
    void resize(int rows, int cols);
    void update(int r, int c, double delta);
    double query(int r, int c) const;
    double queryRange(int r1, int c1, int r2, int c2) const;
    void clear();
    int rows() const; int cols() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void updated(int r, int c, double delta);
private:
    int m_rows = 0, m_cols = 0;
    QVector<QVector<double>> m_tree;
    Stats m_stats; double m_timeSum = 0.0;
};
