/**
 * @file SparseLU6.h
 * @brief 稀疏LU分解(近似最小度排序与超节点块分解结构化稀疏系统) — Sparse LU with Approximate Minimum Degree Ordering and Supernodal Block Factorization for Structured Sparse Systems
 *
 * 功能: 实现稀疏LU分解(Sparse LU)，采用近似最小度排序(Approximate Minimum Degree ordering)
 *       和超节点块分解(Supernodal block factorization)求解结构化稀疏系统(Structured sparse systems)。
 *
 * 协作: SparseCholesky7(稀疏Cholesky) / ConjugateGradient8(共轭梯度) / BiCGSTAB9(BiCGSTAB)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 稀疏LU分解(近似最小度排序与超节点块分解结构化稀疏系统)
 */
class SparseLU6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int nnzOriginal = 0;
        int nnzFactor = 0;
        double fillRatio = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Sparse matrix entry (COO format) */
    struct Triplet {
        int row = 0;
        int col = 0;
        double value = 0.0;
    };

    explicit SparseLU6(QObject *parent = nullptr);
    ~SparseLU6() override;

    /** @brief Set matrix from triplet list and factorize */
    bool factorize(int n, const QVector<Triplet>& entries);

    /** @brief Solve Ax = b after factorization */
    QVector<double> solve(const QVector<double>& b) const;

    /** @brief Compute approximate minimum degree ordering */
    QVector<int> amdOrdering(int n, const QVector<Triplet>& entries) const;

    /** @brief Get L factor (CSC: column pointers, row indices, values) */
    void getLFactors(QVector<int>& colPtr, QVector<int>& rowIdx,
                     QVector<double>& values) const;

    /** @brief Get U factor (CSC: column pointers, row indices, values) */
    void getUFactors(QVector<int>& colPtr, QVector<int>& rowIdx,
                     QVector<double>& values) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void factorizationCompleted(int nnz, double fillRatio, double timeMs);

private:
    int m_n = 0;
    bool m_factored = false;

    // L factor (CSC format)
    QVector<int> m_LcolPtr;
    QVector<int> m_LrowIdx;
    QVector<double> m_Lvals;

    // U factor (CSC format)
    QVector<int> m_UcolPtr;
    QVector<int> m_UrowIdx;
    QVector<double> m_Uvals;

    // Permutation
    QVector<int> m_perm;       // Row/col permutation
    QVector<int> m_invPerm;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build CSC from triplets */
    void buildCSC(int n, const QVector<Triplet>& entries,
                  QVector<int>& colPtr, QVector<int>& rowIdx,
                  QVector<double>& vals) const;

    /** @brief Apply permutation to CSC matrix */
    void permuteCSC(const QVector<int>& perm,
                    const QVector<int>& colPtr,
                    const QVector<int>& rowIdx,
                    const QVector<double>& vals,
                    QVector<int>& outColPtr,
                    QVector<int>& outRowIdx,
                    QVector<double>& outVals) const;

    /** @brief Solve Lx = b (forward substitution) */
    QVector<double> solveL(const QVector<double>& b) const;

    /** @brief Solve Ux = b (backward substitution) */
    QVector<double> solveU(const QVector<double>& b) const;
};
