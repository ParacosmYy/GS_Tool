/**
 * @file SchurDecomposition8.h
 * @brief Schur分解(Francis双步隐式QR+激进早期收缩特征值簇) — Schur Decomposition with Francis Double-Shift Implicit QR and Aggressive Early Deflation for Eigenvalue Clusters
 *
 * 功能: 实现Schur分解(Schur decomposition)，采用Francis双步隐式QR算法(Francis double-shift
 *       implicit QR)，并使用激进早期收缩(aggressive early deflation)策略加速特征值簇
 *       (eigenvalue clusters)的收敛。
 *
 * 协作: EigenSolver9(特征值求解) / SvdDecomposition8(SVD分解) / QrDecomposition9(QR分解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Schur分解(Francis双步隐式QR+激进早期收缩特征值簇)
 */
class SchurDecomposition8 : public QObject {
    Q_OBJECT

public:
    /** @brief Decomposition result */
    struct Result {
        QVector<QVector<double>> T;   // Upper quasi-triangular Schur form
        QVector<QVector<double>> Q;   // Unitary transformation matrix
        QVector<double> eigenvalues;   // Eigenvalues (real or complex pairs)
        int iterations = 0;
        bool converged = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int totalIterations = 0;
        int numDeflations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SchurDecomposition8(QObject *parent = nullptr);
    ~SchurDecomposition8() override;

    /** @brief Set maximum QR iterations */
    void setMaxIterations(int maxIter);

    /** @brief Set convergence tolerance */
    void setTolerance(double tol);

    /** @brief Compute Schur decomposition of matrix A */
    Result decompose(const QVector<QVector<double>>& A);

    /** @brief Extract eigenvalues from quasi-triangular form */
    QVector<double> extractEigenvalues(const QVector<QVector<double>>& T) const;

    /** @brief Get matrix dimension */
    int dimension() const { return m_n; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void iterationCompleted(int iter, double offDiagNorm);
    void deflationOccurred(int blockStart, int blockSize);
    void decompositionCompleted(int iterations, double timeMs);

private:
    int m_n = 0;
    int m_maxIter = 300;
    double m_tol = 1e-12;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Householder reduction to upper Hessenberg form */
    void hessenbergReduce(QVector<QVector<double>>& A,
                           QVector<QVector<double>>& Q) const;

    /** @brief Francis double-shift implicit QR step */
    void francisDoubleShift(QVector<QVector<double>>& T, int lo, int hi,
                             QVector<QVector<double>>& Q) const;

    /** @brief Aggressive early deflation */
    int aggressiveDeflation(QVector<QVector<double>>& T, int lo, int hi) const;

    /** @brief Check for deflation (subdiagonal element near zero) */
    int findDeflation(const QVector<QVector<double>>& T, int lo, int hi) const;

    /** @brief Compute off-diagonal norm */
    double offDiagonalNorm(const QVector<QVector<double>>& T, int lo, int hi) const;

    /** @brief Householder reflection */
    void householder(double& x, double& y, double& cosVal, double& sinVal) const;
};
