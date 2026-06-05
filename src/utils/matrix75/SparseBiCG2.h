#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class SparseBiCG2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSolves = 0; int totalIterations = 0; double avgProcessingTimeMs = 0.0; };
    explicit SparseBiCG2(QObject* parent = nullptr);
    void setDimension(int n);
    void addEntry(int row, int col, double val);
    void setMaxIterations(int iter);
    void setTolerance(double tol);
    QVector<double> solve(const QVector<double>& b);
    int iterationsUsed() const { return m_iterUsed; }
    double finalResidual() const { return m_residual; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void solveCompleted(int iterations, double residual);
private:
    int m_n = 0; int m_maxIter = 1000; double m_tol = 1e-8;
    int m_iterUsed = 0; double m_residual = 0.0;
    QVector<int> m_rowPtr; QVector<int> m_colIdx; QVector<double> m_values;
    QVector<double> spMV(const QVector<double>& x) const;
    QVector<double> spMVT(const QVector<double>& x) const;
    Stats m_stats; double m_timeSum = 0.0;
};
