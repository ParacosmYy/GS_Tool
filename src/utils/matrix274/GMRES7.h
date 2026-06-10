/**
 * @file GMRES7.h
 * @brief GMRES(重启Arnoldi过程与Givens旋转QR更新Krylov子空间最小二乘求解) — GMRES with Restarted Arnoldi Process and Givens Rotation QR Update for Least-Squares Minimization in Krylov Subspace
 *
 * 功能: 实现GMRES(广义最小残差法)，采用重启Arnoldi过程(restarted Arnoldi process)
 *       与Givens旋转QR更新(Givens rotation QR update)实现Krylov子空间最小二乘求解(least-squares minimization)。
 *
 * 协作: BiCGSTAB6(双共轭梯度) / ConjugateGradient5(共轭梯度) / SparseMatrix4(稀疏矩阵)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief GMRES(重启Arnoldi过程与Givens旋转QR更新Krylov子空间最小二乘求解)
 */
class GMRES7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int krylovDim = 0;
        int iterations = 0;
        double residual = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GMRES7(QObject *parent = nullptr);
    ~GMRES7() override;

    /** @brief Set max Krylov subspace dimension (restart parameter) */
    void setKrylovDimension(int m);

    /** @brief Set convergence tolerance */
    void setTolerance(double tol);

    /** @brief Set maximum restarts */
    void setMaxRestarts(int maxR);

    /** @brief Solve Ax = b with A as row-major dense matrix */
    QVector<double> solve(const QVector<QVector<double>>& A,
                          const QVector<double>& b);

    /** @brief Get final residual norm */
    double residual() const;

    /** @brief Get iteration count */
    int iterations() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int iterations, double residual, double timeMs);

private:
    int m_krylovDim = 30;
    double m_tol = 1e-8;
    int m_maxRestarts = 100;

    double m_residual = 0.0;
    int m_iterations = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Matrix-vector multiply */
    QVector<double> matVec(const QVector<QVector<double>>& A,
                           const QVector<double>& x) const;

    /** @brief Dot product */
    double dot(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Vector norm */
    double norm(const QVector<double>& v) const;

    /** @brief Apply Givens rotation to Hessenberg column */
    void applyGivens(QVector<double>& col, const QVector<QPair<double, double>>& rotations,
                     int startRow) const;

    /** @brief Back-substitution for upper triangular solve */
    QVector<double> backSubstitute(const QVector<QVector<double>>& R,
                                   const QVector<double>& rhs, int k) const;
};
