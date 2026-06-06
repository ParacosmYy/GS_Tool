/**
 * @file SparseQR.h
 * @brief 稀疏QR分解(列消去树+符号/数值分解+丢弃容差) — Sparse QR Factorization with Column Elimination Tree, Symbolic/Numeric Factorization and Drop Tolerance
 *
 * 功能: 实现稀疏矩阵QR分解，支持列消去树结构分析、符号分解确定非零模式、
 *       数值Householder分解和丢弃容差稀疏保持。
 *
 * 协作: SparseLU4(稀疏LU) / ConjugateGradient4(CG) / BiCGSTAB4(BiCGSTAB)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 稀疏QR分解器(列消去树+符号/数值分解)
 */
class SparseQR : public QObject {
    Q_OBJECT

public:
    /** @brief 稀疏矩阵(CSC格式) */
    struct SparseMatrix {
        int rows = 0;
        int cols = 0;
        QVector<int> colPtr;    ///< Column pointers (size cols+1)
        QVector<int> rowIdx;    ///< Row indices of nonzeros
        QVector<double> values; ///< Nonzero values
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalFactorizations = 0;
        int matrixRows = 0;
        int matrixCols = 0;
        int nnzOriginal = 0;
        int nnzFactor = 0;
        double dropRatio = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SparseQR(QObject *parent = nullptr);
    ~SparseQR() override;

    void setDropTolerance(double tol);

    /** @brief 执行稀疏QR分解 */
    bool factorize(const SparseMatrix& A);

    /** @brief 求解 Ax = b */
    QVector<double> solve(const QVector<double>& b) const;

    /** @brief 构建列消去树 */
    QVector<int> eliminationTree(const SparseMatrix& A) const;

    /** @brief 符号分解: 确定R的非零模式 */
    void symbolicAnalysis(const SparseMatrix& A);

    /** @brief 获取R矩阵 */
    SparseMatrix factorR() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void factorizationCompleted(int rows, int cols, int nnz);

private:
    double m_dropTol = 1e-6;

    // Internal factor storage: R in upper triangular CSC
    SparseMatrix m_R;
    // Householder vectors (compressed)
    QVector<QVector<double>> m_householder;
    QVector<double> m_tau;
    // Column permutation
    QVector<int> m_perm;
    // Elimination tree
    QVector<int> m_etree;

    int m_rows = 0;
    int m_cols = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Convert to dense for small matrices */
    QVector<QVector<double>> toDense(const SparseMatrix& A) const;

    /** @brief Dense Householder QR (fallback for small matrices) */
    void denseQR(QVector<QVector<double>>& A,
                  QVector<double>& tau);

    /** @brief Apply Householder reflection */
    void applyHouseholder(QVector<QVector<double>>& R,
                           const QVector<double>& v, double tau,
                           int col, int startRow);

    /** @brief Apply Q^T to vector */
    QVector<double> applyQTranspose(const QVector<double>& b) const;

    /** @brief Back-substitution with upper triangular R */
    QVector<double> backSolve(const QVector<double>& y) const;
};
