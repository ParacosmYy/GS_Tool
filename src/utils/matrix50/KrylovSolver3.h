/**
 * @file KrylovSolver3.h
 * @brief Krylov求解器3 — MINRES+对称不定系统
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class KrylovSolver3 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalSolves = 0;
        int totalIterations = 0;
        int matrixSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit KrylovSolver3(QObject* parent = nullptr);

    void setMatrix(int n, const QVector<int>& rowPtr,
                   const QVector<int>& colIdx,
                   const QVector<double>& values);
    void setTolerance(double tol);
    void setMaxIterations(int maxIter);
    QVector<double> solveCG(const QVector<double>& rhs);
    QVector<double> solveMINRES(const QVector<double>& rhs);
    QVector<double> solveBiCGSTAB(const QVector<double>& rhs);

    int iterationsUsed() const { return m_iterUsed; }
    double residualNorm() const { return m_resNorm; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(const QString& method, int iterations, double residual);

private:
    int m_n = 0;
    double m_tol = 1e-8;
    int m_maxIter = 1000;
    int m_iterUsed = 0;
    double m_resNorm = 0.0;
    QVector<int> m_rowPtr;
    QVector<int> m_colIdx;
    QVector<double> m_values;

    QVector<double> spmv(const QVector<double>& x) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
