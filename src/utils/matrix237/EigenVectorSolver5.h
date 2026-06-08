/**
 * @file EigenVectorSolver5.h
 * @brief 特征向量求解器(Lanczos双正交化+前瞻Lanczos避免崩溃) — Eigenvector Solver with Lanczos Biorthogonalization and Look-Ahead Lanczos for Breakdown Avoidance
 *
 * 功能: 实现特征向量求解器(eigenvector solver)，采用Lanczos双正交化(Lanczos
 *       biorthogonalization)方法，结合前瞻Lanczos(look-ahead Lanczos)策略
 *       避免崩溃(breakdown avoidance)。
 *
 * 协作: EigenValueSolver4(特征值求解) / SvdSolver3(SVD求解) / MatrixOps2(矩阵运算)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 特征向量求解器(Lanczos双正交化+前瞻Lanczos避免崩溃)
 */
class EigenVectorSolver5 : public QObject {
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
        int krylovDim = 0;
        int numBreakdowns = 0;
        int numLookahead = 0;
        int iterationsUsed = 0;
        double residual = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit EigenVectorSolver5(QObject *parent = nullptr);
    ~EigenVectorSolver5() override;

    /** @brief Set number of requested eigenpairs */
    void setNumEigenPairs(int k);

    /** @brief Set maximum Krylov subspace dimension */
    void setMaxIterations(int maxIter);

    /** @brief Solve for eigenpairs of a square matrix */
    QVector<EigenPair> solve(const QVector<QVector<double>>& matrix,
                              double tol = 1e-10);

    /** @brief Get residual norms */
    QVector<double> residuals() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void iterationCompleted(int iter, double residual);
    void solveCompleted(int numPairs, double timeMs);

private:
    int m_numPairs = 5;
    int m_maxIter = 100;
    double m_tol = 1e-10;

    QVector<EigenPair> m_eigenPairs;
    QVector<double> m_residuals;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Matrix-vector multiply y = A * x */
    QVector<double> matVec(const QVector<QVector<double>>& A,
                            const QVector<double>& x) const;

    /** @brief Dot product */
    double dot(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Vector norm */
    double norm(const QVector<double>& v) const;

    /** @brief Lanczos biorthogonalization with look-ahead */
    bool lanczosBiortho(const QVector<QVector<double>>& A,
                         QVector<QVector<double>>& V,
                         QVector<QVector<double>>& W,
                         QVector<double>& alpha,
                         QVector<double>& beta,
                         int& actualDim);

    /** @brief Solve tridiagonal eigenvalue problem (QR iteration) */
    void tridiagEigen(const QVector<double>& alpha,
                       const QVector<double>& beta,
                       QVector<double>& eigenvalues,
                       QVector<QVector<double>>& eigenvectors) const;
};
