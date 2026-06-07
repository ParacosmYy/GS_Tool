/**
 * @file BiCGSTAB4.h
 * @brief BiCGSTAB求解器(GPBi-CG变体+SAINV预条件器用于不定对称系统) — BiCGSTAB with GPBi-CG Variant and SAINV Preconditioner for Indefinite Symmetric Systems
 *
 * 功能: 实现BiCGSTAB迭代求解器，支持GPBi-CG变体、
 *       SAINV(Sparse Approximate Inverse)预条件和不定对称系统。
 *
 * 协作: ConjugateGradient6(共轭梯度) / SparseMatrix5(稀疏矩阵) / GaussSeidel4(Gauss-Seidel)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief BiCGSTAB求解器(GPBi-CG变体+SAINV预条件器)
 */
class BiCGSTAB4 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalSolves = 0;
        int matrixSize = 0;
        int iterations = 0;
        double finalResidual = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BiCGSTAB4(QObject *parent = nullptr);
    ~BiCGSTAB4() override;

    void setMaxIterations(int maxIter);
    void setTolerance(double tol);
    void setUseGPBiCG(bool enable);

    /** @brief Build SAINV preconditioner from sparse matrix (CSR format) */
    void buildSAINV(const QVector<double>& values,
                     const QVector<int>& colIdx,
                     const QVector<int>& rowPtr, int n);

    /** @brief Solve Ax = b using BiCGSTAB */
    QVector<double> solve(const QVector<QVector<double>>& A,
                           const QVector<double>& b);

    /** @brief Solve with sparse CSR format */
    QVector<double> solveSparse(const QVector<double>& values,
                                 const QVector<int>& colIdx,
                                 const QVector<int>& rowPtr,
                                 const QVector<double>& b, int n);

    /** @brief Apply SAINV preconditioner: M^{-1} v */
    QVector<double> applyPreconditioner(const QVector<double>& v) const;

    /** @brief Get convergence history */
    QVector<double> convergenceHistory() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int iterations, double residual, double timeMs);

private:
    int m_maxIter = 1000;
    double m_tol = 1e-8;
    bool m_useGPBiCG = true;

    // SAINV preconditioner (lower/upper triangular factors)
    QVector<QVector<QPair<int, double>>> m_sainvL;
    QVector<QVector<QPair<int, double>>> m_sainvU;
    int m_precondN = 0;

    QVector<double> m_convergence;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Sparse matrix-vector product */
    static QVector<double> spmv(const QVector<QVector<double>>& A,
                                 const QVector<double>& x);

    /** @brief Dot product */
    static double dot(const QVector<double>& a, const QVector<double>& b);

    /** @brief Apply lower triangular solve */
    QVector<double> forwardSolve(
        const QVector<QVector<QPair<int, double>>>& L,
        const QVector<double>& b) const;

    /** @brief Apply upper triangular solve */
    QVector<double> backwardSolve(
        const QVector<QVector<QPair<int, double>>>& U,
        const QVector<double>& b) const;
};
