/**
 * @file SparseQR2.h
 * @brief 稀疏QR分解(超节点组装+列合并树缓存优化) — Sparse QR Factorization with Supernodal Assembly and Column Merge Tree for Cache Efficiency
 *
 * 功能: 实现稀疏QR分解，支持超节点组装、
 *       列合并树排序和缓存友好的Householder分解。
 *
 * 协作: SparseLU4(稀疏LU) / Cholesky5(Cholesky分解) / LeastSquares5(最小二乘)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 稀疏QR(超节点组装+列合并树)
 */
class SparseQR2 : public QObject {
    Q_OBJECT

public:
    /** @brief Sparse matrix entry (row, col, value) */
    using SparseEntry = QPair<QPair<int, int>, double>;

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalFactorizations = 0;
        int numRows = 0;
        int numCols = 0;
        int numNonZeros = 0;
        int numSupernodes = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SparseQR2(QObject *parent = nullptr);
    ~SparseQR2() override;

    /** @brief Factorize sparse matrix */
    void factorize(int rows, int cols, const QVector<SparseEntry>& entries);

    /** @brief Solve Ax = b using QR factors */
    QVector<double> solve(const QVector<double>& b) const;

    /** @brief Compute least-squares solution for overdetermined system */
    QVector<double> leastSquares(const QVector<double>& b) const;

    /** @brief Get R matrix entries */
    QVector<SparseEntry> rMatrix() const;

    /** @brief Get column merge tree (parent array) */
    QVector<int> columnMergeTree() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void factorizationCompleted(int rows, int cols, int nnz, double timeMs);

private:
    int m_rows = 0;
    int m_cols = 0;

    // Compressed column storage for R
    QVector<int> m_colPtr;
    QVector<int> m_rowIdx;
    QVector<double> m_values;

    // Householder vectors (V), tau coefficients
    QVector<QVector<double>> m_householder;
    QVector<double> m_tau;

    // Supernodal structure
    QVector<int> m_superParent;    // column merge tree
    QVector<QVector<int>> m_supernodes;

    // Permutation
    QVector<int> m_colPerm;
    QVector<int> m_invColPerm;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build column merge tree (elimination tree) */
    QVector<int> buildMergeTree(int cols,
                                const QVector<SparseEntry>& entries) const;

    /** @brief Identify supernodes in merge tree */
    QVector<QVector<int>> identifySupernodes(
        const QVector<int>& parent, int cols) const;

    /** @brief Assemble supernodal block and apply Householder */
    void processSupernode(int sn,
                          QVector<QVector<double>>& frontal);

    /** @brief Apply Householder reflections to vector */
    QVector<double> applyHouseholders(const QVector<double>& x) const;

    /** @brief Back-substitution on upper triangular R */
    QVector<double> backSolve(const QVector<double>& y) const;
};
