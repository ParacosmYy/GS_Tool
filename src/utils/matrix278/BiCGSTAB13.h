/**
 * @file BiCGSTAB13.h
 * @brief 双共轭梯度稳定法(右预条件与残差平滑不定线性系统改进收敛) — BiCGSTAB with Right Preconditioning and Residual Smoothing for Improved Convergence in Indefinite Linear Systems
 *
 * 功能: 实现双共轭梯度稳定法(BiCGSTAB)，采用右预条件(right preconditioning)
 *       与残差平滑(residual smoothing)实现不定线性系统改进收敛(improved convergence)。
 *
 * 协作: CGSolver14(共轭梯度) / GMRES10(GMRES) / SparseMatrix9(稀疏矩阵)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 双共轭梯度稳定法(右预条件与残差平滑)
 */
class BiCGSTAB13 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int numIterations = 0;
        int maxIterations = 500;
        double residualNorm = 0.0;
        double tolerance = 1e-10;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BiCGSTAB13(QObject *parent = nullptr);
    ~BiCGSTAB13() override;

    /** @brief Set maximum iterations */
    void setMaxIterations(int iters);

    /** @brief Set convergence tolerance */
    void setTolerance(double tol);

    /** @brief Solve Ax = b with dense A; returns solution x */
    QVector<double> solve(const QVector<QVector<double>>& A,
                          const QVector<double>& b);

    /** @brief Solve with sparse CSR-like: rowPtr, colIdx, values */
    QVector<double> solveSparse(int n,
                                const QVector<int>& rowPtr,
                                const QVector<int>& colIdx,
                                const QVector<double>& values,
                                const QVector<double>& b);

    /** @brief Get residual history per iteration */
    QVector<double> residualHistory() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveDone(int iterations, double residual, double timeMs);

private:
    int m_maxIter = 500;
    double m_tol = 1e-10;

    QVector<double> m_residualHistory;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Dense matrix-vector multiply */
    QVector<double> matVec(const QVector<QVector<double>>& A,
                           const QVector<double>& x) const;

    /** @brief Sparse matrix-vector multiply */
    QVector<double> sparseMatVec(int n,
                                  const QVector<int>& rowPtr,
                                  const QVector<int>& colIdx,
                                  const QVector<double>& values,
                                  const QVector<double>& x) const;

    /** @brief Jacobi preconditioner: M^{-1} r */
    QVector<double> precondition(const QVector<double>& r,
                                  const QVector<double>& diag) const;

    /** @brief Vector dot product */
    double dot(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Vector norm */
    double norm(const QVector<double>& v) const;
};
