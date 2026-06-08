/**
 * @file IterativeRefinement2.h
 * @brief 迭代精化(混合精度残差计算+GMRES内校正扫描) — Iterative Refinement with Mixed-Precision Residual Computation and GMRES Inner Correction Sweep
 *
 * 功能: 实现迭代精化求解线性系统，支持混合精度残差计算、
 *       GMRES内校正扫描和收敛性监测。
 *
 * 协作: SOR3(逐次超松弛) / ConjugateGradient3(共轭梯度) / SparseSolver2(稀疏求解器)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 迭代精化(混合精度残差计算+GMRES内校正扫描)
 */
class IterativeRefinement2 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int refinementSteps = 0;
        double initialResidual = 0.0;
        double finalResidual = 0.0;
        int gmresIterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Convergence result */
    struct Result {
        QVector<double> solution;
        double residualNorm = 0.0;
        int iterations = 0;
        bool converged = false;
    };

    explicit IterativeRefinement2(QObject *parent = nullptr);
    ~IterativeRefinement2() override;

    /** @brief Set maximum refinement iterations */
    void setMaxIterations(int maxIter);

    /** @brief Set convergence tolerance */
    void setTolerance(double tol);

    /** @brief Set GMRES inner iteration count */
    void setGMRESIterations(int iters);

    /** @brief Solve Ax = b using iterative refinement */
    Result solve(const QVector<QVector<double>>& A,
                 const QVector<double>& b,
                 const QVector<double>& x0);

    /** @brief Compute residual r = b - Ax in double precision */
    static QVector<double> computeResidual(
        const QVector<QVector<double>>& A,
        const QVector<double>& x,
        const QVector<double>& b);

    /** @brief GMRES inner solve for correction */
    QVector<double> gmresSolve(const QVector<QVector<double>>& A,
                                const QVector<double>& rhs,
                                int maxIter) const;

    /** @brief Compute matrix-vector product */
    static QVector<double> matVec(const QVector<QVector<double>>& A,
                                  const QVector<double>& x);

    /** @brief Compute vector norm */
    static double vecNorm(const QVector<double>& v);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void refinementStep(int step, double residual, double timeMs);
    void solveCompleted(int iterations, double finalResidual, double timeMs);

private:
    int m_maxIter = 50;
    double m_tol = 1e-10;
    int m_gmresIter = 10;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Back substitution for upper triangular solve */
    static QVector<double> backSolve(const QVector<QVector<double>>& U,
                                      const QVector<double>& b);

    /** @brief Arnoldi process for GMRES */
    void arnoldi(const QVector<QVector<double>>& A,
                 QVector<QVector<double>>& Q,
                 QVector<QVector<double>>& H,
                 int step) const;
};
