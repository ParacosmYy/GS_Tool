/**
 * @file GMRES4.h
 * @brief 广义最小残差法(灵活预处理+调和Ritz值收缩重启慢收敛) — GMRES with Flexible Preconditioning and Deflated Restarting with Harmonic Ritz Values for Slow Convergence
 *
 * 功能: 实现GMRES求解器，支持灵活预处理(flexible preconditioning)，
 *       使用调和Ritz值(harmonic Ritz values)引导收缩重启(deflated restarting)
 *       改善慢收敛问题。
 *
 * 协作: SparseMatrix3(稀疏矩阵) / ConjugateGradient5(共轭梯度) / BiCGSTAB4(双共轭梯度)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief GMRES求解器(灵活预处理+调和Ritz值收缩重启)
 */
class GMRES4 : public QObject {
    Q_OBJECT

public:
    /** @brief Solver result */
    struct SolveResult {
        QVector<double> solution;
        int iterations = 0;
        double residual = 0.0;
        bool converged = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int problemSize = 0;
        int restartLength = 0;
        int totalIterations = 0;
        int numDeflations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GMRES4(QObject *parent = nullptr);
    ~GMRES4() override;

    /** @brief Set solver parameters */
    void setParameters(int restartLength, double tolerance, int maxIter);

    /** @brief Solve Ax = b with matrix-vector product callback style */
    SolveResult solve(int n,
                      const QVector<QVector<double>>& A,
                      const QVector<double>& b);

    /** @brief Solve with preconditioning (left preconditioner) */
    SolveResult solvePreconditioned(int n,
                                      const QVector<QVector<double>>& A,
                                      const QVector<double>& b,
                                      const QVector<QVector<double>>& M);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int iters, double residual, double timeMs);
    void iterationProgress(int iter, double residual);

private:
    int m_restartLength = 30;
    double m_tolerance = 1e-8;
    int m_maxIter = 500;
    int m_numDeflationVectors = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Matrix-vector multiplication */
    QVector<double> matVec(const QVector<QVector<double>>& A,
                            const QVector<double>& x) const;

    /** @brief Dot product */
    double dot(const QVector<double>& a,
               const QVector<double>& b) const;

    /** @brief Vector norm */
    double norm(const QVector<double>& v) const;

    /** @brief Arnoldi process to build Hessenberg matrix */
    int arnoldi(int k, int m, const QVector<QVector<double>>& A,
                QVector<QVector<double>>& V, QVector<QVector<double>>& H);

    /** @brief Solve least-squares upper Hessenberg system */
    QVector<double> solveHessenberg(const QVector<QVector<double>>& H,
                                      const QVector<double>& g, int k) const;

    /** @brief Compute harmonic Ritz values for deflation */
    QVector<double> harmonicRitzValues(const QVector<QVector<double>>& H,
                                         int k) const;
};
