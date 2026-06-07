/**
 * @file EigenVectorSolver3.h
 * @brief 特征向量求解器(子空间迭代+Ritz加速+块Rayleigh-Ritz) — Eigenvector Computation via Subspace Iteration with Ritz Acceleration and Block Rayleigh-Ritz
 *
 * 功能: 实现子空间迭代特征向量求解，支持Ritz加速收敛、
 *       块Rayleigh-Ritz投影和对称矩阵特征分解。
 *
 * 协作: SvdSolver4(SVD求解器) / MatrixDecomp3(矩阵分解) / EigenVectorSolver2(幂迭代)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 特征向量求解器(子空间迭代+Ritz加速+块Rayleigh-Ritz)
 */
class EigenVectorSolver3 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalSolves = 0;
        int matrixSize = 0;
        int numEigenvalues = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit EigenVectorSolver3(QObject *parent = nullptr);
    ~EigenVectorSolver3() override;

    void setNumEigenvalues(int k);
    void setMaxIterations(int iter);
    void setTolerance(double tol);

    /** @brief Solve for eigenvalues and eigenvectors of symmetric matrix */
    void solve(const QVector<QVector<double>>& matrix);

    /** @brief Perform one subspace iteration step with Ritz acceleration */
    void subspaceIteration(QVector<QVector<double>>& V,
                            const QVector<QVector<double>>& A) const;

    /** @brief Block Rayleigh-Ritz projection */
    void rayleighRitz(const QVector<QVector<double>>& V,
                       const QVector<QVector<double>>& A,
                       QVector<double>& ritzValues,
                       QVector<QVector<double>>& ritzVectors) const;

    /** @brief Matrix-vector multiply */
    static QVector<double> matVec(const QVector<QVector<double>>& A,
                                   const QVector<double>& x);

    /** @brief Get computed eigenvalues */
    QVector<double> eigenvalues() const;

    /** @brief Get computed eigenvectors (columns) */
    QVector<QVector<double>> eigenvectors() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solved(int numEigen, double residual, double timeMs);

private:
    int m_numEigen = 4;
    int m_maxIter = 200;
    double m_tol = 1e-8;

    QVector<double> m_eigenvalues;
    QVector<QVector<double>> m_eigenvectors;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Orthogonalize columns of V via modified Gram-Schmidt */
    static void orthogonalize(QVector<QVector<double>>& V);
};
