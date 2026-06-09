/**
 * @file GaussSeidel6.h
 * @brief 高斯-赛德尔迭代(加权Jacobi预处理+Chebyshev加速非对称收敛) — Gauss-Seidel with Weighted Jacobi Preconditioning and Chebyshev Acceleration for Non-Symmetric Convergence
 *
 * 功能: 实现高斯-赛德尔迭代求解线性方程组(Gauss-Seidel iterative solver)，
 *       加权Jacobi预处理(weighted Jacobi preconditioning)改善条件数，
 *       Chebyshev加速(Chebyshev acceleration)优化非对称矩阵收敛。
 *
 * 协作: ConjugateGradient8(共轭梯度) / SOR5(SOR迭代) / GMRES6(GMRES)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 高斯-赛德尔迭代(加权Jacobi预处理+Chebyshev加速)
 */
class GaussSeidel6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int iterationsUsed = 0;
        double finalResidual = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GaussSeidel6(QObject *parent = nullptr);
    ~GaussSeidel6() override;

    /** @brief Set max iterations */
    void setMaxIterations(int maxIter);

    /** @brief Set convergence tolerance */
    void setTolerance(double tol);

    /** @brief Set Jacobi weight for preconditioning (0 < w < 2) */
    void setJacobiWeight(double w);

    /** @brief Enable/disable Chebyshev acceleration */
    void setChebyshevAcceleration(bool enable);

    /** @brief Solve Ax = b, return solution vector x */
    QVector<double> solve(const QVector<QVector<double>>& A,
                          const QVector<double>& b);

    /** @brief Get residual history */
    QVector<double> residualHistory() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int iterations, double residual, double timeMs);

private:
    int m_maxIter = 1000;
    double m_tolerance = 1e-10;
    double m_jacobiWeight = 0.8;
    bool m_useChebyshev = true;

    QVector<double> m_residualHistory;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute L2 norm of residual r = b - Ax */
    double residualNorm(const QVector<QVector<double>>& A,
                        const QVector<double>& x,
                        const QVector<double>& b) const;

    /** @brief Estimate spectral radius for Chebyshev parameters */
    void estimateSpectralRadius(const QVector<QVector<double>>& A,
                                double& rhoMin, double& rhoMax) const;

    /** @brief One Gauss-Seidel sweep (forward) */
    void gsSweep(const QVector<QVector<double>>& A,
                 const QVector<double>& b,
                 QVector<double>& x) const;

    /** @brief Weighted Jacobi preconditioning step */
    void jacobiPrecond(const QVector<QVector<double>>& A,
                       const QVector<double>& b,
                       QVector<double>& x) const;

    /** @brief Apply Chebyshev acceleration to update */
    void chebyshevAccelerate(QVector<double>& x,
                             const QVector<double>& xPrev,
                             int iter, double rhoMin, double rhoMax,
                             double& delta, double& deltaPrev) const;
};
