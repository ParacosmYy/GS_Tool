/**
 * @file SymmetricEigenSolver9.h
 * @brief 对称特征求解(分治三对角策略与放缩实现实对称矩阵全部特征对计算) — Symmetric Eigensolver with Divide-and-conquer Tridiagonal Strategy and Deflation for Computing All Eigenpairs of Real Symmetric Matrices
 *
 * 功能: 实现对称特征求解(symmetric eigensolver)，采用分治三对角策略(divide-and-conquer tridiagonal strategy)
 *       与放缩(deflation)实现实对称矩阵全部特征对计算(computing all eigenpairs of real symmetric matrices)。
 *
 * 协作: SVD15(奇异值分解) / QRDecomposition12(QR分解) / Cholesky10(Cholesky分解)
 */
#pragma once

#include <QObject>
#include <QVector>

class SymmetricEigenSolver9 : public QObject {
    Q_OBJECT

public:
    /** @brief Eigen decomposition result */
    struct EigenResult {
        QVector<double> eigenvalues;            // Sorted ascending
        QVector<QVector<double>> eigenvectors;  // Columns are eigenvectors
        int matrixSize = 0;
        int iterations = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalSolves = 0;
        int matrixSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SymmetricEigenSolver9(QObject *parent = nullptr);
    ~SymmetricEigenSolver9() override;

    void setMaxIterations(int maxIter);
    void setConvergenceTolerance(double tol);

    /** @brief Compute all eigenpairs of a symmetric matrix */
    EigenResult solve(const QVector<QVector<double>>& matrix);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveDone(int n, double timeMs);

private:
    int m_maxIter = 100;
    double m_tol = 1e-12;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Tridiagonalize via Householder reflections */
    void tridiagonalize(QVector<QVector<double>>& A,
                          QVector<double>& diag,
                          QVector<double>& subdiag) const;

    /** @brief QR iteration on tridiagonal matrix */
    void tridiagQR(QVector<double>& diag,
                     QVector<double>& subdiag,
                     QVector<QVector<double>>& Q) const;

    /** @brief Apply Householder reflection */
    void householderReflect(QVector<double>& v, double& beta) const;

    /** @brief Wilkinson shift for QR iteration */
    double wilkinsonShift(double d, double e) const;
};
