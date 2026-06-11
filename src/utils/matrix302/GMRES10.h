/**
 * @file GMRES10.h
 * @brief GMRES(嵌套Krylov子空间回收与压缩重启实现多右端项线性系统求解) — GMRES with Nested Krylov Subspace Recycling and Deflated Restarting for Solving Multiple Right-hand Side Linear Systems
 *
 * 功能: 实现GMRES(广义最小残差法)，采用嵌套Krylov子空间回收(nested Krylov subspace recycling)
 *       与压缩重启(deflated restarting)实现多右端项线性系统求解(solving multiple right-hand side linear systems)。
 *
 * 协作: ConjugateGradient(共轭梯度) / BiCGSTAB(双共轭梯度稳定) / SparseMatrix(稀疏矩阵)
 */
#pragma once

#include <QObject>
#include <QVector>

class GMRES10 : public QObject {
    Q_OBJECT

public:
    /** @brief Solver configuration */
    struct SolverConfig {
        int maxIterations = 200;
        int restartLength = 30;
        double tolerance = 1e-8;
        int numDeflationVectors = 5;
    };

    /** @brief Solver result */
    struct SolveResult {
        QVector<double> solution;
        double residualNorm = 0.0;
        double initialResidualNorm = 0.0;
        int iterations = 0;
        bool converged = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalSolves = 0;
        int problemSize = 0;
        double avgIterations = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GMRES10(QObject *parent = nullptr);
    ~GMRES10() override;

    void setConfig(const SolverConfig& config);

    /** @brief Solve Ax = b where A is given as rows */
    SolveResult solve(const QVector<QVector<double>>& A, const QVector<double>& b);

    /** @brief Solve multiple right-hand sides using recycled subspace */
    QVector<SolveResult> solveMultiple(const QVector<QVector<double>>& A,
                                        const QVector<QVector<double>>& B);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveDone(int size, int iters, double residual, double timeMs);

private:
    SolverConfig m_config;
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_iterSum = 0;

    // Recycled subspace from previous solve
    QVector<QVector<double>> m_deflationVectors;

    /** @brief Matrix-vector multiply */
    QVector<double> matVec(const QVector<QVector<double>>& A,
                            const QVector<double>& x) const;

    /** @brief Dot product */
    double dot(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Vector norm */
    double norm(const QVector<double>& v) const;

    /** @brief Apply Arnoldi iteration to build Krylov basis */
    void arnoldi(const QVector<QVector<double>>& A,
                  const QVector<double>& q,
                  QVector<QVector<double>>& Q,
                  QVector<double>& h, int k) const;

    /** @brief Solve upper Hessenberg least-squares via Givens rotations */
    QVector<double> solveHessenberg(const QVector<QVector<double>>& H,
                                     const QVector<double>& g,
                                     int m) const;

    /** @brief Apply deflation: project out recycled subspace */
    QVector<double> applyDeflation(const QVector<double>& v) const;

    /** @brief Update deflation vectors from converged solution */
    void updateDeflation(const QVector<QVector<double>>& Q,
                          int m, const QVector<double>& y);
};
