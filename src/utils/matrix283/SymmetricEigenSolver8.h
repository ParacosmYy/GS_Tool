/**
 * @file SymmetricEigenSolver8.h
 * @brief 对称特征值求解(二分逆迭代与Sturm序列计数的选择性特征值计算) — Symmetric Eigenvalue Solver with Bisection Inverse Iteration and Sturm Sequence Counting for Selective Eigenvalue Computation
 *
 * 功能: 实现对称特征值求解(symmetric eigenvalue solver)，采用二分逆迭代(bisection inverse iteration)
 *       与Sturm序列计数(Sturm sequence counting)实现选择性特征值计算(selective eigenvalue computation)。
 *
 * 协作: CholeskyDecomp7(Cholesky分解) / SvdSolver9(SVD求解) / LUDecomp6(LU分解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 对称特征值求解(二分逆迭代与Sturm序列计数)
 */
class SymmetricEigenSolver8 : public QObject {
    Q_OBJECT

public:
    /** @brief Eigen decomposition result */
    struct EigenResult {
        QVector<double> eigenvalues;
        QVector<QVector<double>> eigenvectors;  // Columns = eigenvectors
        int numEigen = 0;
        int iterations = 0;
        bool converged = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int eigenvaluesComputed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SymmetricEigenSolver8(QObject *parent = nullptr);
    ~SymmetricEigenSolver8() override;

    /** @brief Set convergence tolerance */
    void setTolerance(double tol);

    /** @brief Set max iterations for inverse iteration */
    void setMaxIterations(int iter);

    /** @brief Compute all eigenvalues and eigenvectors */
    EigenResult solveAll(const QVector<QVector<double>>& matrix);

    /** @brief Compute eigenvalues in range [lo, hi] using Sturm bisection */
    EigenResult solveRange(const QVector<QVector<double>>& matrix,
                            double lo, double hi);

    /** @brief Count eigenvalues less than x using Sturm sequence */
    int sturmCount(const QVector<double>& diag, const QVector<double>& offDiag,
                    double x) const;

    /** @brief Tridiagonalize symmetric matrix via Householder */
    void tridiagonalize(const QVector<QVector<double>>& matrix,
                         QVector<double>& diag, QVector<double>& offDiag) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void eigenvalueFound(int idx, double value);
    void solveComplete(int n, int count, double timeMs);

private:
    double m_tolerance = 1e-10;
    int m_maxIter = 100;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Find k-th eigenvalue by bisection using Sturm count */
    double bisectEigenvalue(const QVector<double>& diag,
                             const QVector<double>& offDiag,
                             int k, double lo, double hi) const;

    /** @brief Inverse iteration to find eigenvector for given eigenvalue */
    QVector<double> inverseIteration(const QVector<double>& diag,
                                      const QVector<double>& offDiag,
                                      double eigenvalue) const;

    /** @brief Compute Gershgorin bounds for eigenvalue range */
    void gershgorinBounds(const QVector<double>& diag,
                           const QVector<double>& offDiag,
                           double& lo, double& hi) const;

    /** @brief Solve tridiagonal system T * x = b */
    QVector<double> solveTridiag(const QVector<double>& diag,
                                  const QVector<double>& offDiag,
                                  const QVector<double>& rhs) const;
};
