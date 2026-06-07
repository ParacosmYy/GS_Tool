/**
 * @file SparseLU3.h
 * @brief 稀疏LU分解(贪心近似最小度排序+层级ILU丢弃容差) — Sparse LU with Greedy Approximate Minimum Degree Ordering and Level-Based ILU Drop Tolerance
 *
 * 功能: 实现稀疏LU分解，支持贪心近似最小度排序、
 *       层级ILU丢弃容差和稀疏三角求解。
 *
 * 协作: SparseMatrix3(稀疏矩阵) / IterativeSolver5(迭代求解器) / Preconditioner4(预条件)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 稀疏LU分解(贪心近似最小度排序+层级ILU丢弃容差)
 */
class SparseLU3 : public QObject {
    Q_OBJECT

public:
    /** @brief Sparse entry (column, value) */
    struct Entry {
        int col;
        double val;
        int level;  // Fill level for ILU drop tolerance
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int nnzOriginal = 0;
        int nnzL = 0;
        int nnzU = 0;
        int fillLevel = 0;
        double dropTolerance = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SparseLU3(QObject *parent = nullptr);
    ~SparseLU3() override;

    /** @brief Set ILU fill level limit */
    void setFillLevel(int level);

    /** @brief Set drop tolerance for small entries */
    void setDropTolerance(double tol);

    /** @brief Factorize sparse matrix (CRS format) */
    bool factorize(int n, const QVector<int>& rowPtr,
                   const QVector<int>& colIdx,
                   const QVector<double>& values);

    /** @brief Solve L*U*x = b */
    QVector<double> solve(const QVector<double>& b) const;

    /** @brief Solve L*y = b (forward substitution) */
    QVector<double> solveLower(const QVector<double>& b) const;

    /** @brief Solve U*x = y (back substitution) */
    QVector<double> solveUpper(const QVector<double>& y) const;

    /** @brief Compute approximate minimum degree ordering */
    QVector<int> approxMinDegreeOrder(int n,
                                       const QVector<int>& rowPtr,
                                       const QVector<int>& colIdx) const;

    /** @brief Get permutation vector */
    QVector<int> permutation() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void factorizationCompleted(int nnzL, int nnzU, double timeMs);

private:
    int m_n = 0;
    int m_fillLevel = 5;
    double m_dropTol = 1e-6;

    // L and U stored as sparse rows
    QVector<QVector<Entry>> m_L;
    QVector<QVector<Entry>> m_U;

    // Permutation and inverse permutation
    QVector<int> m_perm;
    QVector<int> m_invPerm;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Apply permutation to CRS matrix */
    void permuteMatrix(int n, const QVector<int>& rowPtr,
                       const QVector<int>& colIdx,
                       const QVector<double>& values);

    /** @brief Eliminate one row/column in LU */
    void eliminateRow(int k);
};
