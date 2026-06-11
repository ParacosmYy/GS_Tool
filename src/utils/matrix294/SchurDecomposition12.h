/**
 * @file SchurDecomposition12.h
 * @brief Schur分解(多 shifts QR迭代与积极提前收缩计算大型稠密矩阵实Schur形式) — Schur Decomposition with Multishift QR Iteration and Aggressive Early Deflation for Computing Real Schur Form of Large Dense Matrices
 *
 * 功能: 实现Schur分解(Schur decomposition)，采用多 shifts QR迭代(multishift QR iteration)
 *       与积极提前收缩(aggressive early deflation)计算大型稠密矩阵实Schur形式(real Schur form)。
 *
 * 协作: EigenDecomposition14(特征值分解) / QRDecomposition13(QR分解) / Svd14(奇异值分解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Schur分解(多 shifts QR迭代与积极提前收缩计算大型稠密矩阵实Schur形式)
 */
class SchurDecomposition12 : public QObject {
    Q_OBJECT

public:
    /** @brief Decomposition result */
    struct SchurResult {
        QVector<QVector<double>> T;     // Upper quasi-triangular Schur form
        QVector<QVector<double>> Q;     // Orthogonal transformation matrix
        QVector<double> eigenvaluesRe;  // Real parts of eigenvalues
        QVector<double> eigenvaluesIm;  // Imaginary parts of eigenvalues
        int n = 0;
        bool converged = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int iterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SchurDecomposition12(QObject *parent = nullptr);
    ~SchurDecomposition12() override;

    void setMaxIterations(int maxIter);
    void setTolerance(double tol);

    /** @brief Compute real Schur decomposition: A = Q T Q^T */
    SchurResult decompose(const QVector<QVector<double>>& matrix);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decompositionDone(int n, int iters, bool converged, double timeMs);

private:
    int m_maxIter = 300;
    double m_tol = 1e-12;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate identity matrix */
    QVector<QVector<double>> identity(int n) const;

    /** @brief Matrix multiply C = A * B */
    QVector<QVector<double>> matMul(const QVector<QVector<double>>& A,
                                     const QVector<QVector<double>>& B) const;

    /** @brief Hessenberg reduction: A = Q H Q^T */
    void hessenbergReduce(QVector<QVector<double>>& H,
                           QVector<QVector<double>>& Q) const;

    /** @brief Single QR step with Wilkinson shift */
    void qrStep(QVector<QVector<double>>& H, int lo, int hi,
                 QVector<QVector<double>>& Q) const;

    /** @brief Multishift QR step with double implicit shift */
    void multishiftQRStep(QVector<QVector<double>>& H, int lo, int hi,
                           QVector<QVector<double>>& Q, int shiftCount) const;

    /** @brief Extract eigenvalues from quasi-triangular form */
    void extractEigenvalues(const QVector<QVector<double>>& T,
                             QVector<double>& re, QVector<double>& im) const;

    /** @brief Apply Givens rotation to matrix */
    void applyGivens(QVector<QVector<double>>& M, int i, int j,
                      double c, double s, bool left) const;

    /** @brief Check for deflation (small subdiagonal) */
    int checkDeflation(const QVector<QVector<double>>& H, int lo, int hi) const;
};
