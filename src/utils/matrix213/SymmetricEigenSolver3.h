/**
 * @file SymmetricEigenSolver3.h
 * @brief 对称特征值求解器(分治三对角策略+Givens旋转降阶) — Symmetric Eigensolver with Divide-and-Conquer Tridiagonal Strategy and Deflation by Givens Rotation
 *
 * 功能: 实现对称矩阵特征值分解，支持Householder三对角化、
 *       分治策略和Givens旋转降阶。
 *
 * 协作: QrDecomposition7(QR分解) / SvdSolver6(SVD求解器) / LinearSolver5(线性求解器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 对称特征值求解器(分治三对角策略+Givens旋转降阶)
 */
class SymmetricEigenSolver3 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int dcDepth = 0;
        int givensRotations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SymmetricEigenSolver3(QObject *parent = nullptr);
    ~SymmetricEigenSolver3() override;

    void setMaxIterations(int maxIter);
    void setTolerance(double tol);

    /** @brief Compute eigenvalues and eigenvectors of symmetric matrix */
    void compute(const QVector<QVector<double>>& matrix);

    /** @brief Get eigenvalues in ascending order */
    QVector<double> eigenvalues() const;

    /** @brief Get eigenvectors as columns */
    QVector<QVector<double>> eigenvectors() const;

    /** @brief Householder tridiagonalization */
    void tridiagonalize(const QVector<QVector<double>>& matrix);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void computationCompleted(int size, int depth, double timeMs);

private:
    int m_maxIter = 100;
    double m_tol = 1e-10;
    int m_n = 0;

    QVector<double> m_eigenvalues;
    QVector<QVector<double>> m_eigenvectors;

    // Tridiagonal form
    QVector<double> m_diag;      // Main diagonal
    QVector<double> m_subdiag;   // Sub-diagonal

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Divide-and-conquer on tridiagonal */
    void divideAndConquer(QVector<double>& diag, QVector<double>& subdiag,
                           QVector<QVector<double>>& Q, int depth);

    /** @brief QR iteration for small tridiagonal matrices */
    void qrTridiagonal(QVector<double>& diag, QVector<double>& subdiag,
                        QVector<QVector<double>>& Q);

    /** @brief Apply Givens rotation for deflation */
    void givensDeflate(QVector<double>& diag, QVector<double>& subdiag,
                        QVector<QVector<double>>& Q, int row, double a, double b);

    /** @brief Compute secular equation for rank-1 update */
    void solveSecular(QVector<double>& diag, double rho,
                       const QVector<double>& v,
                       QVector<double>& eigenvals,
                       QVector<QVector<double>>& eigvecs);

    /** @brief Merge subproblems with rank-1 update */
    void mergeSubproblems(const QVector<double>& d1, const QVector<double>& d2,
                           const QVector<double>& e1, const QVector<double>& e2,
                           const QVector<QVector<double>>& Q1,
                           const QVector<QVector<double>>& Q2,
                           QVector<double>& diag, QVector<QVector<double>>& Q);
};
