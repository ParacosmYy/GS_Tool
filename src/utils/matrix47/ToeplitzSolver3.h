/**
 * @file ToeplitzSolver3.h
 * @brief Toeplitz求解器3 — 嵌套分裂+预处理CG
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class ToeplitzSolver3 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalSolves = 0;
        int totalSystemsSize = 0;
        int totalPreconditionerApplies = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ToeplitzSolver3(QObject* parent = nullptr);

    void setMatrix(const QVector<double>& firstRow);
    QVector<double> solve(const QVector<double>& rhs);
    QVector<double> solvePCG(const QVector<double>& rhs,
                              int maxIter = 100, double tol = 1e-8);
    QVector<double> circulantPreconditioner() const;

    int size() const { return m_n; }
    bool isPositiveDefinite() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int n, int iterations);

private:
    int m_n = 0;
    QVector<double> m_firstRow;
    QVector<double> m_eigenvalues;
    bool m_precomputed = false;

    void computeEigenvalues();
    QVector<double> applyPreconditioner(const QVector<double>& r) const;
    QVector<double> circulantMultiply(const QVector<double>& x) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
