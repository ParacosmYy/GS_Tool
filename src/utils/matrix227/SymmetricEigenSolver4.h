/**
 * @file SymmetricEigenSolver4.h
 * @brief 对称特征值求解器(二分Sturm序列选定特征值计算) — Symmetric Eigensolver with Bisection Sturm Sequence for Selected Eigenvalue Computation
 *
 * 功能: 实现对称三对角矩阵特征值求解，使用Sturm序列计数
 *       结合二分法精确计算指定范围内的特征值。
 *
 * 协作: Householder3(Householder变换) / QRDecomposition5(QR分解) / SvdSolver3(SVD求解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 对称特征值求解器(Sturm二分)
 */
class SymmetricEigenSolver4 : public QObject {
    Q_OBJECT

public:
    /** @brief Eigen solution result */
    struct EigenResult {
        QVector<double> eigenvalues;
        QVector<QVector<double>> eigenvectors;
        int matrixSize = 0;
        int numEigenvalues = 0;
        int iterations = 0;
        double residual = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int numEigenvalues = 0;
        int maxIterations = 0;
        double tolerance = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SymmetricEigenSolver4(QObject *parent = nullptr);
    ~SymmetricEigenSolver4() override;

    /** @brief Set parameters: tolerance, max iterations */
    void setParameters(double tolerance = 1e-10, int maxIter = 100);

    /** @brief Compute all eigenvalues of symmetric tridiagonal matrix */
    EigenResult solveTridiagonal(const QVector<double>& diag,
                                   const QVector<double>& offDiag);

    /** @brief Compute eigenvalues in range [lo, hi] */
    EigenResult solveRange(const QVector<double>& diag,
                             const QVector<double>& offDiag,
                             double lo, double hi);

    /** @brief Reduce symmetric matrix to tridiagonal via Householder */
    void tridiagonalize(const QVector<QVector<double>>& matrix,
                          QVector<double>& diag,
                          QVector<double>& offDiag,
                          QVector<QVector<double>>& transform) const;

    /** @brief Sturm sequence count: eigenvalues less than x */
    int sturmCount(const QVector<double>& diag,
                    const QVector<double>& offDiag, double x) const;

    /** @brief Inverse iteration for eigenvector computation */
    QVector<double> inverseIteration(const QVector<double>& diag,
                                       const QVector<double>& offDiag,
                                       double eigenvalue, int maxIter = 50) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int numEigenvalues, int iterations, double timeMs);

private:
    double m_tolerance = 1e-10;
    int m_maxIter = 100;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Bisection to find eigenvalue at index k */
    double bisect(const QVector<double>& diag,
                    const QVector<double>& offDiag,
                    int k, double lo, double hi) const;

    /** @brief Gershgorin bounds for eigenvalue range */
    QPair<double, double> gershgorinBounds(const QVector<double>& diag,
                                             const QVector<double>& offDiag) const;
};
