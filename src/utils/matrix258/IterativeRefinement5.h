/**
 * @file IterativeRefinement5.h
 * @brief 迭代精化(混合精度累加+残差校正近机器精度收敛) — Iterative Refinement with Mixed-Precision Accumulate and Residual Correction with Near-Machine-Precision Convergence
 *
 * 功能: 实现迭代精化(iterative refinement)求解线性方程组，使用混合精度
 *       累积(mixed-precision accumulate)在低精度分解基础上高精度计算残差，
 *       残差校正(residual correction)迭代达到近机器精度(near-machine
 *       precision)收敛。
 *
 * 协作: GaussianElimination1(高斯消元) / LUDecomposition2(LU分解) / SVD4(奇异值分解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 迭代精化(混合精度累加+残差校正)
 */
class IterativeRefinement5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int numIterations = 0;
        int maxIterations = 50;
        double initialResidual = 0.0;
        double finalResidual = 0.0;
        double tolerance = 1e-14;
        bool converged = false;
        double avgProcessingTimeMs = 0.0;
    };

    explicit IterativeRefinement5(QObject *parent = nullptr);
    ~IterativeRefinement5() override;

    /** @brief Set convergence tolerance */
    void setTolerance(double tol);

    /** @brief Set maximum refinement iterations */
    void setMaxIterations(int maxIter);

    /** @brief Solve Ax = b with iterative refinement */
    QVector<double> solve(const QVector<QVector<double>>& A,
                          const QVector<double>& b);

    /** @brief Get the last solution's residual history */
    QVector<double> residualHistory() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int iterations, double finalResidual, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QVector<double> m_residualHistory;

    /** @brief LU decomposition (Doolittle) stored in-place */
    QVector<QVector<double>> m_lu;
    QVector<int> m_pivot;

    /** @brief Perform LU decomposition with partial pivoting */
    void decomposeLU(const QVector<QVector<double>>& A);

    /** @brief Solve LU x = b via forward/back substitution */
    QVector<double> solveLU(const QVector<double>& b) const;

    /** @brief Compute residual r = b - A*x with high precision */
    QVector<double> computeResidual(const QVector<QVector<double>>& A,
                                    const QVector<double>& b,
                                    const QVector<double>& x) const;

    /** @brief Compute infinity norm of a vector */
    static double infNorm(const QVector<double>& v);
};
