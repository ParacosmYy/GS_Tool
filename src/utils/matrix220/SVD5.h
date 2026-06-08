/**
 * @file SVD5.h
 * @brief 奇异值分解(单侧Jacobi旋转+预条件Golub-Kahan双对角化) — SVD with One-Sided Jacobi Rotation and Preconditioned Golub-Kahan Bidiagonalization
 *
 * 功能: 实现SVD分解，使用单侧Jacobi旋转方法，
 *       结合预条件Golub-Kahan双对角化提升收敛速度。
 *
 * 协作: EigenSolver6(特征值) / QRDecomposition4(QR分解) / LeastSquares7(最小二乘)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 奇异值分解(单侧Jacobi+Golub-Kahan双对角化)
 */
class SVD5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int rows = 0;
        int cols = 0;
        int iterations = 0;
        double conditionNumber = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SVD5(QObject *parent = nullptr);
    ~SVD5() override;

    /** @brief Set convergence parameters */
    void setParameters(int maxIter = 100, double tol = 1e-10);

    /** @brief Compute SVD: A = U * S * V^T */
    void compute(const QVector<QVector<double>>& A);

    /** @brief Get left singular vectors (columns of U) */
    QVector<QVector<double>> matrixU() const;

    /** @brief Get singular values (diagonal of S) */
    QVector<double> singularValues() const;

    /** @brief Get right singular vectors (columns of V) */
    QVector<QVector<double>> matrixV() const;

    /** @brief Reconstruct matrix from SVD components */
    QVector<QVector<double>> reconstruct(int rank = -1) const;

    /** @brief Compute pseudo-inverse via SVD */
    QVector<QVector<double>> pseudoInverse(double threshold = 1e-10) const;

    /** @brief Compute Frobenius norm off-diagonal for convergence check */
    double offDiagonalNorm(const QVector<QVector<double>>& M) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void computationCompleted(int rank, int iterations, double condNum, double timeMs);

private:
    int m_maxIter = 100;
    double m_tol = 1e-10;
    int m_rows = 0;
    int m_cols = 0;

    QVector<QVector<double>> m_U;
    QVector<double> m_S;
    QVector<QVector<double>> m_V;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Golub-Kahan bidiagonalization */
    void bidiagonalize(QVector<QVector<double>>& B,
                       QVector<QVector<double>>& U,
                       QVector<QVector<double>>& V) const;

    /** @brief Apply Householder reflection */
    void applyHouseholder(QVector<QVector<double>>& M, int col,
                          QVector<QVector<double>>& Q, int rowStart) const;

    /** @brief One-sided Jacobi sweep on columns */
    bool jacobiSweep(QVector<QVector<double>>& B,
                     QVector<QVector<double>>& V) const;

    /** @brief Compute 2x2 SVD for Jacobi pair rotation */
    void jacobiPair(int p, int q, const QVector<QVector<double>>& B,
                    double& cs, double& sn) const;
};
