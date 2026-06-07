/**
 * @file SparseQR3.h
 * @brief 稀疏QR分解(多前沿法+装配树超节点利用) — Sparse QR with Multifrontal Method and Assembly Tree for Supernode Structure Exploitation
 *
 * 功能: 实现稀疏QR分解，支持多前沿方法、装配树构建、
 *       超节点检测和Householder反射因子分解。
 *
 * 协作: SparseLU4(稀疏LU) / Cholesky8(Cholesky) / IterativeSolver6(迭代求解器)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 稀疏QR分解(多前沿法+装配树超节点利用)
 */
class SparseQR3 : public QObject {
    Q_OBJECT

public:
    /** @brief Sparse entry (row, col, value) */
    struct Triplet {
        int row = 0;
        int col = 0;
        double value = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalSolves = 0;
        int matrixRows = 0;
        int matrixCols = 0;
        int supernodeCount = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SparseQR3(QObject *parent = nullptr);
    ~SparseQR3() override;

    /** @brief Factorize sparse matrix from triplets */
    void factorize(const QVector<Triplet>& triplets, int rows, int cols);

    /** @brief Solve Ax = b using QR factors */
    QVector<double> solve(const QVector<double>& b) const;

    /** @brief Build column elimination tree (assembly tree) */
    QVector<int> buildEliminationTree(int cols) const;

    /** @brief Detect supernodes in elimination tree */
    QVector<QVector<int>> detectSupernodes(const QVector<int>& parent) const;

    /** @brief Build frontal matrices for multifrontal method */
    void buildFrontalMatrices(const QVector<int>& parent,
                               const QVector<QVector<int>>& supernodes);

    /** @brief Householder QR on a dense frontal matrix */
    void householderQR(QVector<double>& F, int rows, int cols,
                        QVector<double>& tau) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void factorizationCompleted(int rank, double timeMs);

private:
    int m_rows = 0;
    int m_cols = 0;

    // Compressed column storage
    QVector<int> m_colPtr;
    QVector<int> m_rowIdx;
    QVector<double> m_values;

    // QR factors: R stored in CCS, Householder vectors
    QVector<double> m_R;
    QVector<double> m_tau;          // Householder scalars
    QVector<int> m_permutation;     // column permutation

    // Assembly tree
    QVector<int> m_parent;
    QVector<QVector<int>> m_frontalRows;    // row indices per frontal

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build CCS from triplets */
    void buildCCS(const QVector<Triplet>& triplets);

    /** @brief Post-order the elimination tree */
    QVector<int> postOrder(const QVector<int>& parent) const;
};
