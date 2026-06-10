/**
 * @file GaussSeidel7.h
 * @brief 高斯-赛德尔迭代(逐次超松弛与红黑排序加速稀疏系统求解) — Gauss-Seidel with Successive Over-Relaxation and Red-Black Ordering for Accelerated Iterative Sparse System Solving
 *
 * 功能: 实现高斯-赛德尔迭代(Gauss-Seidel iteration)，采用逐次超松弛(SOR)
 *       与红黑排序(red-black ordering)加速迭代稀疏系统求解(iterative sparse solving)。
 *
 * 协作: ConjugateGradient8(共轭梯度) / SparseLU9(稀疏LU分解) / Jacobi6(Jacobi迭代)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 高斯-赛德尔迭代(逐次超松弛与红黑排序加速稀疏系统求解)
 */
class GaussSeidel7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int iterationsUsed = 0;
        double finalResidual = 0.0;
        double omega = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GaussSeidel7(QObject *parent = nullptr);
    ~GaussSeidel7() override;

    /** @brief Set relaxation factor omega (1.0 = standard GS, >1 = SOR) */
    void setOmega(double omega);

    /** @brief Set maximum iterations */
    void setMaxIterations(int maxIter);

    /** @brief Set convergence tolerance */
    void setTolerance(double tol);

    /** @brief Enable red-black ordering for parallel-friendly updates */
    void setRedBlackOrdering(bool enabled);

    /** @brief Solve Ax = b where A is stored as CSR-like triplets */
    QVector<double> solve(int n, const QVector<double>& b,
                          const QVector<int>& rowPtr,
                          const QVector<int>& colIdx,
                          const QVector<double>& values);

    /** @brief Solve dense system Ax = b */
    QVector<double> solveDense(const QVector<QVector<double>>& A,
                               const QVector<double>& b);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int iterations, double residual, double timeMs);

private:
    double m_omega = 1.5;
    int m_maxIter = 1000;
    double m_tol = 1e-10;
    bool m_redBlack = true;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute residual norm ||Ax - b|| */
    double residualNorm(int n, const QVector<double>& x,
                        const QVector<double>& b,
                        const QVector<int>& rowPtr,
                        const QVector<int>& colIdx,
                        const QVector<double>& values) const;

    /** @brief Build red-black coloring for 1D/2D grid */
    QVector<int> buildColoring(int n) const;
};
