/**
 * @file SymmetricEigenSolver5.h
 * @brief 对称特征求解器(隐式对称QR+Wilkinson位移三对角特征值) — Symmetric Eigensolver with Implicit Symmetric QR and Wilkinson Shift for Tridiagonal Matrix Eigenvalue Computation
 *
 * 功能: 实现对称矩阵特征值求解器(symmetric eigensolver)，采用Householder三对角化
 *       (Householder tridiagonalization)、隐式对称QR迭代(implicit symmetric QR iteration)
 *       和Wilkinson位移(Wilkinson shift)计算三对角矩阵的全部特征值和特征向量。
 *
 * 协作: SVD4(奇异值分解) / CholeskyDecomp3(Cholesky分解) / QRDecomposition2(QR分解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 对称特征求解器(隐式对称QR+Wilkinson位移三对角特征值)
 */
class SymmetricEigenSolver5 : public QObject {
    Q_OBJECT

public:
    /** @brief Eigen decomposition result */
    struct EigenResult {
        QVector<double> eigenvalues;
        QVector<QVector<double>> eigenvectors;
        int iterations = 0;
        bool converged = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int totalIterations = 0;
        int numDecompositions = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SymmetricEigenSolver5(QObject *parent = nullptr);
    ~SymmetricEigenSolver5() override;

    /** @brief Set max QR iterations */
    void setMaxIterations(int iter);

    /** @brief Set convergence tolerance */
    void setTolerance(double tol);

    /** @brief Compute all eigenvalues and eigenvectors of symmetric matrix */
    EigenResult solve(const QVector<QVector<double>>& matrix);

    /** @brief Compute only eigenvalues (faster) */
    QVector<double> eigenvaluesOnly(const QVector<QVector<double>>& matrix);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void iterationCompleted(int iter, double offDiagNorm);
    void solveCompleted(int n, int iters, double timeMs);

private:
    int m_maxIter = 300;
    double m_tol = 1e-10;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Householder tridiagonalization: A -> T, accumulate Q */
    void tridiagonalize(QVector<QVector<double>>& mat,
                        QVector<QVector<double>>& Q) const;

    /** @brief Implicit symmetric QR step with Wilkinson shift on tridiagonal */
    void implicitQRStep(QVector<double>& diag, QVector<double>& subdiag,
                        QVector<QVector<double>>& Q, int lo, int hi) const;

    /** @brief Compute Wilkinson shift for 2x2 trailing block */
    double wilkinsonShift(double d1, double d2, double e) const;

    /** @brief Check convergence of off-diagonal elements */
    double offDiagonalNorm(const QVector<double>& subdiag, int lo, int hi) const;

    /** @brief Givens rotation applied to rows/cols of Q */
    void applyGivens(QVector<QVector<double>>& Q, int i, int j,
                     double c, double s) const;

    /** @brief Deflate converged eigenvalue */
    void deflate(QVector<double>& diag, QVector<double>& subdiag,
                 int& lo, int& hi) const;
};
