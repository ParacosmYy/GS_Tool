/**
 * @file BiCGSTAB6.h
 * @brief BiCGSTAB求解器(GPBi-CG稳定化+右预条件变体含灵活GMRES内迭代) — BiCGSTAB with GPBi-CG Stabilization and Right-Preconditioned Variant with Flexible GMRES Inner Iteration
 *
 * 功能: 实现BiCGSTAB线性方程组求解器，集成GPBi-CG稳定化策略，
 *       支持右预条件和灵活GMRES内迭代处理变预条件子。
 *
 * 协作: CGSolver5(CG) / GMRES4(GMRES) / SparseMatrix6(稀疏矩阵)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief BiCGSTAB求解器(GPBi-CG稳定化+右预条件)
 */
class BiCGSTAB6 : public QObject {
    Q_OBJECT

public:
    /** @brief Sparse matrix entry (COO format) */
    struct Entry {
        int row = 0;
        int col = 0;
        double val = 0.0;
    };

    /** @brief Solver result */
    struct SolveResult {
        QVector<double> x;
        double residualNorm = 0.0;
        int iterations = 0;
        bool converged = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int nnz = 0;
        int totalIterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BiCGSTAB6(QObject *parent = nullptr);
    ~BiCGSTAB6() override;

    /** @brief Set solver parameters: tolerance, max iterations, use right preconditioning */
    void setParameters(double tolerance = 1e-10, int maxIter = 1000,
                        bool rightPrecond = true);

    /** @brief Load sparse matrix in COO format */
    void loadMatrix(int n, const QVector<Entry>& entries);

    /** @brief Solve Ax = b */
    SolveResult solve(const QVector<double>& b);

    /** @brief Solve with initial guess */
    SolveResult solve(const QVector<double>& b, const QVector<double>& x0);

    /** @brief Apply Jacobi preconditioner: solve Mz = r (M = diag(A)) */
    void jacobiPrecondition(const QVector<double>& r,
                              QVector<double>& z) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int iterations, double residual, double timeMs);

private:
    double m_tolerance = 1e-10;
    int m_maxIter = 1000;
    bool m_rightPrecond = true;
    int m_n = 0;

    // CSR storage
    QVector<double> m_values;
    QVector<int> m_colIdx;
    QVector<int> m_rowPtr;
    QVector<double> m_diag;  // for Jacobi preconditioner

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Sparse matrix-vector product y = A*x */
    void spMV(const QVector<double>& x, QVector<double>& y) const;

    /** @brief Dot product */
    static double dot(const QVector<double>& a, const QVector<double>& b);

    /** @brief Vector norm */
    static double norm(const QVector<double>& v);

    /** @brief Flexible GMRES inner iteration (1-3 steps) */
    void flexibleInnerGMRES(const QVector<double>& r,
                              QVector<double>& z, int steps) const;

    /** @brief GPBi-CG stabilization polynomial update */
    void gpBiCGStabilize(QVector<double>& r, QVector<double>& p,
                           QVector<double>& v, const QVector<double>& s,
                           double omega, double alpha) const;
};
