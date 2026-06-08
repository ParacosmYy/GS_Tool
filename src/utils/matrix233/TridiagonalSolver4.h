/**
 * @file TridiagonalSolver4.h
 * @brief 三对角求解器(Thomas算法+部分选主元近奇异数值稳定) — Tridiagonal Solver with Thomas Algorithm and Partial Pivoting for Numerical Stability in Near-singular Systems
 *
 * 功能: 实现三对角(Tridiagonal)线性方程组求解器，基于Thomas算法，
 *       引入部分选主元(partial pivoting)策略增强近奇异系统的数值稳定性。
 *
 * 协作: LU7(LU分解) / Cholesky5(Cholesky分解) / BandedSolver3(带状求解器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 三对角求解器(Thomas+部分选主元)
 */
class TridiagonalSolver4 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int systemSize = 0;
        int pivotSwaps = 0;
        double residualNorm = 0.0;
        double conditionEstimate = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TridiagonalSolver4(QObject *parent = nullptr);
    ~TridiagonalSolver4() override;

    /** @brief Solve Ax=d with sub-diag a, main diag b, super-diag c, rhs d */
    QVector<double> solve(const QVector<double>& a,
                          const QVector<double>& b,
                          const QVector<double>& c,
                          const QVector<double>& d);

    /** @brief Solve without pivoting (classic Thomas) */
    QVector<double> solveThomas(const QVector<double>& a,
                                const QVector<double>& b,
                                const QVector<double>& c,
                                const QVector<double>& d);

    /** @brief Estimate condition number of tridiagonal system */
    double estimateCondition(const QVector<double>& a,
                              const QVector<double>& b,
                              const QVector<double>& c) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int n, double residual, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute residual ||Ax - d||_2 */
    double computeResidual(const QVector<double>& a,
                           const QVector<double>& b,
                           const QVector<double>& c,
                           const QVector<double>& d,
                           const QVector<double>& x) const;
};

