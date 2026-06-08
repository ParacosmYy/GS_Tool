/**
 * @file BiCGSTAB7.h
 * @brief 双共轭梯度稳定法(右预条件+IDR(s)稳定变体) — BiCGSTAB with Right Preconditioning and IDR(s) Stabilized Variant for Robust Asymmetric System Solution
 *
 * 功能: 实现BiCGSTAB(双共轭梯度稳定法)求解器，采用右预条件(right preconditioning)加速收敛，
 *       并集成IDR(s)稳定变体(IDR(s) stabilized variant)以增强对非对称系统(asymmetric system)的鲁棒性。
 *
 * 协作: GMRES5(GMRES) / SparseLU4(稀疏LU) / ConjugateGradient3(共轭梯度)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 双共轭梯度稳定法(右预条件+IDR(s)稳定变体)
 */
class BiCGSTAB7 : public QObject {
    Q_OBJECT

public:
    /** @brief Sparse matrix entry (COO format) */
    struct Entry {
        int row = 0;
        int col = 0;
        double value = 0.0;
    };

    /** @brief Solver result */
    struct SolveResult {
        QVector<double> solution;
        int iterations = 0;
        double residualNorm = 0.0;
        double initialResidual = 0.0;
        bool converged = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int numNonZeros = 0;
        int totalIterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BiCGSTAB7(QObject *parent = nullptr);
    ~BiCGSTAB7() override;

    /** @brief Set solver parameters */
    void setParameters(int maxIter = 1000, double tolerance = 1e-10, int idrS = 4);

    /** @brief Build sparse matrix from COO entries */
    void buildMatrix(int n, const QVector<Entry>& entries);

    /** @brief Solve Ax = b */
    SolveResult solve(const QVector<double>& b, const QVector<double>& x0 = {});

    /** @brief Get current residual history */
    QVector<double> residualHistory() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void iterationCompleted(int iter, double residual);
    void solveCompleted(int iterations, double finalResidual, double timeMs);

private:
    int m_n = 0;
    int m_maxIter = 1000;
    double m_tol = 1e-10;
    int m_idrS = 4;

    // CSR storage
    QVector<double> m_values;
    QVector<int> m_colIdx;
    QVector<int> m_rowPtr;

    // Diagonal for Jacobi preconditioner
    QVector<double> m_diag;

    // IDR(s) subspace vectors
    QVector<QVector<double>> m_pVectors;  // s vectors for IDR
    QVector<QVector<double>> m_gVectors;

    QVector<double> m_residualHistory;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief CSR SpMV: y = A * x */
    void spmv(const QVector<double>& x, QVector<double>& y) const;

    /** @brief Jacobi preconditioner solve: M^{-1} * r */
    void precondition(const QVector<double>& r, QVector<double>& z) const;

    /** @brief Vector operations */
    static double dot(const QVector<double>& a, const QVector<double>& b);
    static double norm(const QVector<double>& v);
    static void axpy(double alpha, const QVector<double>& x, QVector<double>& y);
};
