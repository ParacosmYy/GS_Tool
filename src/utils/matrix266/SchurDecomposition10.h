/**
 * @file SchurDecomposition10.h
 * @brief Schur分解(Francis双移QR迭代隐式Wilkinson移位稳定实Schur形式) — Schur Decomposition with Francis Double-shift QR Iteration and Implicit Wilkinson-type Shift for Stable Real Schur Form
 *
 * 功能: 实现Schur分解(Schur decomposition)，采用Francis双移QR迭代(Francis
 *       double-shift QR iteration)和隐式Wilkinson型移位(implicit Wilkinson-type
 *       shift)计算稳定实Schur形式(stable real Schur form)。
 *
 * 协作: EigenDecomposition10(特征值分解) / QrDecomposition10(QR分解) / SvdDecomposition10(SVD)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Schur分解(Francis双移QR迭代隐式Wilkinson移位稳定实Schur形式)
 */
class SchurDecomposition10 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int numIterations = 0;
        int numDoubleShifts = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Decomposition result */
    struct Result {
        QVector<QVector<double>> T;     // quasi-upper triangular (Schur form)
        QVector<QVector<double>> Q;     // orthogonal transformation matrix
        QVector<double> eigenvalues;    // real and complex conjugate pairs
        int iterations = 0;
        bool converged = false;
    };

    explicit SchurDecomposition10(QObject *parent = nullptr);
    ~SchurDecomposition10() override;

    /** @brief Set max iterations for QR convergence */
    void setMaxIterations(int maxIter);

    /** @brief Set convergence tolerance */
    void setTolerance(double tol);

    /** @brief Compute real Schur decomposition A = Q * T * Q^T */
    Result decompose(const QVector<QVector<double>>& matrix);

    /** @brief Extract eigenvalues from quasi-triangular T */
    QVector<double> extractEigenvalues(const QVector<QVector<double>>& T) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decompositionCompleted(int size, int iterations, bool converged, double timeMs);

private:
    int m_maxIter = 1000;
    double m_tol = 1e-12;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Create identity matrix n x n */
    QVector<QVector<double>> identity(int n) const;

    /** @brief Matrix multiply C = A * B */
    QVector<QVector<double>> matMul(const QVector<QVector<double>>& A,
                                      const QVector<QVector<double>>& B) const;

    /** @brief Matrix transpose */
    QVector<QVector<double>> matTranspose(const QVector<QVector<double>>& A) const;

    /** @brief Francis double-shift QR step on submatrix [ilo..ihi] */
    void francisDoubleShift(QVector<QVector<double>>& H, QVector<QVector<double>>& Q,
                            int ilo, int ihi);

    /** @brief Wilkinson shift: compute eigenvalues of 2x2 bottom block */
    void wilkinsonShift2x2(const QVector<QVector<double>>& H, int n,
                           double& s1, double& s2) const;

    /** @brief Reduce to upper Hessenberg form via Householder */
    void hessenbergReduce(QVector<QVector<double>>& H, QVector<QVector<double>>& Q);

    /** @brief Check if subdiagonal element is negligible */
    bool isNegligible(double val, double ref) const;

    /** @brief Householder reflection: apply from left */
    void householderLeft(QVector<QVector<double>>& A, int col, int rowStart, int rowEnd);

    /** @brief Householder reflection: apply from right */
    void householderRight(QVector<QVector<double>>& A, int col, int rowStart, int rowEnd);
};
