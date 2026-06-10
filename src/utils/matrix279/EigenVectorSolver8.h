/**
 * @file EigenVectorSolver8.h
 * @brief 特征向量求解器(隐式重启Arnoldi与Schur向量提取大型稀疏特征值问题) — Eigenvector Solver with Implicitly Restarted Arnoldi and Schur Vector Extraction for Large Sparse Eigenvalue Problems
 *
 * 功能: 实现特征向量求解器(Eigenvector solver)，采用隐式重启Arnoldi
 *       (implicitly restarted Arnoldi)与Schur向量提取(Schur vector extraction)
 *       实现大型稀疏特征值问题(large sparse eigenvalue problems)。
 *
 * 协作: SVD9(奇异值分解) / QRFactorization7(QR分解) / PowerIteration6(幂迭代)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 特征向量求解器(隐式重启Arnoldi与Schur向量提取)
 */
class EigenVectorSolver8 : public QObject {
    Q_OBJECT

public:
    /** @brief Eigenvalue type selector */
    enum EigenTarget { LargestMagnitude = 0, SmallestMagnitude, LargestReal, SmallestReal };

    /** @brief Computed eigenpair */
    struct EigenPair {
        double realPart = 0.0;
        double imagPart = 0.0;
        QVector<double> eigenvector;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int numEigenvalues = 0;
        int arnoldiIterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit EigenVectorSolver8(QObject *parent = nullptr);
    ~EigenVectorSolver8() override;

    /** @brief Set number of desired eigenvalues */
    void setNumEigenvalues(int k);

    /** @brief Set Arnoldi subspace dimension (>= k+1) */
    void setSubspaceDim(int m);

    /** @brief Set maximum Arnoldi restarts */
    void setMaxRestarts(int r);

    /** @brief Set convergence tolerance */
    void setTolerance(double tol);

    /** @brief Set which eigenvalues to target */
    void setTarget(EigenTarget target);

    /** @brief Solve using sparse matrix (CSR format: values, colIdx, rowPtr) */
    QVector<EigenPair> solveSparse(const QVector<double>& values,
                                   const QVector<int>& colIdx,
                                   const QVector<int>& rowPtr, int n);

    /** @brief Solve using dense matrix (row-major) */
    QVector<EigenPair> solveDense(const QVector<QVector<double>>& matrix);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solvingDone(int numEigenvalues, int iterations, double timeMs);

private:
    int m_k = 5;               // Number of desired eigenvalues
    int m_m = 20;              // Arnoldi subspace dimension
    int m_maxRestarts = 100;
    double m_tol = 1e-10;
    EigenTarget m_target = LargestMagnitude;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Arnoldi decomposition: A*V_m = V_{m+1} * H_m */
    void arnoldiDecomp(const QVector<QVector<double>>& A,
                       QVector<QVector<double>>& V, QVector<QVector<double>>& H,
                       int m, QVector<double>& fVector);

    /** @brief Sparse matrix-vector multiply */
    QVector<double> spmv(const QVector<double>& values,
                         const QVector<int>& colIdx,
                         const QVector<int>& rowPtr,
                         const QVector<double>& x) const;

    /** @brief QR shift and restart: implicit QR step on H */
    void implicitQRShift(QVector<QVector<double>>& H,
                         QVector<QVector<double>>& V, int m,
                         double shiftRe, double shiftIm);

    /** @brief Extract Schur vectors and convert to eigenvectors */
    void extractSchurVectors(const QVector<QVector<double>>& H,
                             const QVector<QVector<double>>& V,
                             QVector<EigenPair>& pairs);

    /** @brief Compute residual norm for convergence check */
    double residualNorm(const QVector<QVector<double>>& H,
                        const QVector<double>& f, int k) const;

    /** @brief Dense matrix-vector multiply */
    QVector<double> denseMV(const QVector<QVector<double>>& A,
                            const QVector<double>& x) const;

    /** @brief Normalize a vector, returns norm */
    double normalize(QVector<double>& v) const;
};
