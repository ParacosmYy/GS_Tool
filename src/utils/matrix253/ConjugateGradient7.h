/**
 * @file ConjugateGradient7.h
 * @brief 共轭梯度法(降阶Lanczos+增广Krylov子空间多特征对收敛) — Conjugate Gradient with Deflated Lanczos and Augmented Krylov Subspace for Multiple Eigenpair Convergence
 *
 * 功能: 实现共轭梯度法(Conjugate Gradient)，结合降阶Lanczos过程
 *       (deflated Lanczos)提取已收敛特征对，通过增广Krylov子空间
 *       (augmented Krylov subspace)加速多特征对的联合收敛。
 *
 * 协作: LU7(LU分解) / Cholesky7(Cholesky分解) / SVD8(奇异值分解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 共轭梯度法(降阶Lanczos+增广Krylov子空间)
 */
class ConjugateGradient7 : public QObject {
    Q_OBJECT

public:
    /** @brief Eigenpair result */
    struct EigenPair {
        double eigenvalue = 0.0;
        QVector<double> eigenvector;
    };

    /** @brief Solver result */
    struct SolveResult {
        QVector<double> solution;
        double residual = 0.0;
        int iterations = 0;
        bool converged = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int numIterations = 0;
        int numEigenpairs = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ConjugateGradient7(QObject *parent = nullptr);
    ~ConjugateGradient7() override;

    /** @brief Set tolerance and max iterations */
    void setTolerance(double tol, int maxIter);

    /** @brief Solve Ax = b where A is SPD */
    SolveResult solve(const QVector<QVector<double>>& A,
                      const QVector<double>& b);

    /** @brief Compute k largest eigenpairs via deflated Lanczos */
    QVector<EigenPair> eigenpairs(const QVector<QVector<double>>& A, int k);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int iterations, double residual, double timeMs);

private:
    double m_tolerance = 1e-8;
    int m_maxIter = 1000;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Matrix-vector multiplication */
    QVector<double> matVec(const QVector<QVector<double>>& A,
                            const QVector<double>& x) const;

    /** @brief Dot product */
    double dot(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Vector norm */
    double norm(const QVector<double>& v) const;

    /** @brief Lanczos tridiagonalization for n steps */
    void lanczos(const QVector<QVector<double>>& A, int n,
                  QVector<double>& alpha, QVector<double>& beta,
                  QVector<QVector<double>>& Q) const;

    /** @brief QR algorithm for tridiagonal eigenvalue problem */
    void tridiagQR(QVector<double>& diag, QVector<double>& subdiag,
                    QVector<QVector<double>>& eigvecs) const;
};
