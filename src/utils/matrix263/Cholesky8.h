/**
 * @file Cholesky8.h
 * @brief Cholesky分解(稀疏符号分析+超节点块因子化缓存优化) — Cholesky with Sparse Symbolic Analysis and Supernodal Block Factorization with Dense Kernel for Cache Efficiency
 *
 * 功能: 实现稀疏Cholesky分解(Cholesky decomposition)，采用稀疏符号
 *       分析(sparse symbolic analysis)确定填充模式，超节点块因子化
 *       (supernodal block factorization)配合密集内核(dense kernel)
 *       实现缓存高效(cache efficiency)计算。
 *
 * 协作: LDL7(LDL分解) / SparseLU6(稀疏LU) / QRDecomp5(QR分解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Cholesky分解(稀疏符号分析+超节点块因子化缓存优化)
 */
class Cholesky8 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int numNonZeros = 0;
        int numSupernodes = 0;
        double factorizationTimeMs = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Supernode descriptor */
    struct Supernode {
        int startRow = 0;
        int endRow = 0;       // Exclusive
        QVector<int> columnIndices;
    };

    explicit Cholesky8(QObject *parent = nullptr);
    ~Cholesky8() override;

    /** @brief Set matrix in compressed column storage */
    void setMatrix(int n, const QVector<int>& colPtr,
                   const QVector<int>& rowIdx,
                   const QVector<double>& values);

    /** @brief Perform symbolic analysis to determine fill pattern */
    bool symbolicAnalysis();

    /** @brief Perform numerical factorization (after symbolic) */
    bool factorize();

    /** @brief Solve L*x = b (forward substitution) */
    QVector<double> solveLower(const QVector<double>& b) const;

    /** @brief Solve L'*x = b (backward substitution) */
    QVector<double> solveUpper(const QVector<double>& b) const;

    /** @brief Full solve: A*x = b */
    QVector<double> solve(const QVector<double>& b);

    /** @brief Get supernodes */
    QVector<Supernode> supernodes() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void factorizationCompleted(int size, int supernodes, double timeMs);
    void solveCompleted(int size, double timeMs);

private:
    int m_n = 0;

    // Input sparse matrix (CSC format)
    QVector<int> m_colPtr;
    QVector<int> m_rowIdx;
    QVector<double> m_values;

    // Factor L (CSC format)
    QVector<int> m_LcolPtr;
    QVector<int> m_LrowIdx;
    QVector<double> m_Lvalues;

    // Supernodal structure
    QVector<Supernode> m_supernodes;
    QVector<int> m_superMembership;   // Row -> supernode index

    bool m_symbolicDone = false;
    bool m_factorized = false;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute elimination tree */
    QVector<int> eliminationTree() const;

    /** @brief Post-order traversal of elimination tree */
    QVector<int> postOrder(const QVector<int>& parent) const;

    /** @brief Identify supernodes from elimination tree */
    void identifySupernodes(const QVector<int>& parent, const QVector<int>& post);

    /** @brief Supernodal factorization with dense kernel */
    void supernodalFactorize();

    /** @brief Dense Cholesky for supernode block */
    void denseCholesky(QVector<double>& block, int blockSize) const;
};
