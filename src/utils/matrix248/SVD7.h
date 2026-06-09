/**
 * @file SVD7.h
 * @brief 奇异值分解(Golub-Kahan双对角化+隐式零移位QR迭代) — SVD with Golub-Kahan Bidiagonalization and Implicit Zero-Shift QR Iteration for Singular Value Decomposition
 *
 * 功能: 实现奇异值分解(Singular Value Decomposition)，通过Golub-Kahan双对角化
 *       (Golub-Kahan bidiagonalization)将矩阵化为双对角形式，再用隐式零移位QR
 *       (implicit zero-shift QR)迭代计算奇异值和奇异向量。
 *
 * 协作: EigenDecomp8(特征分解) / QRDecomp6(QR分解) / LeastSquares9(最小二乘)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 奇异值分解(Golub-Kahan双对角化+隐式零移位QR迭代)
 */
class SVD7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numRows = 0;
        int numCols = 0;
        int rank = 0;
        double conditionNumber = 0.0;
        int iterationsUsed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SVD7(QObject *parent = nullptr);
    ~SVD7() override;

    /** @brief Set maximum QR iterations */
    void setMaxIterations(int iters);

    /** @brief Set convergence tolerance */
    void setTolerance(double tol);

    /** @brief Compute SVD of matrix A (m×n), stores U, S, V^T */
    void compute(const QVector<QVector<double>>& A);

    /** @brief Get left singular vectors U (m×m) */
    QVector<QVector<double>> matrixU() const;

    /** @brief Get singular values σ_1..σ_r */
    QVector<double> singularValues() const;

    /** @brief Get right singular vectors V (n×n) */
    QVector<QVector<double>> matrixV() const;

    /** @brief Reconstruct matrix from U·diag(S)·V^T */
    QVector<QVector<double>> reconstruct() const;

    /** @brief Compute pseudo-inverse A⁺ = V·diag(1/σ)·U^T */
    QVector<QVector<double>> pseudoInverse() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decompositionCompleted(int rank, double conditionNum, double timeMs);

private:
    int m_maxIter = 200;
    double m_tol = 1e-12;
    int m_m = 0, m_n = 0;

    QVector<QVector<double>> m_U;
    QVector<double> m_S;
    QVector<QVector<double>> m_V;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Golub-Kahan bidiagonalization: A = U₁·B·V₁^T */
    void bidiagonalize(QVector<QVector<double>>& B,
                        QVector<QVector<double>>& U,
                        QVector<QVector<double>>& V);

    /** @brief Householder reflection to zero out below-diagonal elements */
    void householderCol(QVector<QVector<double>>& M, int col, int startRow,
                        QVector<double>& v, double& beta);

    /** @brief Householder reflection to zero out right-of-diagonal elements */
    void householderRow(QVector<QVector<double>>& M, int row, int startCol,
                        QVector<double>& v, double& beta);

    /** @brief Apply Householder: M = (I - β·vv^T)·M */
    void applyHouseholderLeft(QVector<QVector<double>>& M, const QVector<double>& v,
                               double beta, int col);

    /** @brief Apply Householder: M = M·(I - β·vv^T) */
    void applyHouseholderRight(QVector<QVector<double>>& M, const QVector<double>& v,
                                double beta, int row);

    /** @brief Implicit zero-shift QR iteration on bidiagonal B */
    void implicitQRSweep(QVector<QVector<double>>& B,
                          QVector<QVector<double>>& U,
                          QVector<QVector<double>>& V);

    /** @brief Givens rotation to zero element */
    void givensRotation(double a, double b, double& c, double& s);

    /** @brief Identity matrix of size n */
    static QVector<QVector<double>> identity(int n);
};
