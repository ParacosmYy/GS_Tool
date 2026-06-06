/**
 * @file GMRES.h
 * @brief GMRES迭代求解器(Arnoldi过程+重启变体+柔性预处理) — GMRES Iterative Solver with Arnoldi Process, Restarted Variant and Flexible Preconditioning
 *
 * 功能: 实现GMRES迭代线性方程组求解器，支持Arnoldi正交化过程、
 *       重启GMRES(m)变体和柔性预处理(flexible preconditioning)。
 *
 * 协作: SORSolver5(SOR) / CGSolver6(CG) / SparseMatrix7(稀疏矩阵)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief GMRES迭代求解器(Arnoldi+重启+柔性预处理)
 */
class GMRES : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSolves = 0;
        int iterationsUsed = 0;
        double finalResidual = 0.0;
        double initialResidual = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GMRES(QObject *parent = nullptr);
    ~GMRES() override;

    void setMaxIterations(int iter);
    void setTolerance(double tol);
    void setRestartInterval(int m);
    void setFlexiblePreconditioning(bool enabled);

    /** @brief 求解线性方程组 Ax=b，A为密集矩阵 */
    QVector<double> solve(const QVector<QVector<double>>& A,
                          const QVector<double>& b);

    /** @brief 求解线性方程组(A为CSR稀疏格式) */
    QVector<double> solveSparse(const QVector<int>& rowPtr,
                                const QVector<int>& colIdx,
                                const QVector<double>& values,
                                const QVector<double>& b);

    /** @brief 计算残差 ||Ax-b|| */
    double residual(const QVector<QVector<double>>& A,
                    const QVector<double>& x,
                    const QVector<double>& b) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int iterations, double residual);

private:
    int m_maxIterations = 100;
    double m_tolerance = 1e-8;
    int m_restartM = 30;
    bool m_flexible = false;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Dot product */
    double dot(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Vector norm */
    double norm(const QVector<double>& v) const;

    /** @brief Matrix-vector multiply */
    QVector<double> matVec(const QVector<QVector<double>>& A,
                           const QVector<double>& x) const;

    /** @brief Sparse matrix-vector multiply */
    QVector<double> sparseMatVec(const QVector<int>& rowPtr,
                                 const QVector<int>& colIdx,
                                 const QVector<double>& values,
                                 const QVector<double>& x) const;

    /** @brief Apply preconditioner (Jacobi diagonal scaling) */
    QVector<double> precondition(const QVector<QVector<double>>& A,
                                 const QVector<double>& r) const;

    /** @brief Back-substitution for upper triangular */
    QVector<double> backSolve(const QVector<QVector<double>>& H,
                              const QVector<double>& g, int k) const;
};
