/**
 * @file GaussSeidel3.h
 * @brief Gauss-Seidel迭代(逐次超松弛SOR+红黑排序并行收敛) — Gauss-Seidel with Successive Over-Relaxation SOR and Red-Black Ordering for Parallel Convergence
 *
 * 功能: 实现Gauss-Seidel迭代求解器，支持SOR松弛、
 *       红黑排序并行化和收敛性监测。
 *
 * 协作: Jacobi4(Jacobi迭代) / ConjugateGradient5(共轭梯度) / SparseMatrix3(稀疏矩阵)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Gauss-Seidel迭代(SOR+红黑排序)
 */
class GaussSeidel3 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalSolves = 0;
        int matrixSize = 0;
        int iterations = 0;
        double omega = 1.0;    // SOR relaxation factor
        double finalResidual = 0.0;
        bool converged = false;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GaussSeidel3(QObject *parent = nullptr);
    ~GaussSeidel3() override;

    /** @brief Set SOR relaxation parameter omega (1.0 = pure Gauss-Seidel) */
    void setOmega(double omega);

    /** @brief Set convergence tolerance and max iterations */
    void setConvergence(double tolerance, int maxIterations);

    /** @brief Solve Ax = b where A is stored as row-major dense matrix */
    QVector<double> solve(const QVector<QVector<double>>& A,
                          const QVector<double>& b);

    /** @brief Solve with red-black ordering for parallel-friendly convergence */
    QVector<double> solveRedBlack(const QVector<QVector<double>>& A,
                                  const QVector<double>& b);

    /** @brief Compute residual ||Ax - b|| */
    static double residual(const QVector<QVector<double>>& A,
                           const QVector<double>& x,
                           const QVector<double>& b);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int iterations, double residual, double timeMs);

private:
    double m_omega = 1.0;
    double m_tolerance = 1e-10;
    int m_maxIter = 1000;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Single Gauss-Seidel sweep (SOR update) */
    static void sweepSOR(QVector<double>& x,
                         const QVector<QVector<double>>& A,
                         const QVector<double>& b,
                         double omega);

    /** @brief Red sweep: update red-colored unknowns */
    static void sweepRed(QVector<double>& x,
                         const QVector<QVector<double>>& A,
                         const QVector<double>& b,
                         double omega);

    /** @brief Black sweep: update black-colored unknowns */
    static void sweepBlack(QVector<double>& x,
                           const QVector<QVector<double>>& A,
                           const QVector<double>& b,
                           double omega);
};
