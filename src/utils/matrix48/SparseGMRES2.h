/**
 * @file SparseGMRES2.h
 * @brief 稀疏GMRES2 — 重启GMRES+预条件ILU(0)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class SparseGMRES2 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalSolves = 0;
        int totalIterations = 0;
        int totalRestarts = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SparseGMRES2(QObject* parent = nullptr);

    void setMatrix(int n, const QVector<int>& rowPtr,
                   const QVector<int>& colIdx,
                   const QVector<double>& values);
    void setRestart(int m);
    void setTolerance(double tol);
    void setMaxIterations(int maxIter);
    bool buildPreconditioner();
    QVector<double> solve(const QVector<double>& rhs);

    int size() const { return m_n; }
    int iterationsUsed() const { return m_iterationsUsed; }
    double residualNorm() const { return m_residualNorm; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int iterations, double residual);

private:
    int m_n = 0;
    int m_restart = 30;
    double m_tol = 1e-8;
    int m_maxIter = 1000;
    QVector<int> m_rowPtr;
    QVector<int> m_colIdx;
    QVector<double> m_values;
    QVector<int> m_iluRowPtr;
    QVector<int> m_iluColIdx;
    QVector<double> m_iluValues;
    bool m_precondBuilt = false;
    int m_iterationsUsed = 0;
    double m_residualNorm = 0.0;

    QVector<double> sparseMultiply(const QVector<double>& x) const;
    QVector<double> iluSolve(const QVector<double>& r) const;
    void buildILU0();

    Stats m_stats;
    double m_timeSum = 0.0;
};
