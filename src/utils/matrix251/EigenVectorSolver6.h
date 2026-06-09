/**
 * @file EigenVectorSolver6.h
 * @brief 特征向量求解器(子空间迭代+Rayleigh-Ritz投影的主特征对提取) — Eigenvector Solver with Subspace Iteration and Rayleigh-Ritz Projection for Dominant Eigenpair Extraction
 *
 * 功能: 实现特征向量求解器(Eigenvector Solver)，使用子空间迭代(subspace
 *       iteration)逐步逼近主导不变子空间，通过Rayleigh-Ritz投影(Rayleigh-
 *       Ritz projection)提取主特征对(dominant eigenpair)。
 *
 * 协作: QRDecomposition5(QR分解) / SvdSolver6(奇异值分解) / LUDecomposition5(LU分解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 特征向量求解器(子空间迭代+Rayleigh-Ritz投影)
 */
class EigenVectorSolver6 : public QObject {
    Q_OBJECT

public:
    /** @brief Eigenpair result */
    struct EigenPair {
        double eigenvalue = 0.0;
        QVector<double> eigenvector;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int numEigenpairs = 0;
        int totalIterations = 0;
        double residual = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit EigenVectorSolver6(QObject *parent = nullptr);
    ~EigenVectorSolver6() override;

    /** @brief Set number of eigenpairs to compute */
    void setNumEigenpairs(int k);

    /** @brief Set maximum subspace iterations */
    void setMaxIterations(int iters);

    /** @brief Set convergence tolerance */
    void setTolerance(double tol);

    /** @brief Set subspace dimension (>= numEigenpairs) */
    void setSubspaceDimension(int dim);

    /** @brief Solve for dominant eigenpairs of symmetric matrix */
    QVector<EigenPair> solve(const QVector<QVector<double>>& matrix);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solvingCompleted(int numPairs, int iterations, double residual, double timeMs);

private:
    int m_numPairs = 1;
    int m_maxIter = 200;
    double m_tol = 1e-10;
    int m_subDim = 4;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Matrix-vector multiply */
    QVector<double> matVec(const QVector<QVector<double>>& A,
                            const QVector<double>& x) const;

    /** @brief Vector dot product */
    static double dot(const QVector<double>& a, const QVector<double>& b);

    /** @brief Vector norm */
    static double norm(const QVector<double>& v);

    /** @brief Orthonormalize basis via modified Gram-Schmidt */
    void orthonormalize(QVector<QVector<double>>& basis) const;

    /** @brief Compute Rayleigh-Ritz projection and extract Ritz pairs */
    QVector<EigenPair> rayleighRitz(const QVector<QVector<double>>& A,
                                     const QVector<QVector<double>>& Q) const;

    /** @brief QR decomposition (thin) for 2D tridiagonal solve */
    void solveTridiagonal(QVector<double>& diag, QVector<double>& offDiag,
                           QVector<QVector<double>>& eigvecs) const;
};
