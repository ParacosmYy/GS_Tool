/**
 * @file SVD11.h
 * @brief 奇异值分解(双对角化与隐式QR位移实现高相对精度奇异值计算) — SVD with Bidiagonalization and Implicit QR Shift for Computing Singular Value Decomposition with High Relative Accuracy
 *
 * 功能: 实现奇异值分解(SVD)，采用双对角化(bidiagonalization)
 *       与隐式QR位移(implicit QR shift)实现高相对精度奇异值计算(computing SVD with high relative accuracy)。
 *
 * 协作: EigenDecomposition(特征值分解) / QRDecomposition(QR分解) / Pseudoinverse(伪逆)
 */
#pragma once

#include <QObject>
#include <QVector>

class SVD11 : public QObject {
    Q_OBJECT

public:
    /** @brief SVD decomposition result */
    struct SVDResult {
        QVector<QVector<double>> U;       // m x m orthogonal
        QVector<double> singularValues;   // min(m,n) sorted descending
        QVector<QVector<double>> V;       // n x n orthogonal
        double conditionNumber = 0.0;
        double frobeniusNorm = 0.0;
        int rank = 0;
        int iterations = 0;
        bool converged = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalDecompositions = 0;
        int rows = 0;
        int cols = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SVD11(QObject *parent = nullptr);
    ~SVD11() override;

    void setMaxIterations(int iter);
    void setConvergenceTolerance(double tol);
    void setThreshold(double threshold);

    /** @brief Compute full SVD of matrix A (m x n) */
    SVDResult decompose(const QVector<QVector<double>>& A);

    /** @brief Reconstruct matrix from SVD components */
    QVector<QVector<double>> reconstruct(const SVDResult& svd, int k) const;

    /** @brief Compute pseudoinverse from SVD */
    QVector<QVector<double>> pseudoinverse(const SVDResult& svd) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decompositionDone(int m, int n, int rank, double timeMs);

private:
    int m_maxIter = 300;
    double m_tol = 1e-12;
    double m_threshold = 1e-10;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Householder reflection: compute v, beta such that H = I - beta*v*v^T */
    double householder(QVector<double>& x, QVector<double>& v) const;

    /** @brief Bidiagonalize matrix A -> U*B*V^T */
    void bidiagonalize(QVector<QVector<double>>& B,
                        QVector<QVector<double>>& U,
                        QVector<QVector<double>>& V) const;

    /** @brief Implicit QR shift step on bidiagonal */
    void implicitQRStep(QVector<double>& diagonal, QVector<double>& superDiag,
                         QVector<QVector<double>>& U, QVector<QVector<double>>& V,
                         int lo, int hi) const;

    /** @brief Check convergence of off-diagonal elements */
    bool isConverged(const QVector<double>& superDiag, int lo, int hi) const;

    /** @brief Matrix multiply C = A * B */
    QVector<QVector<double>> matMul(const QVector<QVector<double>>& A,
                                     const QVector<QVector<double>>& B) const;

    /** @brief Apply Givens rotation to rows of matrix */
    void applyGivensLeft(QVector<QVector<double>>& M, int i, int j,
                          double c, double s) const;

    /** @brief Apply Givens rotation to columns of matrix */
    void applyGivensRight(QVector<QVector<double>>& M, int i, int j,
                           double c, double s) const;
};
