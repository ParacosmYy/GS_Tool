/**
 * @file SparseLU7.h
 * @brief 稀疏LU分解(列近似最小度排序与块三角形式的可约稀疏系统) — Sparse LU with Column Approximate Minimum Degree Ordering and Block Triangular Form for Reducible Sparse Systems
 *
 * 功能: 实现稀疏LU分解(sparse LU)，采用列近似最小度排序(column approximate minimum degree ordering)
 *       与块三角形式(block triangular form)实现可约稀疏系统(reducible sparse systems)。
 *
 * 协作: DenseLU6(稠密LU) / Cholesky5(Cholesky分解) / SparseMatrix4(稀疏矩阵)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 稀疏LU分解(列近似最小度排序与块三角形式)
 */
class SparseLU7 : public QObject {
    Q_OBJECT

public:
    /** @brief Sparse matrix entry (COO format) */
    struct SparseEntry {
        int row = 0;
        int col = 0;
        double value = 0.0;
    };

    /** @brief Factorization result */
    struct LUResult {
        QVector<SparseEntry> L;     // Lower triangular (unit diagonal)
        QVector<SparseEntry> U;     // Upper triangular
        QVector<int> rowPerm;       // Row permutation P
        QVector<int> colPerm;       // Column permutation Q
        double determinant = 0.0;
        bool success = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int nnzOriginal = 0;
        int nnzL = 0;
        int nnzU = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SparseLU7(QObject *parent = nullptr);
    ~SparseLU7() override;

    /** @brief Set matrix dimension */
    void setSize(int n);

    /** @brief Set sparse matrix entries (COO format) */
    void setEntries(const QVector<SparseEntry>& entries);

    /** @brief Perform LU factorization with COLAMD ordering */
    LUResult factorize();

    /** @brief Solve Ax = b using computed factorization */
    QVector<double> solve(const QVector<double>& b) const;

    /** @brief Get block triangular form (Dulmage-Mendelsohn decomposition) */
    QVector<QVector<int>> blockTriangularForm() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void factorizeDone(int n, int nnzL, int nnzU, double det, double timeMs);

private:
    int m_n = 0;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compressed sparse entries */
    QVector<SparseEntry> m_entries;

    /** @brief Cached factorization result */
    LUResult m_lastLU;

    /** @brief Dense workspace for factorization */
    QVector<QVector<double>> m_dense;

    /** @brief Convert sparse entries to dense matrix */
    void toDense(const QVector<SparseEntry>& entries, int n);

    /** @brief Column approximate minimum degree ordering */
    QVector<int> colamdOrder(const QVector<QVector<double>>& mat) const;

    /** @brief Compute row permutation via partial pivoting */
    QVector<int> partialPivot(QVector<QVector<double>>& A, int n);

    /** @brief Triangular solve L*y = b */
    QVector<double> forwardSolve(const QVector<QVector<double>>& L,
                                  const QVector<int>& rowP,
                                  const QVector<double>& b) const;

    /** @brief Triangular solve U*x = y */
    QVector<double> backSolve(const QVector<QVector<double>>& U,
                               const QVector<int>& colP,
                               const QVector<double>& y) const;
};
