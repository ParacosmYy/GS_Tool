/**
 * @file SchurDecomposition7.h
 * @brief Schur分解(AED加速收缩+Francis双隐式位移QR步) — Schur Decomposition with AED Accelerated Deflation and Francis Double-Implicit-Shift QR Step
 *
 * 功能: 实现实Schur分解，使用AED(Aggressive Early Deflation)加速收缩，
 *       Francis双隐式位移QR迭代完成上三角化。
 *
 * 协作: EigenSolver5(特征值求解) / QRDecomposition6(QR分解) / SVD7(奇异值分解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Schur分解(AED加速收缩+Francis双隐式位移QR)
 */
class SchurDecomposition7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int qrIterations = 0;
        int deflations = 0;
        int aedDeflations = 0;
        bool converged = false;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SchurDecomposition7(QObject *parent = nullptr);
    ~SchurDecomposition7() override;

    /** @brief Set parameters: max iterations, convergence tolerance */
    void setParameters(int maxIterations = 300, double tolerance = 1e-12);

    /** @brief Compute Schur decomposition: A = Q * T * Q^T */
    void decompose(const QVector<QVector<double>>& matrix);

    /** @brief Get the quasi-upper-triangular Schur form T */
    QVector<QVector<double>> schurForm() const;

    /** @brief Get the orthogonal transformation matrix Q */
    QVector<QVector<double>> transformationMatrix() const;

    /** @brief Extract real eigenvalues from Schur form */
    QVector<double> realEigenvalues() const;

    /** @brief Extract complex eigenvalue pairs (conjugate pairs) */
    QVector<QVector<double>> complexEigenvaluePairs() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decompositionCompleted(int size, int iterations, bool converged, double timeMs);

private:
    int m_maxIterations = 300;
    double m_tolerance = 1e-12;
    int m_n = 0;

    QVector<QVector<double>> m_T;  // Schur form
    QVector<QVector<double>> m_Q;  // Orthogonal matrix

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Householder reflection to reduce to upper Hessenberg */
    void hessenbergReduction(QVector<QVector<double>>& A, QVector<QVector<double>>& Q);

    /** @brief Francis double-implicit-shift QR step */
    void francisQRStep(QVector<QVector<double>>& H, int lo, int hi,
                       QVector<QVector<double>>& Q);

    /** @brief Aggressive early deflation */
    int aggressiveEarlyDeflation(QVector<QVector<double>>& H, int lo, int hi,
                                  QVector<QVector<double>>& Q);

    /** @brief Check for deflation at position */
    int checkDeflation(const QVector<QVector<double>>& H, int lo, int hi) const;

    /** @brief Apply Givens rotation to matrix */
    void applyGivensLeft(QVector<QVector<double>>& A, int i, int j,
                          double c, double s);
    void applyGivensRight(QVector<QVector<double>>& A, int i, int j,
                           double c, double s);

    /** @brief Compute 2x2 eigenvalues */
    QVector<double> eigenvalues2x2(double a, double b, double c, double d) const;

    /** @brief Identity matrix */
    QVector<QVector<double>> identity(int n) const;

    /** @brief Matrix multiply A*B */
    QVector<QVector<double>> matMul(const QVector<QVector<double>>& A,
                                      const QVector<QVector<double>>& B) const;
};
