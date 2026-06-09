/**
 * @file SVD8.h
 * @brief 奇异值分解(随机化范围搜索+幂迭代截断低秩近似) — SVD with Randomized Range Finder and Power Iteration for Truncated Low-rank Approximation of Large Matrices
 *
 * 功能: 实现随机化奇异值分解(SVD)，采用随机化范围搜索(randomized
 *       range finder)与幂迭代(power iteration)增强精度，用于大型
 *       矩阵的截断低秩近似(truncated low-rank approximation)。
 *
 * 协作: QR8(QR分解) / EigenSolver7(特征值求解) / GaussElim5(高斯消元)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 奇异值分解(随机化范围搜索+幂迭代截断低秩近似)
 */
class SVD8 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int inputRows = 0;
        int inputCols = 0;
        int targetRank = 0;
        int powerIterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SVD8(QObject *parent = nullptr);
    ~SVD8() override;

    /** @brief Set target rank for truncated SVD */
    void setTargetRank(int rank);

    /** @brief Set number of power iterations for accuracy */
    void setPowerIterations(int iters);

    /** @brief Set oversampling parameter */
    void setOversampling(int p);

    /** @brief Compute truncated SVD of matrix A (rows x cols) stored row-major */
    bool compute(const QVector<QVector<double>>& A);

    /** @brief Get left singular vectors (columns of U) */
    QVector<QVector<double>> matrixU() const;

    /** @brief Get singular values */
    QVector<double> singularValues() const;

    /** @brief Get right singular vectors (rows of V^T) */
    QVector<QVector<double>> matrixVt() const;

    /** @brief Reconstruct low-rank approximation */
    QVector<QVector<double>> reconstruct() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void svdCompleted(int rank, double largestSV, double timeMs);

private:
    int m_targetRank = 10;
    int m_powerIters = 2;
    int m_oversampling = 5;
    int m_rows = 0;
    int m_cols = 0;

    QVector<QVector<double>> m_U;     // m_rows x m_rank
    QVector<double> m_S;              // m_rank
    QVector<QVector<double>> m_Vt;    // m_rank x m_cols

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Generate random Gaussian matrix */
    QVector<QVector<double>> randomGaussian(int rows, int cols) const;

    /** @brief Matrix multiply C = A * B^T */
    QVector<QVector<double>> matMul(const QVector<QVector<double>>& A,
                                     const QVector<QVector<double>>& B) const;

    /** @brief QR decomposition (modified Gram-Schmidt) */
    void qrDecompose(const QVector<QVector<double>>& A,
                     QVector<QVector<double>>& Q,
                     QVector<QVector<double>>& R) const;

    /** @brief Randomized range finder */
    QVector<QVector<double>> rangeFinder(const QVector<QVector<double>>& A, int rank) const;

    /** @brief Compute eigenvalues of symmetric matrix (Jacobi) */
    void jacobiEigen(const QVector<QVector<double>>& S,
                     QVector<double>& eigenvalues,
                     QVector<QVector<double>>& eigenvectors) const;
};
