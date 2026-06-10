/**
 * @file IterativeRefinement7.h
 * @brief 迭代精化(缺陷修正与定点精度累积的增强数值稳定性) — Iterative Refinement with Defect Correction and Fixed-precision Accumulation for Enhanced Numerical Stability
 *
 * 功能: 实现迭代精化(iterative refinement)，采用缺陷修正(defect correction)
 *       与定点精度累积(fixed-precision accumulation)实现增强数值稳定性(enhanced numerical stability)。
 *
 * 协作: GaussElim6(高斯消元) / LUDecomposition5(LU分解) / MatrixSolver4(矩阵求解器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 迭代精化(缺陷修正与定点精度累积)
 */
class IterativeRefinement7 : public QObject {
    Q_OBJECT

public:
    /** @brief Refinement result */
    struct RefinementResult {
        QVector<double> solution;
        double residualNorm = 0.0;
        int iterationsUsed = 0;
        bool converged = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int totalIterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit IterativeRefinement7(QObject *parent = nullptr);
    ~IterativeRefinement7() override;

    /** @brief Set maximum refinement iterations */
    void setMaxIterations(int maxIter);

    /** @brief Set convergence tolerance */
    void setTolerance(double tol);

    /** @brief Solve Ax=b with iterative refinement using defect correction */
    RefinementResult solve(const QVector<QVector<double>>& A,
                            const QVector<double>& b);

    /** @brief Compute residual r = b - A*x */
    QVector<double> residual(const QVector<QVector<double>>& A,
                              const QVector<double>& b,
                              const QVector<double>& x) const;

    /** @brief Compute L2 norm of a vector */
    double normL2(const QVector<double>& v) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void iterationDone(int iter, double residualNorm, double timeMs);
    void solveDone(int n, int iters, double finalNorm, bool converged, double timeMs);

private:
    int m_maxIter = 20;
    double m_tolerance = 1e-10;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Solve Ax=b via Gaussian elimination (partial pivoting) */
    QVector<double> gaussSolve(QVector<QVector<double>> A,
                                QVector<double> b) const;

    /** @brief Matrix-vector multiply */
    QVector<double> matVecMul(const QVector<QVector<double>>& A,
                               const QVector<double>& x) const;

    /** @brief Accumulate correction with fixed-precision blending */
    QVector<double> accumulateCorrection(const QVector<double>& x,
                                          const QVector<double>& dx) const;
};
