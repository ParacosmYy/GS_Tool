/**
 * @file GaussSeidel8.h
 * @brief 高斯-赛德尔迭代(块SOR与双色排序的缓存友好并行稀疏迭代求解) — Gauss-Seidel with Block SOR and Two-color Ordering for Cache-friendly Parallel Sparse Iterative Solving
 *
 * 功能: 实现高斯-赛德尔迭代(Gauss-Seidel iteration)，采用块SOR(block SOR)
 *       与双色排序(two-color ordering)实现缓存友好并行稀疏迭代求解(cache-friendly parallel sparse iterative solving)。
 *
 * 协作: SparseMatrix8(稀疏矩阵) / CG8(共轭梯度) / ILU8(不完全LU分解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 高斯-赛德尔迭代(块SOR与双色排序)
 */
class GaussSeidel8 : public QObject {
    Q_OBJECT

public:
    /** @brief Solver result */
    struct SolveResult {
        QVector<double> solution;
        double residualNorm = 0.0;
        int iterations = 0;
        bool converged = false;
    };

    /** @brief Sparse matrix entry (CSR format) */
    struct SparseEntry {
        int col = -1;
        double value = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int totalIterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GaussSeidel8(QObject *parent = nullptr);
    ~GaussSeidel8() override;

    void setTolerance(double tol);
    void setMaxIterations(int maxIter);
    void setOmega(double omega);      // SOR relaxation factor
    void setBlockSize(int blockSize); // Block size for block SOR

    /** @brief Set sparse matrix in CSR format */
    void setMatrix(int n, const QVector<int>& rowPtr,
                   const QVector<SparseEntry>& entries);

    /** @brief Solve Ax = b */
    SolveResult solve(const QVector<double>& b);

    /** @brief Compute residual r = b - Ax */
    QVector<double> residual(const QVector<double>& x,
                             const QVector<double>& b) const;

    /** @brief Get two-color ordering */
    QVector<int> twoColorOrdering() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void iterationDone(int iter, double residual, double timeMs);
    void solveDone(int n, int iters, bool converged, double timeMs);

private:
    double m_tol = 1e-8;
    int m_maxIter = 1000;
    double m_omega = 1.0;    // 1.0 = standard GS, (0,2) = SOR
    int m_blockSize = 4;
    Stats m_stats;
    double m_timeSum = 0.0;

    int m_n = 0;
    QVector<int> m_rowPtr;
    QVector<SparseEntry> m_entries;

    /** @brief Two-color groups for parallel ordering */
    QVector<int> m_colorGroup[2];

    /** @brief Compute two-color ordering */
    void buildColorOrdering();

    /** @brief Matrix-vector product y = Ax */
    QVector<double> matVec(const QVector<double>& x) const;

    /** @brief Solve a small block (banded subsystem) via direct elimination */
    void solveBlock(QVector<double>& x, const QVector<double>& b,
                    int startRow, int endRow) const;

    /** @brief Compute L2 norm */
    static double normL2(const QVector<double>& v);
};
