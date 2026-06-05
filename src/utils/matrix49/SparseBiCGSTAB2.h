/**
 * @file SparseBiCGSTAB2.h
 * @brief 稀疏BiCGSTAB2 — 预条件+右端GMRES稳定化
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class SparseBiCGSTAB2 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalSolves = 0;
        int totalIterations = 0;
        int totalBreakdowns = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SparseBiCGSTAB2(QObject* parent = nullptr);

    void setMatrix(int n, const QVector<int>& rowPtr,
                   const QVector<int>& colIdx,
                   const QVector<double>& values);
    void setTolerance(double tol);
    void setMaxIterations(int maxIter);
    bool buildPreconditioner();
    QVector<double> solve(const QVector<double>& rhs);

    int size() const { return m_n; }
    int iterationsUsed() const { return m_iterUsed; }
    double residualNorm() const { return m_resNorm; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int iterations, double residual, bool converged);

private:
    int m_n = 0;
    double m_tol = 1e-8;
    int m_maxIter = 1000;
    QVector<int> m_rowPtr;
    QVector<int> m_colIdx;
    QVector<double> m_values;
    QVector<double> m_diagPrecond;
    bool m_precondBuilt = false;
    int m_iterUsed = 0;
    double m_resNorm = 0.0;

    QVector<double> spmv(const QVector<double>& x) const;
    QVector<double> precondSolve(const QVector<double>& r) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
