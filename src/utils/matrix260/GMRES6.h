/**
 * @file GMRES6.h
 * @brief GMRES(缩减重启+调和Ritz值内部特征值逼近) — GMRES with Deflated Restarting and Harmonic Ritz Values for Interior Eigenvalue Approximation in Restart Cycles
 *
 * 功能: 实现GMRES(Generalized Minimal RESidual)迭代求解器，采用缩减
 *       重启(deflated restarting)策略，利用调和Ritz值(harmonic Ritz
 *       values)在重启周期中逼近内部特征值(interior eigenvalue)。
 *
 * 协作: ConjugateGradient3(CG求解) / SVD5(奇异值分解) / SparseLU4(稀疏LU)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief GMRES(缩减重启+调和Ritz值特征值逼近)
 */
class GMRES6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int restartCycles = 0;
        int totalIterations = 0;
        double finalResidual = 0.0;
        int numDeflated = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GMRES6(QObject *parent = nullptr);
    ~GMRES6() override;

    /** @brief Set matrix A in CSR format (row pointers, col indices, values) */
    void setMatrix(int n, const QVector<int>& rowPtr,
                   const QVector<int>& colIdx, const QVector<double>& values);

    /** @brief Set right-hand side vector b */
    void setRHS(const QVector<double>& b);

    /** @brief Set Krylov subspace dimension before restart */
    void setRestartDimension(int m);

    /** @brief Set convergence tolerance */
    void setTolerance(double tol);

    /** @brief Set maximum total iterations */
    void setMaxIterations(int maxIter);

    /** @brief Set number of deflation vectors */
    void setDeflationCount(int count);

    /** @brief Solve Ax = b */
    QVector<double> solve();

    /** @brief Compute residual ||Ax - b|| */
    double residual(const QVector<double>& x) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solvingCompleted(int iterations, double residual, double timeMs);

private:
    int m_n = 0;
    int m_restartDim = 30;
    double m_tol = 1e-8;
    int m_maxIter = 1000;
    int m_numDeflated = 2;

    // Sparse matrix (CSR)
    QVector<int> m_rowPtr;
    QVector<int> m_colIdx;
    QVector<double> m_values;
    QVector<double> m_rhs;

    // Deflation vectors from previous restart
    QVector<QVector<double>> m_deflVectors;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Sparse matrix-vector product y = A*x */
    QVector<double> spmv(const QVector<double>& x) const;

    /** @brief Arnoldi process with deflation */
    void arnoldi(int& m, QVector<QVector<double>>& V,
                 QVector<double>& hCol, int col) const;

    /** @brief Solve least-squares in Krylov subspace */
    QVector<double> solveLeastSquares(
        const QVector<QVector<double>>& H, int m,
        const QVector<double>& beta) const;

    /** @brief Compute harmonic Ritz values for deflation */
    QVector<double> harmonicRitzValues(
        const QVector<QVector<double>>& H, int m) const;

    /** @brief Gram-Schmidt orthogonalization with re-orthogonalization */
    void orthogonalize(QVector<QVector<double>>& V, int col,
                       QVector<double>& hCol) const;

    /** @brief Dot product of two vectors */
    static double dot(const QVector<double>& a, const QVector<double>& b);

    /** @brief Vector norm */
    static double vecNorm(const QVector<double>& v);
};
