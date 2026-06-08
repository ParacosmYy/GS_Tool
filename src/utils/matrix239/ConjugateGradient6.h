/**
 * @file ConjugateGradient6.h
 * @brief 共轭梯度法(不完全Cholesky预处理+柔性CG变预处理求解) — Conjugate Gradient with Incomplete Cholesky Preconditioner and Flexible CG for Variable Preconditioning
 *
 * 功能: 实现共轭梯度法(Conjugate Gradient, CG)，采用不完全Cholesky分解
 *       (incomplete Cholesky preconditioner)作为预处理器(preconditioner)，并支持
 *       柔性CG(Flexible CG)以适应变预处理(variable preconditioning)场景，
 *       用于大规模稀疏对称正定线性系统求解。
 *
 * 协作: GaussianElimination5(高斯消元) / LuDecomposition4(LU分解) / SvdSolver5(SVD求解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 共轭梯度法(不完全Cholesky预处理+柔性CG变预处理)
 */
class ConjugateGradient6 : public QObject {
    Q_OBJECT

public:
    /** @brief Sparse matrix entry (row-compressed) */
    struct SparseEntry {
        int col = 0;
        double val = 0.0;
    };

    /** @brief Solver result */
    struct SolveResult {
        QVector<double> x;
        int iterations = 0;
        double finalResidual = 0.0;
        bool converged = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int totalIterations = 0;
        int numConverged = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ConjugateGradient6(QObject *parent = nullptr);
    ~ConjugateGradient6() override;

    /** @brief Set max iterations */
    void setMaxIterations(int iter);

    /** @brief Set convergence tolerance */
    void setTolerance(double tol);

    /** @brief Enable flexible CG mode */
    void setFlexibleMode(bool flexible);

    /** @brief Solve Ax = b with dense matrix A (SPD) */
    SolveResult solveDense(const QVector<QVector<double>>& A, const QVector<double>& b);

    /** @brief Solve Ax = b with sparse matrix (CRS format) */
    SolveResult solveSparse(const QVector<QVector<SparseEntry>>& A,
                            const QVector<double>& b);

    /** @brief Incomplete Cholesky factorization L */
    QVector<QVector<SparseEntry>> incompleteCholesky(const QVector<QVector<SparseEntry>>& A) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void iterationAdvanced(int iter, double residual);
    void solveCompleted(int iterations, double residual, bool converged);

private:
    int m_maxIterations = 1000;
    double m_tolerance = 1e-8;
    bool m_flexibleMode = false;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Dense matrix-vector multiply */
    QVector<double> denseMV(const QVector<QVector<double>>& A, const QVector<double>& x) const;

    /** @brief Sparse matrix-vector multiply */
    QVector<double> sparseMV(const QVector<QVector<SparseEntry>>& A, const QVector<double>& x) const;

    /** @brief Forward solve L*y = b */
    QVector<double> forwardSolve(const QVector<QVector<SparseEntry>>& L, const QVector<double>& b) const;

    /** @brief Backward solve L^T*x = y */
    QVector<double> backwardSolve(const QVector<QVector<SparseEntry>>& L, const QVector<double>& y) const;

    /** @brief Dot product */
    static double dot(const QVector<double>& a, const QVector<double>& b);

    /** @brief Vector norm */
    static double norm(const QVector<double>& v);
};
