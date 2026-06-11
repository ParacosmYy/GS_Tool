/**
 * @file TridiagonalSolver9.h
 * @brief 三对角求解器(循环归约与并行子问题分解实现GPU友好批量三对角方程组求解) — Tridiagonal Solver with Cyclic Reduction and Parallel Subproblem Decomposition for GPU-Friendly Batch Tridiagonal System Solving
 *
 * 功能: 实现三对角求解器(Tridiagonal solver)，采用循环归约(cyclic reduction)
 *       与并行子问题分解(parallel subproblem decomposition)实现GPU友好批量三对角方程组求解(GPU-friendly batch tridiagonal system solving)。
 *
 * 协作: ThomasAlgorithm(追赶法) / LU decomposition(LU分解) / GaussSeidel(高斯-塞德尔迭代)
 */
#pragma once

#include <QObject>
#include <QVector>

class TridiagonalSolver9 : public QObject {
    Q_OBJECT

public:
    /** @brief Single tridiagonal system: lower diag, main diag, upper diag, rhs */
    struct TridiagSystem {
        QVector<double> lower;    // sub-diagonal a[1..n-1]
        QVector<double> main;     // main diagonal b[0..n-1]
        QVector<double> upper;    // super-diagonal c[0..n-2]
        QVector<double> rhs;      // right-hand side d[0..n-1]
    };

    /** @brief Batch solve result */
    struct BatchResult {
        QVector<QVector<double>> solutions;
        int batchSize = 0;
        int systemSize = 0;
        bool allConverged = true;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalSolves = 0;
        quint64 totalSystems = 0;
        int systemSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TridiagonalSolver9(QObject *parent = nullptr);
    ~TridiagonalSolver9() override;

    /** @brief Solve single tridiagonal system via cyclic reduction */
    QVector<double> solve(const TridiagSystem& system);

    /** @brief Solve batch of tridiagonal systems (parallel subproblem decomposition) */
    BatchResult solveBatch(const QVector<TridiagSystem>& systems);

    /** @brief Solve via Thomas algorithm (sequential fallback) */
    QVector<double> solveThomas(const TridiagSystem& system) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveDone(int size, double timeMs);
    void batchDone(int batchSize, int systemSize, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Cyclic reduction: forward elimination pass */
    void cyclicReduceForward(QVector<double>& a, QVector<double>& b,
                             QVector<double>& c, QVector<double>& d) const;

    /** @brief Cyclic reduction: backward substitution pass */
    void cyclicReduceBackward(const QVector<double>& a, const QVector<double>& b,
                              const QVector<double>& c, const QVector<double>& d,
                              QVector<double>& x) const;

    /** @brief Solve a single reduced subsystem */
    void solveReduced(int n, const QVector<double>& a, const QVector<double>& b,
                      const QVector<double>& c, const QVector<double>& d,
                      QVector<double>& x) const;
};
