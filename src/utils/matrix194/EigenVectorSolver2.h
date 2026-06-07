/**
 * @file EigenVectorSolver2.h
 * @brief 特征向量求解(逆迭代+Rayleigh商移位+压缩) — Eigenvector Computation via Inverse Iteration with Rayleigh Quotient Shift and Deflation
 *
 * 功能: 实现特征向量求解器，支持逆迭代(Inverse Iteration)算法、
 *       Rayleigh商移位加速收敛、压缩(Deflation)求多特征对。
 *
 * 协作: SvdSolver4(SVD求解) / MatrixOps3(矩阵运算) / LUDecomposition4(LU分解)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 特征向量求解器(逆迭代+Rayleigh商移位+压缩)
 */
class EigenVectorSolver2 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSolves = 0;
        int matrixSize = 0;
        int eigenPairsFound = 0;
        int totalIterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit EigenVectorSolver2(QObject *parent = nullptr);
    ~EigenVectorSolver2() override;

    void setMaxIterations(int iter);
    void setTolerance(double tol);
    void setNumEigenPairs(int n);

    /** @brief Solve for eigenpairs (eigenvalues + eigenvectors) */
    QVector<QPair<double, QVector<double>>> solve(
        const QVector<QVector<double>>& matrix);

    /** @brief Get eigenvalues from last solve */
    QVector<double> eigenvalues() const { return m_eigenvalues; }

    /** @brief Get eigenvectors from last solve */
    QVector<QVector<double>> eigenvectors() const { return m_eigenvectors; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int pairs, int iterations, double timeMs);

private:
    int m_maxIterations = 200;
    double m_tolerance = 1e-10;
    int m_numPairs = 1;

    QVector<double> m_eigenvalues;
    QVector<QVector<double>> m_eigenvectors;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Inverse iteration with Rayleigh quotient shift */
    QPair<double, QVector<double>> inverseIteration(
        const QVector<QVector<double>>& mat, double shift) const;

    /** @brief Rayleigh quotient: (v^T A v) / (v^T v) */
    double rayleighQuotient(const QVector<QVector<double>>& mat,
                            const QVector<double>& v) const;

    /** @brief Solve linear system Ax = b via LU decomposition */
    QVector<double> solveLinear(const QVector<QVector<double>>& A,
                                const QVector<double>& b) const;

    /** @brief Deflation: remove found eigenpair from matrix */
    QVector<QVector<double>> deflate(
        const QVector<QVector<double>>& mat,
        double eigenvalue, const QVector<double>& eigenvector) const;

    /** @brief Normalize vector to unit length */
    QVector<double> normalize(const QVector<double>& v) const;

    /** @brief Dot product */
    double dot(const QVector<double>& a, const QVector<double>& b) const;
};
