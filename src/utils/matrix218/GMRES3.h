/**
 * @file GMRES3.h
 * @brief GMRES求解器(重启Arnoldi+泄放重启加速聚类特征值收敛) — GMRES with Restarted Arnoldi and Deflated Restarting for Clustered Eigenvalue Convergence Acceleration
 *
 * 功能: 实现GMRES迭代求解器，支持重启Arnoldi过程、
 *       泄放重启策略和聚类特征值收敛加速。
 *
 * 协作: ConjugateGradient2(共轭梯度) / SparseMatrix3(稀疏矩阵) / SVD2(奇异值分解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief GMRES求解器(重启Arnoldi+泄放重启)
 */
class GMRES3 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalSolves = 0;
        int problemSize = 0;
        int krylovDim = 0;
        int iterations = 0;
        int restarts = 0;
        double residualNorm = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GMRES3(QObject *parent = nullptr);
    ~GMRES3() override;

    /** @brief Set Krylov subspace dimension and convergence tolerance */
    void setParameters(int krylovDim = 30, double tolerance = 1e-8,
                       int maxRestarts = 100);

    /** @brief Solve Ax = b where A is a dense matrix */
    QVector<double> solve(const QVector<QVector<double>>& A,
                          const QVector<double>& b);

    /** @brief Solve with matrix-vector product callback (sparse-friendly) */
    QVector<double> solveSparse(
        const QVector<double>& b, int n,
        std::function<QVector<double>(const QVector<double>&)> matvec) const;

    /** @brief Perform one Arnoldi step: extend Krylov basis */
    void arnoldiStep(QVector<QVector<double>>& V,
                     QVector<QVector<double>>& H,
                     int step, int n,
                     const std::function<QVector<double>(const QVector<double>&)>& matvec) const;

    /** @brief Apply Givens rotations to Hessenberg matrix */
    void applyGivens(QVector<QVector<double>>& H,
                     QVector<double>& cosRot,
                     QVector<double>& sinRot,
                     int step) const;

    /** @brief Compute residual norm without explicit x update */
    double computeResidual(const QVector<QVector<double>>& H,
                           const QVector<double>& g,
                           int step) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int iterations, double residual, double timeMs);

private:
    int m_krylovDim = 30;
    double m_tol = 1e-8;
    int m_maxRestarts = 100;

    Stats m_stats;
    double m_timeSum = 0.0;

    // Deflated restart: stored Ritz vectors
    QVector<QVector<double>> m_deflatedVectors;

    /** @brief Dot product of two vectors */
    static double dot(const QVector<double>& a, const QVector<double>& b);

    /** @brief Vector 2-norm */
    static double norm(const QVector<double>& v);

    /** @brief Back-solve upper triangular system */
    static QVector<double> backSolve(const QVector<QVector<double>>& R,
                                     const QVector<double>& rhs, int size);
};
