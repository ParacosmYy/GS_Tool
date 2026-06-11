/**
 * @file GaussSeidel9.h
 * @brief 高斯-赛德尔迭代(红黑排序与逐次超松弛加速椭圆型PDE离散化收敛) — Gauss-Seidel with Red-black Ordering and Successive Over-relaxation for Accelerating Convergence of Elliptic PDE Discretizations
 *
 * 功能: 实现高斯-赛德尔迭代(Gauss-Seidel iteration)，采用红黑排序(red-black ordering)
 *       与逐次超松弛(successive over-relaxation)加速椭圆型PDE离散化收敛(elliptic PDE discretization convergence)。
 *
 * 协作: ConjugateGradient(共轭梯度) / JacobiIteration(Jacobi迭代) / SparseMatrix(稀疏矩阵)
 */
#pragma once

#include <QObject>
#include <QVector>

class GaussSeidel9 : public QObject {
    Q_OBJECT

public:
    /** @brief Solver result */
    struct SolveResult {
        QVector<double> solution;
        int iterations = 0;
        double finalResidual = 0.0;
        bool converged = false;
        double solveTimeMs = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalSolves = 0;
        int problemSize = 0;
        double avgIterations = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GaussSeidel9(QObject *parent = nullptr);
    ~GaussSeidel9() override;

    void setMaxIterations(int maxIter);
    void setTolerance(double tol);
    void setRelaxationFactor(double omega);

    /** @brief Solve Ax = b using red-black GS-SOR */
    SolveResult solve(const QVector<QVector<double>>& A,
                      const QVector<double>& b);

    /** @brief Solve 2D Poisson equation on uniform grid */
    SolveResult solvePoisson2D(const QVector<QVector<double>>& rhs,
                                int nx, int ny, double dx, double dy);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveDone(int n, int iters, double residual, double timeMs);

private:
    int m_maxIterations = 10000;
    double m_tolerance = 1e-10;
    double m_omega = 1.5;  // SOR relaxation factor
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_iterSum = 0;

    /** @brief Compute residual ||Ax - b||_inf */
    double computeResidual(const QVector<QVector<double>>& A,
                           const QVector<double>& x,
                           const QVector<double>& b) const;

    /** @brief Red-black sweep for general sparse matrix */
    void redBlackSweep(const QVector<QVector<double>>& A,
                       QVector<double>& x,
                       const QVector<double>& b);

    /** @brief Red-black sweep for 2D Poisson (5-point stencil) */
    void poissonSweep(QVector<QVector<double>>& u,
                      const QVector<QVector<double>>& rhs,
                      int nx, int ny, double dx, double dy);
};
