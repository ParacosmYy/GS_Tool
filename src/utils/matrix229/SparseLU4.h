/**
 * @file SparseLU4.h
 * @brief 稀疏LU分解(近似最小填充排序+层次调度ILU阈值丢弃) — Sparse LU with Approximate Minimum Fill Ordering and Level-Scheduled ILU with Threshold Dropping
 *
 * 功能: 实现稀疏矩阵LU分解，采用近似最小填充(AMF)排序减少fill-in，
 *       层次调度ILU预条件器带阈值丢弃控制内存开销。
 *
 * 协作: SparseMatrix8(稀疏矩阵) / ConjugateGradient6(共轭梯度) / GMRES5(GMRES)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QMap>

/**
 * @brief 稀疏LU分解(AMF排序+ILU阈值丢弃)
 */
class SparseLU4 : public QObject {
    Q_OBJECT

public:
    /** @brief Sparse matrix entry */
    struct SparseEntry {
        int row = 0;
        int col = 0;
        double value = 0.0;
    };

    /** @brief Decomposition result */
    struct LUResult {
        QVector<SparseEntry> L;
        QVector<SparseEntry> U;
        QVector<int> permutation;
        int fillInCount = 0;
        double dropCount = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int nnzOriginal = 0;
        int nnzL = 0;
        int nnzU = 0;
        int fillInCount = 0;
        int droplets = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SparseLU4(QObject *parent = nullptr);
    ~SparseLU4() override;

    /** @brief Set drop tolerance for ILU */
    void setParameters(double dropTolerance = 1e-4, int maxFill = -1);

    /** @brief Factorize sparse matrix given as COO entries */
    LUResult factorize(const QVector<SparseEntry>& entries, int n);

    /** @brief Solve Ax=b using factorized LU */
    QVector<double> solve(const LUResult& lu, const QVector<double>& b) const;

    /** @brief Approximate minimum fill ordering */
    QVector<int> amfOrdering(const QVector<SparseEntry>& entries, int n) const;

    /** @brief ILU(0) preconditioner with threshold dropping */
    LUResult iluThreshold(const QVector<SparseEntry>& entries, int n);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void factorizationCompleted(int n, int fillIn, double timeMs);

private:
    double m_dropTolerance = 1e-4;
    int m_maxFill = -1;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Convert COO to CSC (compressed sparse column) */
    void cooToCSC(const QVector<SparseEntry>& entries, int n,
                   QVector<int>& colPtr, QVector<int>& rowIdx,
                   QVector<double>& values) const;

    /** @brief Permute matrix rows and columns */
    QVector<SparseEntry> permute(const QVector<SparseEntry>& entries,
                                   const QVector<int>& perm) const;

    /** @brief Symbolic factorization to predict fill pattern */
    QVector<QVector<int>> symbolicFactor(int n,
        const QVector<int>& colPtr, const QVector<int>& rowIdx) const;

    /** @brief Triangular solve (lower) */
    QVector<double> forwardSolve(const QVector<SparseEntry>& L,
                                   int n, const QVector<double>& b) const;

    /** @brief Triangular solve (upper) */
    QVector<double> backwardSolve(const QVector<SparseEntry>& U,
                                    int n, const QVector<double>& y) const;
};
