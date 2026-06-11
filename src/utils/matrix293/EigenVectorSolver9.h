/**
 * @file EigenVectorSolver9.h
 * @brief 特征向量求解器(Lanczos三对角化与隐式重启谱变换求解内部特征值) — Eigenvector Solver with Lanczos Tridiagonalization and Implicit Restart with Spectral Transformation for Interior Eigenvalues
 *
 * 功能: 实现特征向量求解器(Eigenvector solver)，采用Lanczos三对角化(Lanczos tridiagonalization)
 *       与隐式重启谱变换(implicit restart with spectral transformation)求解内部特征值(interior eigenvalues)。
 *
 * 协作: SVD12(奇异值分解) / LeastSquares11(最小二乘) / MatrixInverse10(矩阵求逆)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 特征向量求解器(Lanczos三对角化与隐式重启谱变换求解内部特征值)
 */
class EigenVectorSolver9 : public QObject {
    Q_OBJECT

public:
    /** @brief Eigenvalue/eigenvector result */
    struct EigenResult {
        QVector<double> eigenvalues;
        QVector<QVector<double>> eigenvectors; // columns = eigenvectors
        int iterations = 0;
        bool converged = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int lastN = 0;
        int lastK = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit EigenVectorSolver9(QObject *parent = nullptr);
    ~EigenVectorSolver9() override;

    void setNumEigen(int k);       // Number of eigenvalues to compute
    void setMaxIter(int iters);
    void setTolerance(double tol);
    void setShift(double sigma);   // Spectral shift for interior eigenvalues

    /** @brief Solve for eigenvalues/eigenvectors of symmetric matrix */
    EigenResult solve(const QVector<QVector<double>>& matrix) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveDone(int n, int k, bool converged, double timeMs);

private:
    int m_k = 5;
    int m_maxIter = 300;
    double m_tol = 1e-10;
    double m_shift = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Matrix-vector product */
    QVector<double> matVec(const QVector<QVector<double>>& A,
                           const QVector<double>& v) const;

    /** @brief Lanczos iteration: build tridiagonal matrix T */
    void lanczos(const QVector<QVector<double>>& A, int m,
                 QVector<double>& alpha, QVector<double>& beta,
                 QVector<QVector<double>>& Q) const;

    /** @brief QR algorithm for tridiagonal eigenvalue problem */
    void tridiagQR(QVector<double>& diag, QVector<double>& subdiag,
                   QVector<QVector<double>>& eigvecs, int maxIter) const;

    /** @brief Implicit restart (filter unwanted Ritz values) */
    void implicitRestart(QVector<double>& alpha, QVector<double>& beta,
                         QVector<QVector<double>>& Q, int k, int p) const;

    /** @brief Reconstruct eigenvectors from Lanczos basis */
    QVector<QVector<double>> reconstructEigenvectors(
        const QVector<QVector<double>>& Q,
        const QVector<QVector<double>>& tridiagEigvecs) const;
};
