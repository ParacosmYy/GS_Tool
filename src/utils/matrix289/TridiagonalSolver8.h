/**
 * @file TridiagonalSolver8.h
 * @brief 三对角求解器(循环约化与并行前向消元) — Tridiagonal Solver with Cyclic Reduction and Parallel Forward Elimination for GPU-friendly Tridiagonal System Solving
 *
 * 功能: 实现三对角求解器(tridiagonal solver)，采用循环约化(cyclic reduction)
 *       与并行前向消元(parallel forward elimination)实现GPU友好三对角系统求解(GPU-friendly tridiagonal system solving)。
 *
 * 协作: LU10(LU分解) / Cholesky8(Cholesky分解) / SORSolver9(SOR迭代)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 三对角求解器(循环约化与并行前向消元)
 */
class TridiagonalSolver8 : public QObject {
    Q_OBJECT

public:
    /** @brief Tridiagonal system data */
    struct TridiagSystem {
        QVector<double> lower;     // Sub-diagonal a[1..n-1]
        QVector<double> main;      // Main diagonal b[0..n-1]
        QVector<double> upper;     // Super-diagonal c[0..n-2]
        QVector<double> rhs;       // Right-hand side d[0..n-1]
    };

    /** @brief Solver result */
    struct SolveResult {
        QVector<double> solution;
        double residual = 0.0;
        int size = 0;
        bool converged = true;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int lastSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TridiagonalSolver8(QObject *parent = nullptr);
    ~TridiagonalSolver8() override;

    /** @brief Solve tridiagonal system via cyclic reduction */
    SolveResult solve(const TridiagSystem& sys);

    /** @brief Solve via Thomas algorithm (sequential) */
    SolveResult solveThomas(const TridiagSystem& sys);

    /** @brief Solve multiple independent systems (batch) */
    QVector<SolveResult> solveBatch(const QVector<TridiagSystem>& systems);

    /** @brief Compute residual ||Ax - b|| */
    double computeResidual(const TridiagSystem& sys,
                           const QVector<double>& x) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveDone(int n, double residual, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Cyclic reduction: forward reduction phase */
    void crForwardReduce(QVector<double>& a, QVector<double>& b,
                         QVector<double>& c, QVector<double>& d,
                         int n) const;

    /** @brief Cyclic reduction: backward substitution phase */
    void crBackSubstitute(const QVector<double>& a, const QVector<double>& b,
                          const QVector<double>& c, const QVector<double>& d,
                          QVector<double>& x, int n) const;

    /** @brief Pad system to power of 2 for cyclic reduction */
    int padToPow2(TridiagSystem& sys) const;
};
