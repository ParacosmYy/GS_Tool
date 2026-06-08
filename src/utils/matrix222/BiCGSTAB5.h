/**
 * @file BiCGSTAB5.h
 * @brief 稳定双共轭梯度法(复合步稳定化+多项式预处理) — BiCGSTAB with Composite Step Stabilization and Polynomial Preconditioner for Asymmetric Spectral Radius
 *
 * 功能: 实现BiCGSTAB线性方程组求解器，采用复合步稳定化策略
 *       防止算法停滞，多项式预处理器优化非对称谱半径收敛。
 *
 * 协作: GMRES4(GMRES求解器) / ConjugateGradient4(共轭梯度) / SparseMatrix4(稀疏矩阵)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 稳定双共轭梯度法(复合步稳定化+多项式预处理)
 */
class BiCGSTAB5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalSolves = 0;
        int matrixSize = 0;
        int iterations = 0;
        double finalResidual = 0.0;
        double initialResidual = 0.0;
        bool converged = false;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BiCGSTAB5(QObject *parent = nullptr);
    ~BiCGSTAB5() override;

    /** @brief Set parameters: max iterations, tolerance, preconditioner degree */
    void setParameters(int maxIter = 500, double tolerance = 1e-10,
                       int precondDegree = 2);

    /** @brief Solve Ax = b with sparse matrix (CSR format) */
    QVector<double> solve(const QVector<double>& values,
                          const QVector<int>& colIdx,
                          const QVector<int>& rowPtr,
                          const QVector<double>& rhs);

    /** @brief Compute residual norm ||Ax - b|| */
    double residualNorm(const QVector<double>& values,
                        const QVector<int>& colIdx,
                        const QVector<int>& rowPtr,
                        const QVector<double>& x,
                        const QVector<double>& rhs) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int iterations, double residual, bool converged,
                        double timeMs);

private:
    int m_maxIter = 500;
    double m_tol = 1e-10;
    int m_precondDegree = 2;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Sparse matrix-vector product */
    QVector<double> spmv(const QVector<double>& values,
                         const QVector<int>& colIdx,
                         const QVector<int>& rowPtr,
                         const QVector<double>& x) const;

    /** @brief Polynomial preconditioner application */
    QVector<double> polyPrecondition(const QVector<double>& values,
                                     const QVector<int>& colIdx,
                                     const QVector<int>& rowPtr,
                                     const QVector<double>& r) const;

    /** @brief Dot product of two vectors */
    double dot(const QVector<double>& a,
               const QVector<double>& b) const;

    /** @brief Compute initial preconditioner coefficients */
    void estimatePrecondCoeffs(const QVector<double>& values,
                               const QVector<int>& colIdx,
                               const QVector<int>& rowPtr);
};
