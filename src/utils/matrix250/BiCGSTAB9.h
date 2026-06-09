/**
 * @file BiCGSTAB9.h
 * @brief 双共轭梯度稳定法(右预条件+复合步稳定的改进收敛鲁棒性) — BiCGSTAB with Right Preconditioning and Composite Step Stabilization for Improved Convergence Robustness
 *
 * 功能: 实现双共轭梯度稳定法(BiCGSTAB)，采用右预条件(right preconditioning)
 *       加速收敛，复合步稳定(composite step stabilization)提升收敛鲁棒性，
 *       求解大型稀疏非对称线性方程组。
 *
 * 协作: ConjugateGradient8(共轭梯度) / GMRES7(GMRES) / SparseMatrix6(稀疏矩阵)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 双共轭梯度稳定法(右预条件+复合步稳定)
 */
class BiCGSTAB9 : public QObject {
    Q_OBJECT

public:
    /** @brief Iteration record */
    struct IterationRecord {
        int iteration = 0;
        double residualNorm = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int iterations = 0;
        double finalResidual = 0.0;
        double initialResidual = 0.0;
        bool converged = false;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BiCGSTAB9(QObject *parent = nullptr);
    ~BiCGSTAB9() override;

    /** @brief Set convergence tolerance */
    void setTolerance(double tol);

    /** @brief Set maximum iterations */
    void setMaxIterations(int iters);

    /** @brief Solve Ax = b with sparse matrix (CSR format) */
    QVector<double> solve(const QVector<double>& values,
                          const QVector<int>& colIndices,
                          const QVector<int>& rowPtr,
                          int n,
                          const QVector<double>& b);

    /** @brief Get convergence history */
    QVector<IterationRecord> history() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int iterations, double residual, bool converged, double timeMs);

private:
    double m_tol = 1e-10;
    int m_maxIter = 1000;

    QVector<IterationRecord> m_history;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Sparse matrix-vector product (CSR) */
    static QVector<double> spMV(const QVector<double>& val,
                                const QVector<int>& col,
                                const QVector<int>& rowPtr,
                                int n,
                                const QVector<double>& x);

    /** @brief Jacobi preconditioner (diagonal inverse) */
    static QVector<double> jacobiPrecond(const QVector<double>& val,
                                         const QVector<int>& col,
                                         const QVector<int>& rowPtr,
                                         int n,
                                         const QVector<double>& r);

    /** @brief Vector dot product */
    static double dot(const QVector<double>& a, const QVector<double>& b);

    /** @brief Vector norm */
    static double norm(const QVector<double>& v);
};
