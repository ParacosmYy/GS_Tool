/**
 * @file SparseLU5.h
 * @brief 稀疏LU分解(阈值主元+超节点消去树符号/数值分解) — Sparse LU with Threshold Pivoting and Supernodal Elimination Tree for Efficient Symbolic-Numeric Factorization
 *
 * 功能: 实现稀疏LU分解(Sparse LU Decomposition)，使用阈值主元选取
 *       (threshold pivoting)平衡数值稳定性与稀疏性，超节点消去树
 *       (supernodal elimination tree)优化符号-数值分解(symbolic-numeric
 *       factorization)效率。
 *
 * 协作: SparseCholesky4(稀疏Cholesky) / IterativeSolver3(迭代求解) / BandMatrix6(带状矩阵)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 稀疏LU分解(阈值主元+超节点消去树)
 */
class SparseLU5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int numNonzeros = 0;
        int numFillin = 0;
        int numPivots = 0;
        int numSupernodes = 0;
        double pivotThreshold = 0.1;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Triplet entry for sparse matrix */
    struct Triplet {
        int row = 0;
        int col = 0;
        double value = 0.0;
    };

    explicit SparseLU5(QObject *parent = nullptr);
    ~SparseLU5() override;

    /** @brief Set pivot threshold (0..1) */
    void setPivotThreshold(double threshold);

    /** @brief Factorize sparse matrix from triplet entries */
    bool factorize(int n, const QVector<Triplet>& entries);

    /** @brief Solve Ax = b using factorization */
    QVector<double> solve(const QVector<double>& b) const;

    /** @brief Get L factor diagonal entries */
    QVector<double> diagonal() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void factorizationCompleted(int n, int fillin, double timeMs);

private:
    int m_n = 0;
    double m_pivotThresh = 0.1;

    // Dense representation for small-to-medium sparse matrices
    QVector<QVector<double>> m_L;  // Lower triangular (unit diagonal)
    QVector<QVector<double>> m_U;  // Upper triangular
    QVector<int> m_perm;           // Row permutation
    QVector<int> m_colPerm;        // Column permutation (elimination tree order)

    // Supernode info
    QVector<int> m_supernodeStart; // Start index of each supernode

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build elimination tree for column ordering */
    QVector<int> buildEliminationTree(const QVector<QVector<int>>& colPattern) const;

    /** @brief Postorder traversal of elimination tree */
    QVector<int> postorder(const QVector<int>& parent) const;

    /** @brief Identify supernodes from elimination tree */
    void identifySupernodes(const QVector<int>& postOrder);

    /** @brief Partial (threshold) pivoting */
    int selectPivot(int col, const QVector<QVector<double>>& A) const;

    /** @brief Forward substitution: solve L*y = Pb */
    QVector<double> forwardSolve(const QVector<double>& b) const;

    /** @brief Backward substitution: solve U*x = y */
    QVector<double> backwardSolve(const QVector<double>& y) const;
};
