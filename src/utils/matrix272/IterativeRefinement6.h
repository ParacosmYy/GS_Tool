/**
 * @file IterativeRefinement6.h
 * @brief 迭代精化(混合精度残差计算与条件数估计求解精度提升) — Iterative Refinement with Mixed-Precision Residual Computation and Condition Number Estimation for Solution Accuracy Enhancement
 *
 * 功能: 实现迭代精化(Iterative refinement)，采用混合精度残差计算(mixed-precision residual
 *       computation)与条件数估计(condition number estimation)提升求解精度(solution accuracy)。
 *
 * 协作: GaussElimination8(高斯消元) / LUDecomposition9(LU分解) / SVD11(奇异值分解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 迭代精化(混合精度残差计算与条件数估计求解精度提升)
 */
class IterativeRefinement6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int numIterations = 0;
        int maxIterationsReached = 0;
        double finalResidual = 0.0;
        double conditionEstimate = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit IterativeRefinement6(QObject *parent = nullptr);
    ~IterativeRefinement6() override;

    /** @brief Set maximum refinement iterations */
    void setMaxIterations(int maxIter);

    /** @brief Set convergence tolerance */
    void setTolerance(double tol);

    /** @brief Solve Ax=b with iterative refinement, returns x */
    QVector<double> solve(const QVector<QVector<double>>& A,
                          const QVector<double>& b);

    /** @brief Estimate condition number of matrix A (1-norm) */
    double conditionNumber(const QVector<QVector<double>>& A) const;

    /** @brief Compute residual ||Ax - b||_inf */
    double residualNorm(const QVector<QVector<double>>& A,
                        const QVector<double>& x,
                        const QVector<double>& b) const;

    /** @brief Get convergence history (residual per iteration) */
    QVector<double> convergenceHistory() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void refinementStep(int iteration, double residual, double timeMs);

private:
    int m_maxIter = 20;
    double m_tolerance = 1e-12;

    QVector<double> m_residualHistory;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief LU decomposition (Doolittle, in-place) */
    bool luDecompose(QVector<QVector<double>>& A,
                     QVector<int>& pivot) const;

    /** @brief Solve LU x = b using forward/back substitution */
    QVector<double> luSolve(const QVector<QVector<double>>& LU,
                            const QVector<int>& pivot,
                            const QVector<double>& b) const;

    /** @brief Compute matrix-vector product */
    static QVector<double> matVec(const QVector<QVector<double>>& A,
                                  const QVector<double>& x);

    /** @brief Compute 1-norm of matrix */
    static double matNorm1(const QVector<QVector<double>>& A);

    /** @brief Solve A^T x = b for condition estimation */
    QVector<double> solveTranspose(
        const QVector<QVector<double>>& LU,
        const QVector<int>& pivot,
        const QVector<double>& b) const;
};
