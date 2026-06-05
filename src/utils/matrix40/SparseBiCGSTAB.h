#pragma once
#include <QObject>
#include <QVector>
class SparseBiCGSTAB : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSolves = 0; int totalIterations = 0; double avgProcessingTimeMs = 0.0; };
    explicit SparseBiCGSTAB(QObject* parent = nullptr);
    void buildFromCOO(const QVector<int>& rows, const QVector<int>& cols, const QVector<double>& vals, int n);
    void setTolerance(double tol); void setMaxIterations(int maxIter);
    QVector<double> solve(const QVector<double>& rhs);
    int lastIterations() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void solveComplete(bool converged, int iterations);
private:
    int m_n = 0; double m_tol = 1e-8; int m_maxIter = 1000; int m_lastIter = 0;
    QVector<int> m_rowPtr, m_colIdx; QVector<double> m_values;
    Stats m_stats; double m_timeSum = 0.0;
};
