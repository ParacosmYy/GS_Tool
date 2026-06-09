/**
 * @file SymmetricEigenSolver6.h
 * @brief 对称特征求解器(分治三对角特征值+泄降合并) — Symmetric Eigensolver with Divide-and-Conquer Tridiagonal Eigenvalue Computation and Deflated Merging
 *
 * 功能: 实现对称矩阵特征值求解(Symmetric Eigenvalue Decomposition)，使用
 *       Householder三对角化(tridiagonalization)，分治法(divide-and-conquer)
 *       求解三对角特征值，泄降合并(deflated merging)处理退化情况。
 *
 * 协作: SVD5(SVD分解) / LUDecomposition4(LU分解) / QRDecomposition3(QR分解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 对称特征求解器(分治三对角+泄降合并)
 */
class SymmetricEigenSolver6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int numEigenvalues = 0;
        int numDeflations = 0;
        int divideSteps = 0;
        double residual = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SymmetricEigenSolver6(QObject *parent = nullptr);
    ~SymmetricEigenSolver6() override;

    /** @brief Set tolerance for convergence */
    void setTolerance(double tol);

    /** @brief Compute eigenvalues and eigenvectors of symmetric matrix */
    bool solve(const QVector<QVector<double>>& matrix);

    /** @brief Get eigenvalues (ascending order) */
    QVector<double> eigenvalues() const;

    /** @brief Get eigenvectors as columns (N x N) */
    QVector<QVector<double>> eigenvectors() const;

    /** @brief Verify decomposition: compute ||A*V - V*D|| */
    double verifyDecomposition(const QVector<QVector<double>>& matrix) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int size, double residual, double timeMs);

private:
    double m_tolerance = 1e-10;
    int m_n = 0;

    QVector<double> m_eigenvalues;
    QVector<QVector<double>> m_eigenvectors;   // N x N, columns are eigenvectors

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Householder tridiagonalization */
    void tridiagonalize(QVector<QVector<double>>& A,
                         QVector<double>& diag,
                         QVector<double>& subdiag);

    /** @brief Divide-and-conquer for tridiagonal eigenproblem */
    void divideConquer(QVector<double>& diag,
                        QVector<double>& subdiag,
                        QVector<QVector<double>>& Q);

    /** @brief Secular equation solver for rank-1 update */
    double solveSecular(int n, const QVector<double>& d,
                         double rho, int k) const;

    /** @brief Deflated merging of child eigenproblems */
    void deflateMerge(const QVector<double>& d1,
                       const QVector<double>& d2,
                       double beta,
                       QVector<double>& merged) const;

    /** @brief Givens rotation to zero out element */
    void applyGivens(QVector<QVector<double>>& M,
                      int i, int j, double c, double s) const;

    /** @brief Matrix-vector multiply */
    QVector<double> matVec(const QVector<QVector<double>>& A,
                            const QVector<double>& v) const;
};
