/**
 * @file Cholesky9.h
 * @brief Cholesky分解(前瞻面板更新与缓存阻塞内存高效对称正定因式分解) — Cholesky with Lookahead Panel Update and Cache-Blocking for Memory-Efficient Dense Symmetric Positive Definite Factorization
 *
 * 功能: 实现Cholesky分解(Cholesky decomposition)，采用前瞻面板更新(lookahead panel update)
 *       与缓存阻塞(cache-blocking)实现内存高效对称正定因式分解(memory-efficient dense SPD factorization)。
 *
 * 协作: QRDecomposition11(QR分解) / SVD10(奇异值分解) / LU12(LU分解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Cholesky分解(前瞻面板更新与缓存阻塞内存高效对称正定因式分解)
 */
class Cholesky9 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int blockSize = 0;
        double determinant = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Cholesky9(QObject *parent = nullptr);
    ~Cholesky9() override;

    /** @brief Set cache block size for tiled factorization */
    void setBlockSize(int bs);

    /** @brief Factor symmetric positive definite matrix A = L * L^T */
    bool factorize(const QVector<QVector<double>>& A);

    /** @brief Solve L * L^T * x = b via forward/back substitution */
    QVector<double> solve(const QVector<double>& b) const;

    /** @brief Compute determinant from diagonal of L */
    double determinant() const;

    /** @brief Get lower triangular factor L */
    QVector<QVector<double>> lowerFactor() const;

    /** @brief Check if last factorization succeeded */
    bool isPositiveDefinite() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void factorizationDone(int size, double determinant, double timeMs);

private:
    int m_blockSize = 64;

    QVector<QVector<double>> m_L;   // Lower triangular factor
    int m_n = 0;
    bool m_isPD = false;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Panel factorization: factor a block column */
    void factorPanel(int j, int panelEnd);

    /** @brief Lookahead: update trailing submatrix using completed panel */
    void updateTrailing(int j, int panelEnd);

    /** @brief Cache-blocked SYRK: C = C - A * A^T for trailing update */
    void syrkUpdate(int j, int panelEnd, int trailingStart);

    /** @brief Forward substitution L * y = b */
    QVector<double> forwardSub(const QVector<double>& b) const;

    /** @brief Backward substitution L^T * x = y */
    QVector<double> backSub(const QVector<double>& y) const;
};
