/**
 * @file TridiagonalSolver2.h
 * @brief 三对角矩阵求解器(循环约化并行求解+条件数估计) — Tridiagonal Solver with Cyclic Reduction for Parallel-Suitable Solution and Condition Number Estimate
 *
 * 功能: 实现三对角线性方程组求解器，支持循环约化(Cyclic Reduction)并行化算法、
 *       Thomas算法串行求解、条件数估计和多右端向量批量求解。
 *
 * 协作: SparseSolver6(稀疏矩阵) / EigenSolver4(特征值) / MatrixInverse3(矩阵求逆)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 三对角矩阵求解器(循环约化+条件数估计)
 */
class TridiagonalSolver2 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSolves = 0;
        int systemSize = 0;
        double conditionEstimate = 0.0;
        double residualNorm = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TridiagonalSolver2(QObject *parent = nullptr);
    ~TridiagonalSolver2() override;

    /** @brief Set tridiagonal system: lower diag a, main diag b, upper diag c */
    void setSystem(const QVector<double>& a, const QVector<double>& b,
                   const QVector<double>& c);

    /** @brief Solve using Thomas algorithm (serial, O(n)) */
    QVector<double> solveThomas(const QVector<double>& rhs);

    /** @brief Solve using cyclic reduction (parallel-suitable) */
    QVector<double> solveCyclicReduction(const QVector<double>& rhs);

    /** @brief Solve multiple right-hand sides */
    QVector<QVector<double>> solveBatch(const QVector<QVector<double>>& rhsList);

    /** @brief Estimate condition number of tridiagonal system */
    double estimateConditionNumber() const;

    /** @brief Compute residual ||Ax - b|| */
    double computeResidual(const QVector<double>& x,
                           const QVector<double>& rhs) const;

    int size() const { return m_n; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int n, double residual, double timeMs);

private:
    int m_n = 0;
    QVector<double> m_a; // Lower diagonal (size n-1)
    QVector<double> m_b; // Main diagonal (size n)
    QVector<double> m_c; // Upper diagonal (size n-1)

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Validate system dimensions */
    bool validateSystem() const;

    /** @brief Compute 1-norm of the tridiagonal matrix */
    double matrixNorm1() const;

    /** @brief Simple LU inverse norm estimate via power iteration */
    double inverseNormEstimate() const;
};
