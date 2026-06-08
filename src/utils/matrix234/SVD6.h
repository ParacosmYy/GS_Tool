/**
 * @file SVD6.h
 * @brief SVD奇异值分解(Golub-Kahan双对角化+隐式零位移QR迭代) — SVD with Golub-Kahan Bidiagonalization and Implicit Zero-shift QR Iteration for Singular Value Computation
 *
 * 功能: 实现奇异值分解(SVD)，使用Golub-Kahan双对角化(Golub-Kahan bidiagonalization)预处理，
 *       再通过隐式零位移QR迭代(implicit zero-shift QR iteration)计算奇异值。
 *
 * 协作: EigenDecomp3(特征值分解) / QRDecomp2(QR分解) / MatrixOps1(矩阵运算)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief SVD奇异值分解(Golub-Kahan双对角化+隐式零位移QR迭代)
 */
class SVD6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixRows = 0;
        int matrixCols = 0;
        int rank = 0;
        double conditionNumber = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SVD6(QObject *parent = nullptr);
    ~SVD6() override;

    /** @brief Compute SVD: A = U * S * V^T */
    bool compute(const QVector<QVector<double>>& A);

    /** @brief Get U matrix (left singular vectors) */
    QVector<QVector<double>> matrixU() const;

    /** @brief Get singular values (diagonal of S) */
    QVector<double> singularValues() const;

    /** @brief Get V^T matrix (right singular vectors transposed) */
    QVector<QVector<double>> matrixVT() const;

    /** @brief Reconstruct matrix from U, S, V^T */
    QVector<QVector<double>> reconstruct(int rank = -1) const;

    /** @brief Compute pseudo-inverse using SVD */
    QVector<QVector<double>> pseudoInverse(double threshold = 1e-10) const;

    /** @brief Estimate numerical rank */
    int estimateRank(double threshold = 1e-10) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void computeCompleted(int rank, double condNumber, double timeMs);

private:
    int m_rows = 0;
    int m_cols = 0;

    QVector<QVector<double>> m_U;
    QVector<double> m_S;
    QVector<QVector<double>> m_VT;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Householder reflection: compute v and beta such that (I - beta*v*v^T)*x = ||x||*e1 */
    void householder(const QVector<double>& x, QVector<double>& v,
                     double& beta) const;

    /** @brief Apply Householder reflector to matrix rows */
    void applyHouseholderLeft(QVector<QVector<double>>& M, int row,
                               int col, const QVector<double>& v,
                               double beta);

    /** @brief Apply Householder reflector to matrix columns */
    void applyHouseholderRight(QVector<QVector<double>>& M, int row,
                                int col, const QVector<double>& v,
                                double beta);

    /** @brief Golub-Kahan bidiagonalization: A -> bidiagonal B */
    void bidiagonalize(QVector<QVector<double>>& B,
                        QVector<QVector<double>>& Q,
                        QVector<QVector<double>>& P);

    /** @brief Implicit zero-shift QR iteration on bidiagonal */
    void implicitQRStep(QVector<double>& diagonal,
                         QVector<double>& superDiag,
                         QVector<QVector<double>>& V,
                         int lo, int hi);

    /** @brief Golub-Kahan SVD step: compute rotation */
    void gkSVDStep(const QVector<double>& d, const QVector<double>& e,
                    int lo, int hi, double& shift,
                    double& cosL, double& sinL,
                    double& cosR, double& sinR) const;
};
