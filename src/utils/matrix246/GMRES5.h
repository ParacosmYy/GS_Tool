/**
 * @file GMRES5.h
 * @brief GMRES求解器(重启Arnoldi+Householder QR最小二乘残差Krylov子空间) — GMRES Solver with Restarted Arnoldi and Householder QR for Least-Squares Residual Minimization in Krylov Subspace
 *
 * 功能: 实现广义最小残差法(GMRES)，通过重启Arnoldi过程(restarted Arnoldi)
 *       构建Krylov子空间正交基，结合Householder QR分解(Householder QR)求解
 *       最小二乘残差最小化(least-squares residual minimization)迭代求解线性系统。
 *
 * 协作: SVD9(奇异值分解) / Cholesky7(Cholesky分解) / BiCGSTAB6(双共轭梯度稳定法)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief GMRES求解器(重启Arnoldi+Householder QR最小二乘残差Krylov子空间)
 */
class GMRES5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int restartLength = 0;
        int totalIterations = 0;
        double finalResidual = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GMRES5(QObject *parent = nullptr);
    ~GMRES5() override;

    /** @brief Set restart length m for GMRES(m) */
    void setRestartLength(int m);

    /** @brief Set maximum total iterations */
    void setMaxIterations(int iters);

    /** @brief Set convergence tolerance on relative residual */
    void setTolerance(double tol);

    /**
     * @brief Solve Ax = b where A is given as a matrix-vector product callback
     * @param matrix Rows of the matrix (dense storage)
     * @param b Right-hand side vector
     * @return Solution vector x
     */
    QVector<double> solve(const QVector<QVector<double>>& matrix,
                          const QVector<double>& b);

    /** @brief Get residual history */
    QVector<double> residualHistory() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void iterationCompleted(int iter, double residual);
    void solveCompleted(int iters, double finalResidual, double timeMs);

private:
    int m_restart = 30;
    int m_maxIter = 1000;
    double m_tol = 1e-10;

    QVector<double> m_residuals;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Dense matrix-vector multiply */
    static QVector<double> matVec(const QVector<QVector<double>>& A,
                                  const QVector<double>& x);

    /** @brief Dot product */
    static double dot(const QVector<double>& a, const QVector<double>& b);

    /** @brief Vector norm */
    static double norm(const QVector<double>& v);

    /** @brief Arnoldi step: build one column of H and Q */
    void arnoldiStep(const QVector<QVector<double>>& A,
                     QVector<QVector<double>>& Q,
                     QVector<QVector<double>>& H,
                     int j) const;

    /** @brief Solve least-squares upper Hessenberg system via Householder QR */
    QVector<double> solveLeastSquares(const QVector<QVector<double>>& H,
                                      const QVector<double>& beta,
                                      int j) const;
};
