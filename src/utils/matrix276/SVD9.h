/**
 * @file SVD9.h
 * @brief 奇异值分解(Golub-Kahan双对角化与隐式QR移位稳定分解) — SVD with Golub-Kahan Bidiagonalization and Implicit QR Shift for Stable Singular Value Decomposition
 *
 * 功能: 实现奇异值分解(SVD)，采用Golub-Kahan双对角化(Golub-Kahan bidiagonalization)
 *       与隐式QR移位(implicit QR shift)实现稳定奇异值分解(stable SVD)。
 *
 * 协作: EigenDecomp10(特征分解) / QRDecomposition8(QR分解) / Pseudoinverse7(伪逆)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 奇异值分解(Golub-Kahan双对角化与隐式QR移位稳定分解)
 */
class SVD9 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int rows = 0;
        int cols = 0;
        int rank = 0;
        double conditionNumber = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SVD9(QObject *parent = nullptr);
    ~SVD9() override;

    /** @brief Set convergence tolerance for QR iterations */
    void setTolerance(double tol);

    /** @brief Set maximum QR iterations */
    void setMaxIterations(int iters);

    /** @brief Compute SVD of matrix A (m x n). Returns U, S, V^T */
    void compute(const QVector<QVector<double>>& A);

    /** @brief Get left singular vectors U (m x m) */
    QVector<QVector<double>> matrixU() const;

    /** @brief Get singular values (min(m,n) vector) */
    QVector<double> singularValues() const;

    /** @brief Get right singular vectors V^T (n x n) */
    QVector<QVector<double>> matrixVT() const;

    /** @brief Get effective rank (singular values > tolerance) */
    int rank() const;

    /** @brief Compute pseudoinverse A^+ = V * S^+ * U^T */
    QVector<QVector<double>> pseudoinverse() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decompositionDone(int rows, int cols, int rank, double condNum, double timeMs);

private:
    double m_tol = 1e-12;
    int m_maxIter = 1000;

    int m_rows = 0;
    int m_cols = 0;

    QVector<QVector<double>> m_U;
    QVector<double> m_S;
    QVector<QVector<double>> m_VT;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Householder reflection: compute v such that H*x = ||x||*e1 */
    void householder(const QVector<double>& x, QVector<double>& v, double& beta) const;

    /** @brief Apply Householder from left: A = (I - beta*v*v^T) * A */
    void applyHouseholderLeft(QVector<QVector<double>>& A, const QVector<double>& v,
                               double beta, int rowStart, int colStart) const;

    /** @brief Apply Householder from right: A = A * (I - beta*v*v^T) */
    void applyHouseholderRight(QVector<QVector<double>>& A, const QVector<double>& v,
                                double beta, int rowStart, int colStart) const;

    /** @brief Golub-Kahan bidiagonalization: A = U1 * B * V1^T */
    void bidiagonalize(QVector<QVector<double>>& B,
                        QVector<QVector<double>>& U,
                        QVector<QVector<double>>& VT) const;

    /** @brief Implicit QR shift iteration on bidiagonal matrix */
    void implicitQRShift(QVector<double>& diag, QVector<double>& superdiag,
                          QVector<QVector<double>>& U, QVector<QVector<double>>& VT,
                          int start, int end);

    /** @brief Givens rotation: compute c,s to zero out b */
    void givens(double a, double b, double& c, double& s) const;
};
