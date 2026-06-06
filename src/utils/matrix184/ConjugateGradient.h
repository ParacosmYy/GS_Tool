/**
 * @file ConjugateGradient.h
 * @brief 共轭梯度法(Jacobi预条件+残差监控+收敛控制) — Conjugate Gradient Solver with Jacobi Preconditioner, Residual Monitoring and Convergence Control
 *
 * 功能: 实现共轭梯度法求解稀疏对称正定线性方程组，支持Jacobi对角预条件、
 *       残差范数监控、收敛判据和迭代历史记录。
 *
 * 协作: LUDecomposition5(LU分解) / GaussSeidel5(Gauss-Seidel) / SparseMatrix5(稀疏矩阵)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 共轭梯度求解器(Jacobi预条件+残差监控)
 */
class ConjugateGradient : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSolves = 0;
        int matrixSize = 0;
        int iterationsUsed = 0;
        double initialResidual = 0.0;
        double finalResidual = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief 求解结果 */
    struct SolveResult {
        QVector<double> solution;
        int iterations = 0;
        double residualNorm = 0.0;
        bool converged = false;
        QVector<double> residualHistory;
    };

    explicit ConjugateGradient(QObject *parent = nullptr);
    ~ConjugateGradient() override;

    void setMaxIterations(int iter);
    void setTolerance(double tol);
    void setPreconditioner(bool enabled);

    /** @brief 求解 Ax = b (A symmetric positive definite) */
    SolveResult solve(const QVector<QVector<double>>& A,
                      const QVector<double>& b);

    /** @brief 求解稀疏格式: A stored as row-based non-zero entries */
    SolveResult solveSparse(const QVector<QVector<int>>& colIdx,
                             const QVector<QVector<double>>& values,
                             const QVector<double>& b, int n);

    /** @brief 矩阵-向量乘积 */
    QVector<double> matVecMultiply(const QVector<QVector<double>>& A,
                                    const QVector<double>& x) const;

    /** @brief 计算残差 r = b - Ax */
    QVector<double> residual(const QVector<QVector<double>>& A,
                              const QVector<double>& x,
                              const QVector<double>& b) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int iterations, double residualNorm, bool converged);

private:
    int m_maxIter = 1000;
    double m_tol = 1e-8;
    bool m_usePrecond = true;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Dot product */
    double dot(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Vector norm */
    double norm(const QVector<double>& v) const;

    /** @brief Jacobi preconditioner: M^{-1} r */
    QVector<double> jacobiPrecond(const QVector<double>& r,
                                   const QVector<double>& diag) const;

    /** @brief Extract diagonal of A */
    QVector<double> diagonal(const QVector<QVector<double>>& A) const;
};
