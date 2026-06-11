/**
 * @file GMRES11.h
 * @brief GMRES求解器(灵活预处理与增强Krylov子空间实现跨多次求解的基向量高效复用) — GMRES with Flexible Preconditioning and Augmented Krylov Subspace for Efficient Reuse of Basis Vectors Across Multiple Solves
 *
 * 功能: 实现GMRES求解器(GMRES solver)，采用灵活预处理(flexible preconditioning)
 *       与增强Krylov子空间(augmented Krylov subspace)实现跨多次求解的基向量高效复用(efficient reuse of basis vectors across multiple solves)。
 *
 * 协作: ConjugateGradient(共轭梯度) / BiCGSTAB(双共轭梯度稳定) / LUSolver(LU分解)
 */
#pragma once

#include <QObject>
#include <QVector>

class GMRES11 : public QObject {
    Q_OBJECT

public:
    /** @brief Sparse matrix in CRS format */
    struct SparseMatrix {
        int rows = 0;
        int cols = 0;
        QVector<double> values;
        QVector<int> colIndices;
        QVector<int> rowPtr;
    };

    /** @brief Solve result */
    struct SolveResult {
        QVector<double> solution;
        double residual = 0.0;
        int iterations = 0;
        bool converged = false;
        double elapsedMs = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalSolves = 0;
        int maxIterations = 0;
        double avgResidual = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GMRES11(QObject *parent = nullptr);
    ~GMRES11() override;

    void setMaxIterations(int iter);
    void setTolerance(double tol);
    void setRestartInterval(int m);

    /** @brief Set augmented subspace vectors for basis reuse */
    void setAugmentedVectors(const QVector<QVector<double>>& vecs);

    /** @brief Solve Ax = b with flexible preconditioning */
    SolveResult solve(const SparseMatrix& A, const QVector<double>& b);

    /** @brief Solve with initial guess */
    SolveResult solveWithGuess(const SparseMatrix& A, const QVector<double>& b,
                               const QVector<double>& x0);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveDone(int iterations, double residual, double timeMs);

private:
    int m_maxIter = 1000;
    double m_tol = 1e-10;
    int m_restart = 30;
    Stats m_stats;
    double m_resSum = 0.0;
    double m_timeSum = 0.0;

    // Augmented subspace vectors for basis reuse
    QVector<QVector<double>> m_augVecs;

    /** @brief Sparse matrix-vector product y = A*x */
    QVector<double> spmv(const SparseMatrix& A, const QVector<double>& x) const;

    /** @brief Flexible preconditioner application (Jacobi-like) */
    QVector<double> precondition(const SparseMatrix& A, const QVector<double>& r) const;

    /** @brief Arnoldi process with flexible preconditioning */
    void arnoldi(const SparseMatrix& A, const QVector<QVector<double>>& V,
                 QVector<double>& h, int j);

    /** @brief Solve upper Hessenberg least squares via Givens rotations */
    QVector<double> solveHessenberg(const QVector<QVector<double>>& H,
                                    const QVector<double>& g,
                                    int m) const;

    /** @brief Vector dot product */
    double dot(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Vector 2-norm */
    double norm(const QVector<double>& v) const;
};
