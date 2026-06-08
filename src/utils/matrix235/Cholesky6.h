/**
 * @file Cholesky6.h
 * @brief Cholesky分解(多重前沿装配树+并行前沿矩阵因子分解) — Cholesky with Multifrontal Assembly Tree and Parallel Frontal Matrix Factorization for Sparse Systems
 *
 * 功能: 实现稀疏Cholesky分解，采用多重前沿装配树(multifrontal assembly tree)方法，
 *       支持并行前沿矩阵因子分解(parallel frontal factorization)以加速大规模稀疏线性系统求解。
 *
 * 协作: SparseMatrix8(稀疏矩阵) / ConjugateGradient5(共轭梯度) / LDLT7(LDLT分解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Cholesky分解(多重前沿装配树+并行前沿矩阵因子分解)
 */
class Cholesky6 : public QObject {
    Q_OBJECT

public:
    /** @brief Sparse matrix entry (row, col, value) */
    struct SparseEntry {
        int row;
        int col;
        double value;
    };

    /** @brief Frontal matrix in the assembly tree */
    struct FrontalMatrix {
        int nodeId = -1;
        QVector<int> rowIndices;
        QVector<int> colIndices;
        QVector<double> frontalData;  // packed dense frontal
        int pivotSize = 0;
        QVector<int> children;
        int parent = -1;
        bool factorized = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int numNonZeros = 0;
        int numFrontals = 0;
        int treeHeight = 0;
        double flops = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Cholesky6(QObject *parent = nullptr);
    ~Cholesky6() override;

    /** @brief Set matrix dimension */
    void setDimension(int n);

    /** @brief Add entries to the sparse matrix (symmetric lower triangle) */
    void addEntry(int row, int col, double value);

    /** @brief Perform Cholesky factorization (multifrontal) */
    bool factorize();

    /** @brief Solve L*x = b (forward substitution) */
    QVector<double> solveForward(const QVector<double>& b) const;

    /** @brief Solve L^T*x = b (backward substitution) */
    QVector<double> solveBackward(const QVector<double>& b) const;

    /** @brief Solve A*x = b via forward+backward */
    QVector<double> solve(const QVector<double>& b) const;

    /** @brief Get the assembly tree structure */
    QVector<FrontalMatrix> assemblyTree() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void factorizeCompleted(int frontals, double flops, double timeMs);
    void frontalFactorized(int nodeId, int pivotSize);

private:
    int m_n = 0;
    QVector<SparseEntry> m_entries;

    // Assembly tree
    QVector<FrontalMatrix> m_frontals;
    int m_rootFrontal = -1;

    // Factor L stored in compressed column form
    QVector<QVector<QPair<int, double>>> m_factorL;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build elimination tree from sparse pattern */
    QVector<int> buildEliminationTree() const;

    /** @brief Build assembly tree from elimination tree via post-ordering */
    void buildAssemblyTree(const QVector<int>& elimTree);

    /** @brief Assemble frontal matrix from children and original entries */
    void assembleFrontal(int nodeId);

    /** @brief Factorize a single frontal matrix (dense Cholesky) */
    void factorizeFrontal(int nodeId);

    /** @brief Parallel factorization of independent frontals */
    void factorizeParallel();

    /** @brief Extract factor L from factorized frontals */
    void extractFactorL();

    /** @brief Column approximate minimum degree ordering */
    QVector<int> colamdOrdering() const;
};
