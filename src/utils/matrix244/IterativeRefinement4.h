/**
 * @file IterativeRefinement4.h
 * @brief 迭代精化(混合精度残差计算+条件数感知收敛) — Iterative Refinement with Mixed-Precision Residual Computation and Condition-Number Aware Convergence
 *
 * 功能: 实现迭代精化(iterative refinement)，采用混合精度残差计算(mixed-precision residual
 *       computation)在低精度求解后用高精度计算残差，通过条件数感知收敛(condition-number aware
 *       convergence)根据矩阵条件数动态调整迭代策略，实现高精度线性方程组求解。
 *
 * 协作: SVD15(奇异值分解) / Cholesky12(Cholesky分解) / EigenSolver18(特征值求解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 迭代精化(混合精度残差计算+条件数感知收敛)
 */
class IterativeRefinement4 : public QObject {
    Q_OBJECT

public:
    /** @brief Solver result */
    struct SolveResult {
        QVector<double> solution;
        double residualNorm = 0.0;
        int iterations = 0;
        double condEstimate = 0.0;
        bool converged = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int numSolves = 0;
        int totalIterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit IterativeRefinement4(QObject *parent = nullptr);
    ~IterativeRefinement4() override;

    /** @brief Set maximum refinement iterations */
    void setMaxIterations(int maxIter);

    /** @brief Set convergence tolerance */
    void setTolerance(double tol);

    /** @brief Solve Ax = b with iterative refinement */
    SolveResult solve(const QVector<QVector<double>>& A,
                      const QVector<double>& b);

    /** @brief Estimate condition number of A (1-norm) */
    double estimateConditionNumber(const QVector<QVector<double>>& A) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void iterationCompleted(int iter, double residual, double correction);
    void solveCompleted(int iters, double condEst, double timeMs);

private:
    int m_maxIter = 50;
    double m_tol = 1e-12;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief LU decomposition with partial pivoting (double precision) */
    bool luDecompose(QVector<QVector<double>>& A, QVector<int>& pivot) const;

    /** @brief Solve LUx = b via forward/back substitution */
    QVector<double> luSolve(const QVector<QVector<double>>& LU,
                            const QVector<int>& pivot,
                            const QVector<double>& b) const;

    /** @brief Compute residual r = b - Ax in high precision */
    QVector<double> computeResidual(const QVector<QVector<double>>& A,
                                    const QVector<double>& x,
                                    const QVector<double>& b) const;

    /** @brief Matrix-vector product */
    QVector<double> matVec(const QVector<QVector<double>>& A,
                           const QVector<double>& x) const;

    /** @brief Vector norm (2-norm) */
    static double vecNorm(const QVector<double>& v);
};
