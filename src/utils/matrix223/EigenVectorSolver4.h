/**
 * @file EigenVectorSolver4.h
 * @brief 特征向量求解器(隐式重启Arnoldi+精确位移选择移位QR) — Eigenvector Solver with Implicitly Restarted Arnoldi and Shifted QR with Exact Shift Selection
 *
 * 功能: 实现隐式重启Arnoldi迭代求解大规模稀疏矩阵的特征值和特征向量，
 *       通过精确位移选择的移位QR加速收敛。
 *
 * 协作: EigenDecomposition3(特征值分解) / SVD4(奇异值分解) / LinearSolver5(线性求解器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 特征向量求解器(隐式重启Arnoldi+移位QR)
 */
class EigenVectorSolver4 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int numEigenvalues = 0;
        int arnoldiIterations = 0;
        double residualNorm = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit EigenVectorSolver4(QObject *parent = nullptr);
    ~EigenVectorSolver4() override;

    /** @brief Set parameters: number of eigenvalues, Arnoldi basis size, max iterations, tolerance */
    void setParameters(int numEigenvalues = 6,
                       int arnoldiBasisSize = 20,
                       int maxIterations = 300,
                       double tolerance = 1e-10);

    /** @brief Solve for dominant eigenvalues and eigenvectors */
    void solve(const QVector<QVector<double>>& matrix);

    /** @brief Get computed eigenvalues */
    QVector<double> eigenvalues() const;

    /** @brief Get computed eigenvectors (columns) */
    QVector<QVector<double>> eigenvectors() const;

    /** @brief Compute residual norm for verification */
    double residualNorm(const QVector<QVector<double>>& matrix) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int numEigen, double residual, double timeMs);

private:
    int m_numEigen = 6;
    int m_basisSize = 20;
    int m_maxIter = 300;
    double m_tol = 1e-10;

    QVector<double> m_eigenvalues;
    QVector<QVector<double>> m_eigenvectors;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Matrix-vector multiplication */
    QVector<double> matVec(const QVector<QVector<double>>& A,
                            const QVector<double>& v) const;

    /** @brief Vector dot product */
    double dot(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Vector norm */
    double norm(const QVector<double>& v) const;

    /** @brief Implicitly restarted Arnoldi iteration */
    void arnoldiIRAM(const QVector<QVector<double>>& matrix);

    /** @brief QR shift selection: exact shifts from unwanted Ritz values */
    QVector<double> selectShifts(const QVector<double>& ritzValues,
                                  int numWanted) const;
};
