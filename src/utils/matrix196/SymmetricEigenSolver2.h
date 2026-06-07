/**
 * @file SymmetricEigenSolver2.h
 * @brief 对称特征值求解(分治+压缩消去+割线方程) — Symmetric Eigenvalue via Divide-and-Conquer with Deflation and Secular Equation Solution
 *
 * 功能: 实现对称矩阵特征值分解，支持分治策略(D&C)、
 *       压缩消去(deflation)和割线方程(secular equation)求解。
 *
 * 协作: SvdSolver4(SVD) / QRDecomposition6(QR) / Cholesky5(Cholesky)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 对称特征值求解器(分治+消去+割线方程)
 */
class SymmetricEigenSolver2 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;
        int matrixSize = 0;
        int deflations = 0;
        double residual = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SymmetricEigenSolver2(QObject *parent = nullptr);
    ~SymmetricEigenSolver2() override;

    /** @brief Solve eigenvalues and eigenvectors of symmetric matrix */
    bool solve(const QVector<QVector<double>>& matrix);

    /** @brief Get eigenvalues in ascending order */
    QVector<double> eigenvalues() const;

    /** @brief Get eigenvectors (columns) */
    QVector<QVector<double>> eigenvectors() const;

    /** @brief Verify solution: A*v = lambda*v */
    double verifyResidual(const QVector<QVector<double>>& matrix) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int size, double residual, double timeMs);

private:
    QVector<double> m_eigenvalues;
    QVector<QVector<double>> m_eigenvectors;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Tridiagonalize symmetric matrix via Householder */
    void tridiagonalize(QVector<double>& diag, QVector<double>& subdiag,
                        QVector<QVector<double>>& Q);

    /** @brief Divide-and-conquer on tridiagonal */
    void dcSolve(QVector<double>& diag, QVector<double>& subdiag,
                 QVector<QVector<double>>& Q, int lo, int hi);

    /** @brief Solve secular equation for rank-1 update */
    QVector<double> solveSecular(const QVector<double>& d,
                                  const QVector<double>& z,
                                  double rho) const;

    /** @brief Deflation: detect and remove degenerate cases */
    int deflate(QVector<double>& d, QVector<double>& z,
                double& rho) const;

    /** @brief Compute Givens rotation */
    void givens(double a, double b, double& c, double& s) const;

    /** @brief Matrix-vector multiply */
    QVector<double> matVec(const QVector<QVector<double>>& A,
                           const QVector<double>& v) const;
};
