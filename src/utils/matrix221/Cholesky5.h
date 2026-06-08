/**
 * @file Cholesky5.h
 * @brief Cholesky分解(超节点块因子分解+左看面板更新缓存优化) — Cholesky with Supernodal Block Factorization and Left-Looking Panel Update for Cache Efficiency
 *
 * 功能: 实现超节点Cholesky分解，采用左看面板更新策略优化缓存命中率，
 *       支持稠密/带状矩阵分解、自动块大小选择和条件数估计。
 *
 * 协作: Cholesky4(Cholesky分解) / SVD5(奇异值分解) / SparseMatrix5(稀疏矩阵)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Cholesky分解(超节点块因子+左看面板更新)
 */
class Cholesky5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int blockSize = 64;
        int supernodes = 0;
        double conditionEstimate = 0.0;
        double logDet = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Cholesky5(QObject *parent = nullptr);
    ~Cholesky5() override;

    /** @brief Set block size for panel factorization (0 = auto) */
    void setParameters(int blockSize = 0);

    /** @brief Factorize symmetric positive definite matrix */
    bool factorize(const QVector<QVector<double>>& matrix);

    /** @brief Solve Lx = b (forward substitution) */
    QVector<double> solve(const QVector<double>& b) const;

    /** @brief Solve Ax = b using factorization */
    QVector<double> solveSystem(const QVector<double>& b) const;

    /** @brief Compute log determinant */
    double logDeterminant() const;

    /** @brief Get lower triangular factor L */
    QVector<QVector<double>> factorL() const;

    /** @brief Estimate condition number */
    double conditionEstimate() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void factorizationCompleted(int size, int supernodes, double timeMs);

private:
    int m_n = 0;
    int m_blockSize = 64;

    // Lower triangular factor stored column-major
    QVector<QVector<double>> m_L;

    // Supernodal structure: each supernode is a contiguous column range
    QVector<QPair<int, int>> m_supernodes;  // (startCol, endCol)

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Left-looking panel update */
    void leftLookingPanel(int j, int jb);

    /** @brief Factor diagonal block (small dense Cholesky) */
    bool factorDiagonalBlock(int j, int jb);

    /** @brief Update trailing submatrix using panel */
    void updateTrailing(int j, int jb);

    /** @brief Detect supernodal structure from sparsity pattern */
    void detectSupernodes();

    /** @brief Symmetric rank-k update for block */
    void syrk(int row, int col, int k, int bs);
};
