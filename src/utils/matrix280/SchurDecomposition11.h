/**
 * @file SchurDecomposition11.h
 * @brief Schur分解(激进早期收缩与多位移QR的高性能特征值计算) — Schur Decomposition with Aggressive Early Deflation and Multi-shift QR for High-performance Eigenvalue Computation
 *
 * 功能: 实现Schur分解(Schur decomposition)，采用激进早期收缩(aggressive early deflation)
 *       与多位移QR(multi-shift QR)实现高性能特征值计算(high-performance eigenvalue computation)。
 *
 * 协作: EigenDecomposition10(特征值分解) / SvdDecomposition10(奇异值分解) / QrDecomposition10(QR分解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Schur分解(激进早期收缩与多位移QR)
 */
class SchurDecomposition11 : public QObject {
    Q_OBJECT

public:
    /** @brief Decomposition result */
    struct Result {
        QVector<QVector<double>> T;   // Upper quasi-triangular Schur form
        QVector<QVector<double>> Q;   // Unitary transformation matrix
        QVector<double> eigenvalues;  // Real eigenvalues
        QVector<QVector<double>> complexPairs; // Complex eigenvalue pairs (re, im)
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
        double residualNorm = 0.0;
    };

    explicit SchurDecomposition11(QObject *parent = nullptr);
    ~SchurDecomposition11() override;

    /** @brief Set maximum iterations */
    void setMaxIterations(int maxIter);

    /** @brief Set convergence tolerance */
    void setTolerance(double tol);

    /** @brief Set number of shifts per QR step */
    void setNumShifts(int shifts);

    /** @brief Compute Schur decomposition of real matrix */
    Result decompose(const QVector<QVector<double>>& matrix);

    /** @brief Get eigenvalues from last decomposition */
    QVector<double> eigenvalues() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decompositionDone(int n, int iterations, double residual, double timeMs);

private:
    int m_maxIter = 300;
    double m_tol = 1e-12;
    int m_numShifts = 6;  // Number of shifts per QR step

    Result m_lastResult;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Reduce to upper Hessenberg form via Householder reflections */
    void hessenbergReduce(QVector<QVector<double>>& A, QVector<QVector<double>>& Q);

    /** @brief Multi-shift implicit QR step */
    void multiShiftQR(QVector<QVector<double>>& H, QVector<QVector<double>>& Q,
                      int ilo, int ihi);

    /** @brief Aggressive early deflation check */
    void aggressiveDeflation(QVector<QVector<double>>& H, QVector<QVector<double>>& Q,
                             int& ilo, int& ihi);

    /** @brief Extract eigenvalues from quasi-triangular form */
    void extractEigenvalues(const QVector<QVector<double>>& T);

    /** @brief Apply Givens rotation to matrix rows/cols */
    void applyGivens(QVector<QVector<double>>& H, QVector<QVector<double>>& Q,
                     int i, int j, double c, double s, int n);

    /** @brief Compute Francis double shift parameters */
    void francisShift(const QVector<QVector<double>>& H, int ilo, int ihi,
                      double& s1, double& s2) const;

    /** @brief Chase bulge in implicit QR step */
    void chaseBulge(QVector<QVector<double>>& H, QVector<QVector<double>>& Q,
                    int ilo, int ihi, int m);

    /** @brief Check convergence of subdiagonal element */
    bool converged(const QVector<QVector<double>>& H, int i) const;

    /** @brief Compute residual ||A - Q T Q^T||_F */
    double computeResidual(const QVector<QVector<double>>& A,
                           const QVector<QVector<double>>& T,
                           const QVector<QVector<double>>& Q) const;
};
