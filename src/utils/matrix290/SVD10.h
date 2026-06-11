/**
 * @file SVD10.h
 * @brief 奇异值分解(单侧Jacobi旋转与扫描收敛高瘦矩阵奇异三元组) — SVD with One-sided Jacobi Rotation and Sweep Convergence for Computing Singular Triplets of Tall-skinny Matrices
 *
 * 功能: 实现奇异值分解(SVD)，采用单侧Jacobi旋转(one-sided Jacobi rotation)
 *       与扫描收敛(sweep convergence)实现高瘦矩阵奇异三元组计算(singular triplets of tall-skinny matrices)。
 *
 * 协作: EigenDecomposition9(特征值分解) / QRDecomposition8(QR分解) / LeastSquares10(最小二乘)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 奇异值分解(单侧Jacobi旋转与扫描收敛高瘦矩阵奇异三元组)
 */
class SVD10 : public QObject {
    Q_OBJECT

public:
    /** @brief SVD result: A = U * Sigma * V^T */
    struct SVDResult {
        QVector<QVector<double>> U;      // m x m (or m x min(m,n))
        QVector<double> singularValues;  // min(m,n)
        QVector<QVector<double>> V;      // n x n (or n x min(m,n))
        int rank = 0;
        double conditionNumber = 0.0;
        int sweeps = 0;
        bool converged = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int rows = 0;
        int cols = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SVD10(QObject *parent = nullptr);
    ~SVD10() override;

    void setMaxSweeps(int sweeps);
    void setTolerance(double tol);

    /** @brief Compute full SVD of matrix */
    SVDResult compute(const QVector<QVector<double>>& matrix);

    /** @brief Compute rank-k truncated SVD */
    SVDResult computeTruncated(const QVector<QVector<double>>& matrix, int k);

    /** @brief Compute effective rank from singular values */
    int estimateRank(const QVector<double>& sv, double threshold = 1e-10) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void svdDone(int m, int n, int rank, double timeMs);

private:
    int m_maxSweeps = 100;
    double m_tol = 1e-12;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute column dot product */
    double colDot(const QVector<QVector<double>>& A, int c1, int c2) const;

    /** @brief Compute column norm */
    double colNorm(const QVector<QVector<double>>& A, int c) const;

    /** @brief Apply Jacobi rotation to columns */
    void applyJacobi(QVector<QVector<double>>& A,
                     QVector<QVector<double>>& V,
                     int p, int q);

    /** @brief One full sweep of one-sided Jacobi */
    bool jacobiSweep(QVector<QVector<double>>& A,
                     QVector<QVector<double>>& V);

    /** @brief Extract U from A columns after Jacobi */
    void extractU(QVector<QVector<double>>& A,
                  QVector<double>& sigma,
                  QVector<QVector<double>>& U);

    /** @brief Initialize V as identity */
    QVector<QVector<double>> identityMatrix(int n) const;

    /** @brief Transpose matrix */
    QVector<QVector<double>> transpose(const QVector<QVector<double>>& M) const;
};
