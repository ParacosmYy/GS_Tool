/**
 * @file GeneralizedEigenSolver2.h
 * @brief 广义特征值求解(QZ分解+实Schur形式+alpha/beta对提取) — Generalized Eigenvalue via QZ Decomposition with Real Schur Form and Alpha/Beta Eigenvalue Pair Extraction
 *
 * 功能: 实现广义特征值求解，支持QZ分解、实Schur形式计算
 *       和alpha/beta特征值对提取(Ax = λBx)。
 *
 * 协作: EigenDecomp5(特征分解) / SvdSolver4(SVD) / QRDecomp3(QR分解)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 广义特征值求解器(QZ+实Schur+alpha/beta)
 */
class GeneralizedEigenSolver2 : public QObject {
    Q_OBJECT

public:
    /** @brief Generalized eigenvalue: alpha/beta pair where lambda = alpha/beta */
    struct EigenPair {
        double alpha_r = 0.0;  // real part of alpha
        double alpha_i = 0.0;  // imag part of alpha
        double beta = 1.0;     // beta (real)
        double lambda() const { return (qAbs(beta) < 1e-15) ? 0.0 : alpha_r / beta; }
        double lambdaImag() const { return (qAbs(beta) < 1e-15) ? 0.0 : alpha_i / beta; }
        bool isReal() const { return qAbs(alpha_i) < 1e-12; }
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSolves = 0;
        int matrixSize = 0;
        int iterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GeneralizedEigenSolver2(QObject *parent = nullptr);
    ~GeneralizedEigenSolver2() override;

    void setMaxIterations(int iter);
    void setTolerance(double tol);

    /** @brief Solve generalized eigenvalue problem Ax = lambda * Bx */
    QVector<EigenPair> solve(const QVector<QVector<double>>& A,
                              const QVector<QVector<double>>& B);

    /** @brief Get Schur form Q from last solve */
    QVector<QVector<double>> schurQ() const;

    /** @brief Get Schur form Z from last solve */
    QVector<QVector<double>> schurZ() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int n, int iterations, double timeMs);

private:
    int m_maxIter = 300;
    double m_tolerance = 1e-10;

    QVector<QVector<double>> m_Q;
    QVector<QVector<double>> m_Z;
    QVector<EigenPair> m_eigenvalues;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Hessenberg reduction of A, upper triangular of B */
    void hessenbergTriangular(QVector<QVector<double>>& A,
                               QVector<QVector<double>>& B,
                               QVector<QVector<double>>& Q,
                               QVector<QVector<double>>& Z);

    /** @brief QZ iteration to get real Schur form */
    void qzIteration(QVector<QVector<double>>& A,
                      QVector<QVector<double>>& B,
                      QVector<QVector<double>>& Q,
                      QVector<QVector<double>>& Z);

    /** @brief Extract alpha/beta pairs from Schur form */
    QVector<EigenPair> extractEigenvalues(
        const QVector<QVector<double>>& S,
        const QVector<QVector<double>>& T) const;

    /** @brief Givens rotation: zero out element at (row, col) */
    void givensRotation(QVector<QVector<double>>& mat, int row1, int row2,
                         int col, double& c, double& s);
};
