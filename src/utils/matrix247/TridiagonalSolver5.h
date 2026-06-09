/**
 * @file TridiagonalSolver5.h
 * @brief 三对角求解器(循环约化+并行前缀和奇偶消元) — Tridiagonal Solver with Cyclic Reduction and Parallel Prefix Sum for Even-Odd Elimination
 *
 * 功能: 实现三对角线性系统(Tridiagonal linear system)求解器，采用循环约化
 *       (cyclic reduction)将系统递归分解为奇偶子系统，结合并行前缀和(parallel
 *       prefix sum)实现高效偶奇消元(even-odd elimination)回代。
 *
 * 协作: Tridiagonal4(Thomas算法) / SOR5(逐次超松弛) / GaussianElim3(高斯消元)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 三对角求解器(循环约化+并行前缀和奇偶消元)
 */
class TridiagonalSolver5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int systemSize = 0;
        int reductionLevels = 0;
        double residualNorm = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TridiagonalSolver5(QObject *parent = nullptr);
    ~TridiagonalSolver5() override;

    /** @brief Solve tridiagonal system Ax = d where A has sub/main/super diagonals */
    QVector<double> solve(const QVector<double>& lower,
                          const QVector<double>& main,
                          const QVector<double>& upper,
                          const QVector<double>& rhs);

    /** @brief Solve multiple independent systems (batch mode) */
    QVector<QVector<double>> solveBatch(const QVector<QVector<double>>& lowers,
                                         const QVector<QVector<double>>& mains,
                                         const QVector<QVector<double>>& uppers,
                                         const QVector<QVector<double>>& rhss);

    /** @brief Compute residual ||Ax - d|| for verification */
    double residual(const QVector<double>& lower,
                    const QVector<double>& main,
                    const QVector<double>& upper,
                    const QVector<double>& rhs,
                    const QVector<double>& x) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int systemSize, int levels, double residual, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Cyclic reduction forward pass (eliminate odd-indexed unknowns) */
    void reduce(QVector<double>& a, QVector<double>& b, QVector<double>& c,
                QVector<double>& d, int n) const;

    /** @brief Back-substitution after reduction */
    void backSubstitute(QVector<double>& a, QVector<double>& b, QVector<double>& c,
                        QVector<double>& d, QVector<double>& x, int n) const;

    /** @brief Pad system to next power of 2 for cyclic reduction */
    int padToPowerOf2(QVector<double>& a, QVector<double>& b,
                      QVector<double>& c, QVector<double>& d, int origN) const;
};
