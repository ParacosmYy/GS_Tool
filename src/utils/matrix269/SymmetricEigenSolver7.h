/**
 * @file SymmetricEigenSolver7.h
 * @brief 对称特征值求解(分治三对角策略与收缩保证正交特征向量) — Symmetric Eigenvalue Solver with Divide-and-Conquer Tridiagonal Strategy and Deflation for Guaranteed Orthogonal Eigenvectors
 *
 * 功能: 实现对称特征值求解(symmetric eigenvalue solver)，采用分治三对角策略
 *       (divide-and-conquer tridiagonal strategy)和收缩(deflation)实现保证正交特征向量
 *       (guaranteed orthogonal eigenvectors)。
 *
 * 协作: MatrixDecomp6(矩阵分解) / SVD5(奇异值分解) / LDLDecomp4(LDL分解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 对称特征值求解(分治三对角策略与收缩保证正交特征向量)
 */
class SymmetricEigenSolver7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int numEigenvalues = 0;
        double residual = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SymmetricEigenSolver7(QObject *parent = nullptr);
    ~SymmetricEigenSolver7() override;

    /** @brief Solve all eigenvalues and eigenvectors of symmetric matrix */
    bool solve(const QVector<QVector<double>>& matrix);

    /** @brief Solve only k largest eigenvalues */
    bool solveLargest(const QVector<QVector<double>>& matrix, int k);

    /** @brief Get eigenvalues in descending order */
    QVector<double> eigenvalues() const;

    /** @brief Get eigenvectors (columns correspond to eigenvalues) */
    QVector<QVector<double>> eigenvectors() const;

    /** @brief Verify orthogonality of eigenvectors, return max deviation */
    double verifyOrthogonality() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int n, int numEigen, double residual, double timeMs);

private:
    int m_n = 0;
    QVector<double> m_eigenvalues;
    QVector<QVector<double>> m_eigenvectors;  // Column-major: [vec0, vec1, ...]

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Householder tridiagonalization */
    void tridiagonalize(QVector<QVector<double>>& A,
                         QVector<double>& diag,
                         QVector<double>& subdiag);

    /** @brief Divide-and-conquer on tridiagonal matrix */
    void divideConquerTD(QVector<double>& diag,
                          QVector<double>& subdiag,
                          QVector<QVector<double>>& eigvecs);

    /** @brief Secular equation solver for rank-1 update eigenvalues */
    QVector<double> solveSecular(const QVector<double>& d,
                                   double rho,
                                   const QVector<double>& z) const;

    /** @brief Apply deflation to handle near-duplicate eigenvalues */
    void deflate(QVector<double>& d, QVector<double>& z,
                  QVector<QVector<double>>& Q) const;

    /** @brief QR iteration for small tridiagonal blocks */
    void qrTridiagonal(QVector<double>& diag,
                        QVector<double>& subdiag,
                        QVector<QVector<double>>& eigvecs) const;

    /** @brief Dot product of two vectors */
    static double dot(const QVector<double>& a, const QVector<double>& b);
};
