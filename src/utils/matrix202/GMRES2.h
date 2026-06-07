/**
 * @file GMRES2.h
 * @brief 灵活GMRES(可变预处理+子空间回收GCRO-DR) — Flexible GMRES with Variable Preconditioning and Subspace Recycling (GCRO-DR)
 *
 * 功能: 实现灵活GMRES求解器，支持可变预处理、
 *       子空间回收(GCRO-DR)和Krylov子空间管理。
 *
 * 协作: SparseMatrix8(稀疏矩阵) / ConjugateGradient5(CG) / BiCGSTAB3(BiCGSTAB)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 灵活GMRES(可变预处理+子空间回收)
 */
class GMRES2 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSolves = 0;
        int matrixSize = 0;
        int iterationsUsed = 0;
        double finalResidual = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Sparse matrix entry */
    struct SparseEntry {
        int row = 0;
        int col = 0;
        double value = 0.0;
    };

    /** @brief Preconditioner function type */
    using PrecondFunc = std::function<QVector<double>(const QVector<double>&)>;

    explicit GMRES2(QObject *parent = nullptr);
    ~GMRES2() override;

    void setMaxIterations(int iters);
    void setTolerance(double tol);
    void setRestartInterval(int m);
    void setRecycleSize(int k);

    /** @brief Solve Ax = b with dense matrix */
    QVector<double> solve(const QVector<QVector<double>>& A,
                          const QVector<double>& b);

    /** @brief Solve with sparse matrix (COO format) */
    QVector<double> solveSparse(const QVector<SparseEntry>& entries,
                                int n, const QVector<double>& b);

    /** @brief Solve with preconditioner */
    QVector<double> solvePreconditioned(const QVector<QVector<double>>& A,
                                        const QVector<double>& b,
                                        PrecondFunc precond);

    /** @brief Compute residual ||Ax - b|| */
    double residual(const QVector<QVector<double>>& A,
                    const QVector<double>& x, const QVector<double>& b) const;

    /** @brief Matrix-vector product */
    QVector<double> matvec(const QVector<QVector<double>>& A,
                           const QVector<double>& x) const;

    /** @brief Sparse matrix-vector product */
    QVector<double> sparseMatvec(const QVector<SparseEntry>& entries,
                                 const QVector<double>& x) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int iterations, double residual, double timeMs);

private:
    int m_maxIter = 1000;
    double m_tol = 1e-8;
    int m_restart = 30;
    int m_recycleK = 5;

    // Recycled subspace (GCRO-DR)
    QVector<QVector<double>> m_recycleV;
    QVector<QVector<double>> m_recycleU;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Arnoldi process with preconditioning */
    void arnoldi(QVector<QVector<double>>& V, QVector<QVector<double>>& H,
                 const QVector<QVector<double>>& A, PrecondFunc precond,
                 int j, QVector<double>& w) const;

    /** @brief Solve upper Hessenberg least-squares via Givens rotations */
    QVector<double> solveHessenberg(const QVector<QVector<double>>& H,
                                    const QVector<double>& g,
                                    int m) const;

    /** @brief Apply Givens rotation */
    void applyGivens(QVector<double>& h, QVector<double>& cs,
                     QVector<double>& sn, int i) const;

    /** @brief Dot product */
    static double dot(const QVector<double>& a, const QVector<double>& b);

    /** @brief Vector norm */
    static double norm(const QVector<double>& v);

    /** @brief Convert sparse COO to dense */
    QVector<QVector<double>> sparseToDense(const QVector<SparseEntry>& entries, int n) const;
};
