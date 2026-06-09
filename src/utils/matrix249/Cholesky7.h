/**
 * @file Cholesky7.h
 * @brief Cholesky分解(分块算法+分块计算缓存高效正定矩阵分解) — Cholesky with Block Algorithm and Tiled Computation for Cache-Efficient Positive Definite Factorization
 *
 * 功能: 实现Cholesky分解(Cholesky Decomposition)，采用分块算法(block
 *       algorithm)和分块计算(tiled computation)实现缓存高效(cache-efficient)
 *       的正定矩阵(positive definite matrix)分解。
 *
 * 协作: QRDecomposition7(QR分解) / SVD9(奇异值分解) / MatrixPower6(矩阵幂)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Cholesky分解(分块算法+分块计算缓存高效正定矩阵分解)
 */
class Cholesky7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int blockSize = 0;
        bool isPositiveDefinite = false;
        double determinant = 0.0;
        double logDeterminant = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Cholesky7(QObject *parent = nullptr);
    ~Cholesky7() override;

    /** @brief Set block/tile size for cache-efficient computation */
    void setBlockSize(int bs);

    /** @brief Factorize a symmetric positive-definite matrix */
    bool factorize(const QVector<QVector<double>>& A);

    /** @brief Solve Ax = b using existing factorization */
    QVector<double> solve(const QVector<double>& b) const;

    /** @brief Solve AX = B (multiple right-hand sides) */
    QVector<QVector<double>> solveMatrix(const QVector<QVector<double>>& B) const;

    /** @brief Compute inverse of the factorized matrix */
    QVector<QVector<double>> inverse() const;

    /** @brief Get lower triangular factor L where A = L * L^T */
    QVector<QVector<double>> factor() const;

    /** @brief Compute determinant from factorization */
    double determinant() const;

    /** @brief Check if last factorization was successful */
    bool isPositiveDefinite() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void factorizationCompleted(int size, bool success, double timeMs);

private:
    int m_blockSize = 64;

    /** @brief Tiled storage: matrix stored as block tiles */
    QVector<QVector<double>> m_L;  // Lower triangular factor
    int m_n = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Register tiled layout helper */
    struct TileRange {
        int rowStart, rowEnd, colStart, colEnd;
    };

    /** @brief Blocked Cholesky: factorize diagonal block */
    void factorizeDiagBlock(int k);

    /** @brief Blocked Cholesky: update off-diagonal block */
    void updateOffDiagBlock(int k, int j);

    /** @brief Blocked Cholesky: update trailing submatrix */
    void updateTrailingSubmatrix(int k);

    /** @brief Get tile range for block index */
    TileRange tileRange(int blockIdx, int blockSize) const;

    /** @brief Forward substitution L * y = b */
    QVector<double> forwardSub(const QVector<double>& b) const;

    /** @brief Backward substitution L^T * x = y */
    QVector<double> backSub(const QVector<double>& y) const;
};
