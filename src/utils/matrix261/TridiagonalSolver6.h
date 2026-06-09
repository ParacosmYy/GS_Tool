/**
 * @file TridiagonalSolver6.h
 * @brief 三对角求解器(抛物线循环约化+向量化扫描GPU友好并行) — Tridiagonal Solver with Parabolic Cyclic Reduction and Vectorized Sweep for GPU-friendly Parallel Execution
 *
 * 功能: 实现三对角线性方程组求解器(Tridiagonal Solver)，采用抛物线循环
 *       约化(Parabolic Cyclic Reduction, PCR)算法，配合向量化扫描
 *       (vectorized sweep)实现GPU友好的并行执行。
 *
 * 协作: LUDecomposition5(LU分解) / GaussElimination4(高斯消元) / SparseSolver7(稀疏求解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 三对角求解器(抛物线循环约化+向量化扫描)
 */
class TridiagonalSolver6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int systemSize = 0;
        int numReductionStages = 0;
        double residualNorm = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Tridiagonal system coefficients */
    struct System {
        QVector<double> lower;   // Sub-diagonal a[1..n-1]
        QVector<double> main;    // Main diagonal b[0..n-1]
        QVector<double> upper;   // Super-diagonal c[0..n-2]
        QVector<double> rhs;     // Right-hand side d[0..n-1]
    };

    /** @brief PCR stage intermediate */
    struct ReductionStage {
        QVector<double> a;
        QVector<double> b;
        QVector<double> c;
        QVector<double> d;
    };

    explicit TridiagonalSolver6(QObject *parent = nullptr);
    ~TridiagonalSolver6() override;

    /** @brief Set system to solve */
    void setSystem(const System& sys);

    /** @brief Set system from raw arrays */
    void setSystem(const QVector<double>& lower,
                   const QVector<double>& main,
                   const QVector<double>& upper,
                   const QVector<double>& rhs);

    /** @brief Solve using PCR algorithm */
    QVector<double> solvePCR();

    /** @brief Solve using Thomas algorithm (sequential) */
    QVector<double> solveThomas();

    /** @brief Compute residual ||Ax - b|| */
    double residual(const QVector<double>& x) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int size, double residual, double timeMs);

private:
    int m_n = 0;
    System m_system;
    QVector<ReductionStage> m_stages;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Pad system to power of 2 for PCR */
    int padToPowerOf2();

    /** @brief Execute one PCR reduction step */
    void pcrReduceStage(int stage, int stride);

    /** @brief Back-substitute from PCR stages */
    QVector<double> pcrBackSubstitute() const;

    /** @brief Validate system dimensions */
    bool validateSystem() const;
};
