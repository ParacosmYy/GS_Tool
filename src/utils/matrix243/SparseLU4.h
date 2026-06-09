/**
 * @file SparseLU4.h
 * @brief 稀疏LU分解(列近似最小度排序+超节点块三角分解) — Sparse LU with Column Approximate Minimum Degree Ordering and Supernodal Block Triangular Factorization
 *
 * 功能: 实现稀疏LU分解(Sparse LU decomposition)，采用列近似最小度排序(column approximate
 *       minimum degree ordering, COLAMD)减少填充元，利用超节点块三角分解(supernodal block
 *       triangular factorization)将稠密子矩阵作为超节点统一处理，提高分解效率。
 *
 * 协作: SparseCholesky5(稀疏Cholesky) / GMRES6(GMRES) / ConjugateGradient5(共轭梯度)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 稀疏LU分解(列近似最小度排序+超节点块三角分解)
 */
class SparseLU4 : public QObject {
    Q_OBJECT

public:
    /** @brief Sparse matrix entry (COO format) */
    struct Triplet {
        int row = 0;
        int col = 0;
        double val = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int nnzOriginal = 0;
        int nnzFactors = 0;
        int numSupernodes = 0;
        int orderingQuality = 0;
        double fillRatio = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SparseLU4(QObject *parent = nullptr);
    ~SparseLU4() override;

    /** @brief Factorize sparse matrix given in COO triplets */
    bool factorize(int n, const QVector<Triplet>& triplets);

    /** @brief Solve Ax = b after factorization */
    QVector<double> solve(const QVector<double>& b) const;

    /** @brief Get L factor values (CSC: column pointers + row indices + values) */
    void getL(QVector<int>& colPtr, QVector<int>& rowIdx, QVector<double>& vals) const;

    /** @brief Get U factor values (CSC: column pointers + row indices + values) */
    void getU(QVector<int>& colPtr, QVector<int>& rowIdx, QVector<double>& vals) const;

    /** @brief Get column permutation (COLAMD ordering) */
    QVector<int> columnPermutation() const;

    /** @brief Get row permutation (partial pivoting) */
    QVector<int> rowPermutation() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void factorizeCompleted(int n, int nnz, double timeMs);
    void solveCompleted(int n, double timeMs);

private:
    int m_n = 0;

    // L factor: CSC format
    QVector<int> m_lColPtr;
    QVector<int> m_lRowIdx;
    QVector<double> m_lVal;

    // U factor: CSC format
    QVector<int> m_uColPtr;
    QVector<int> m_uRowIdx;
    QVector<double> m_uVal;

    // Permutation vectors
    QVector<int> m_colPerm;   // column ordering (COLAMD)
    QVector<int> m_rowPerm;   // row pivoting order
    QVector<int> m_colInvPerm;
    QVector<int> m_rowInvPerm;

    // Supernode boundaries
    QVector<int> m_supernodes;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief COLAMD-like greedy degree ordering */
    QVector<int> colamdOrder(int n, const QVector<Triplet>& triplets) const;

    /** @brief Build CSC from COO triplets with column permutation */
    void buildCSC(int n, const QVector<Triplet>& triplets,
                  const QVector<int>& perm,
                  QVector<int>& colPtr, QVector<int>& rowIdx,
                  QVector<double>& vals) const;

    /** @brief Detect supernodes in U factor */
    void detectSupernodes();

    /** @brief Triangular solve L*y = b (forward substitution) */
    QVector<double> forwardSolve(const QVector<double>& b) const;

    /** @brief Triangular solve U*x = y (back substitution) */
    QVector<double> backSolve(const QVector<double>& y) const;
};
