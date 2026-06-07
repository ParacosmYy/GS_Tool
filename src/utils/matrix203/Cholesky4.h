/**
 * @file Cholesky4.h
 * @brief Cholesky分解(超节点分块因子分解+缓存无关面板更新) — Cholesky with Supernodal Blocked Factorization and Cache-Oblivious Panel Update
 *
 * 功能: 实现Cholesky分解，支持超节点分块因子分解、
 *       缓存无关面板更新策略和分块三角求解。
 *
 * 协作: QR3(QR分解) / SVD2(SVD分解) / LU2(LU分解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Cholesky分解(超节点分块+缓存无关面板更新)
 */
class Cholesky4 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalDecompositions = 0;
        int matrixSize = 0;
        double determinant = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Cholesky4(QObject *parent = nullptr);
    ~Cholesky4() override;

    void setBlockSize(int bs);

    /** @brief Decompose symmetric positive-definite matrix A into L*L^T */
    bool decompose(const QVector<QVector<double>>& A);

    /** @brief Solve L*L^T * x = b via forward/back substitution */
    QVector<double> solve(const QVector<double>& b) const;

    /** @brief Solve L*L^T * X = B for multiple right-hand sides */
    QVector<QVector<double>> solveMulti(const QVector<QVector<double>>& B) const;

    /** @brief Compute log determinant of A */
    double logDeterminant() const;

    /** @brief Compute inverse of A using Cholesky factor */
    QVector<QVector<double>> inverse() const;

    /** @brief Get lower triangular factor L */
    QVector<QVector<double>> factorL() const;

    /** @brief Supernodal blocked panel update */
    void supernodalPanelUpdate(QVector<QVector<double>>& L, int k, int bs);

    /** @brief Cache-oblivious panel update via recursion */
    void cacheObliviousUpdate(QVector<QVector<double>>& L,
                              int rowStart, int rowEnd,
                              int colStart, int colEnd);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decompositionCompleted(int size, double det, double timeMs);

private:
    int m_blockSize = 64;

    int m_n = 0;
    QVector<QVector<double>> m_L;  // lower triangular factor

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Panel factorization: Cholesky on a block column */
    void factorPanel(QVector<QVector<double>>& L, int colStart, int colEnd);

    /** @brief Triangular solve: update trailing panel */
    void triangularSolve(QVector<QVector<double>>& L,
                         int panelStart, int panelEnd,
                         int updateStart, int updateEnd);

    /** @brief Schur complement update */
    void schurUpdate(QVector<QVector<double>>& L,
                     int panelStart, int panelEnd,
                     int updateStart, int updateEnd);
};
