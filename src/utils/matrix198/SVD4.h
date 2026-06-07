/**
 * @file SVD4.h
 * @brief 奇异值分解(单侧Jacobi旋转+高精度+条件数估计) — SVD with One-Sided Jacobi Rotation for High-Accuracy Computation and Condition Estimation
 *
 * 功能: 实现基于单侧Jacobi旋转的SVD算法，支持高精度奇异值计算、
 *       条件数估计和左/右奇异向量恢复。
 *
 * 协作: EigenSolver5(特征值) / QRDecomp4(QR分解) / LeastSquares4(最小二乘)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 奇异值分解(单侧Jacobi+条件数估计)
 */
class SVD4 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalDecomps = 0;
        int rows = 0;
        int cols = 0;
        int rank = 0;
        double conditionNumber = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SVD4(QObject *parent = nullptr);
    ~SVD4() override;

    void setMaxIterations(int iter);
    void setTolerance(double tol);

    /** @brief Compute SVD of m×n matrix (row-major) */
    bool compute(const QVector<QVector<double>>& matrix);

    /** @brief Get singular values (descending) */
    QVector<double> singularValues() const;

    /** @brief Get left singular vectors (U) */
    QVector<QVector<double>> matrixU() const;

    /** @brief Get right singular vectors (V) */
    QVector<QVector<double>> matrixV() const;

    /** @brief Estimate condition number (max_sv / min_sv) */
    double conditionNumber() const;

    /** @brief Compute effective numerical rank */
    int rank() const;

    /** @brief Reconstruct matrix from SVD */
    QVector<QVector<double>> reconstruct() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decompositionCompleted(int rank, double cond, double timeMs);

private:
    int m_maxIter = 200;
    double m_tol = 1e-12;

    int m_rows = 0, m_cols = 0;
    QVector<double> m_sigma;           // singular values
    QVector<QVector<double>> m_U;      // left singular vectors
    QVector<QVector<double>> m_V;      // right singular vectors

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief One-sided Jacobi sweep on columns of B = A */
    bool jacobiSweep(QVector<QVector<double>>& B,
                     QVector<QVector<double>>& V);

    /** @brief Compute 2-norm of a column */
    static double colNorm(const QVector<QVector<double>>& M, int col);

    /** @brief Dot product of two columns */
    static double colDot(const QVector<QVector<double>>& M,
                         int c1, int c2);
};
