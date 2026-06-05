#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class SparseCholesky2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalFactorizations = 0; int totalSolves = 0; double avgProcessingTimeMs = 0.0; };
    explicit SparseCholesky2(QObject* parent = nullptr);
    void setDimension(int n);
    void addEntry(int row, int col, double val);
    bool factorize();
    QVector<double> solve(const QVector<double>& b);
    double logDeterminant() const { return m_logDet; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void factorizationCompleted(int n, double logDet);
private:
    int m_n = 0; double m_logDet = 0.0;
    QVector<int> m_rowPtr; QVector<int> m_colIdx; QVector<double> m_values;
    QVector<double> m_L;
    bool isPositiveDefinite() const;
    Stats m_stats; double m_timeSum = 0.0;
};
