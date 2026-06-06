/**
 * @file Cholesky3.h
 * @brief Cholesky分解(带状存储+半正定矩阵主元变体) — Cholesky Factorization with Band-structured Storage and Pivoted Variant for Semidefinite Matrices
 *
 * 功能: 实现Cholesky分解算法，支持带状存储优化、半正定矩阵的主元变体、
 *       矩阵方程求解、行列式计算和条件数估计。
 *
 * 协作: LU Decomposition(LU) / QR Decomposition(QR) / SVD Solver(奇异值)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Cholesky分解器(带状存储+主元变体)
 */
class Cholesky3 : public QObject {
    Q_OBJECT

public:
    /** @brief Decomposition mode */
    enum class Mode { Standard, Pivoted, Banded };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalDecompositions = 0;
        int matrixSize = 0;
        int bandwidth = 0;
        int rank = 0;
        bool isPositiveDefinite = false;
        double logDeterminant = 0.0;
        double conditionEstimate = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Cholesky3(QObject *parent = nullptr);
    ~Cholesky3() override;

    void setMode(Mode mode);
    void setBandwidth(int bw);
    void setPivotTolerance(double tol);

    /** @brief Standard Cholesky: A = L * L^T */
    bool decompose(const QVector<QVector<double>>& A);

    /** @brief Banded Cholesky: A = L * L^T with bandwidth optimization */
    bool decomposeBanded(const QVector<QVector<double>>& A, int bandwidth);

    /** @brief Pivoted Cholesky for semidefinite matrices */
    bool decomposePivoted(const QVector<QVector<double>>& A);

    /** @brief Solve A * x = b using existing factorization */
    QVector<double> solve(const QVector<double>& b) const;

    /** @brief Solve A * X = B (multiple RHS) */
    QVector<QVector<double>> solveMulti(
        const QVector<QVector<double>>& B) const;

    /** @brief Compute log determinant from factorization */
    double logDeterminant() const;

    /** @brief Estimate condition number */
    double conditionEstimate() const;

    /** @brief Get lower triangular factor L */
    QVector<QVector<double>> factorL() const;

    /** @brief Get permutation (pivoted mode) */
    QVector<int> permutation() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decompositionCompleted(int size, bool positiveDefinite, double logDet);

private:
    Mode m_mode = Mode::Standard;
    int m_bandwidth = 0;
    double m_pivotTol = 1e-10;

    QVector<QVector<double>> m_L;
    QVector<int> m_pivot;       ///< Pivot indices (pivoted mode)
    int m_n = 0;
    int m_rank = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Forward solve L * y = b */
    QVector<double> forwardSolve(const QVector<double>& b) const;

    /** @brief Backward solve L^T * x = y */
    QVector<double> backwardSolve(const QVector<double>& y) const;

    /** @brief Apply permutation */
    QVector<double> applyPermutation(const QVector<double>& v) const;

    /** @brief Inverse permutation */
    QVector<double> applyInversePermutation(const QVector<double>& v) const;
};
