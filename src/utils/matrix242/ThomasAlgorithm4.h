/**
 * @file ThomasAlgorithm4.h
 * @brief Thomas算法三对角求解(部分主元消元+向量化前后向代入) — Thomas Algorithm for Tridiagonal Systems with Partial Pivoting and Vectorized Forward-Backward Substitution
 *
 * 功能: 实现Thomas算法(Thomas algorithm)求解三对角线性方程组(tridiagonal systems)，
 *       采用部分主元消元(partial pivoting)增强数值稳定性，结合向量化前后向代入(vectorized
 *       forward-backward substitution)优化计算效率，支持批量求解多个同结构方程组。
 *
 * 协作: GaussianElimination5(高斯消元) / LUDecomposition6(LU分解) / Cholesky7(Cholesky分解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Thomas算法三对角求解(部分主元消元+向量化前后向代入)
 */
class ThomasAlgorithm4 : public QObject {
    Q_OBJECT

public:
    /** @brief Tridiagonal system coefficients */
    struct TridiagonalSystem {
        QVector<double> lower;   // sub-diagonal a[1..n-1]
        QVector<double> main;    // main diagonal b[0..n-1]
        QVector<double> upper;   // super-diagonal c[0..n-2]
        QVector<double> rhs;     // right-hand side d[0..n-1]
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int systemSize = 0;
        int numSolves = 0;
        int numPivotSwaps = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ThomasAlgorithm4(QObject *parent = nullptr);
    ~ThomasAlgorithm4() override;

    /** @brief Solve a single tridiagonal system Ax = d */
    QVector<double> solve(const TridiagonalSystem& system);

    /** @brief Solve with in-place modification of system */
    QVector<double> solveInPlace(TridiagonalSystem& system);

    /** @brief Batch solve multiple systems with same matrix structure */
    QVector<QVector<double>> solveBatch(const QVector<TridiagonalSystem>& systems);

    /** @brief Solve periodic tridiagonal system (Sherman-Morrison) */
    QVector<double> solvePeriodic(const TridiagonalSystem& system,
                                  double alpha, double beta);

    /** @brief Verify solution: compute residual norm */
    double residualNorm(const TridiagonalSystem& system,
                        const QVector<double>& x) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int n, int pivotSwaps, double timeMs);
    void batchCompleted(int count, double totalTimeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Forward elimination with partial pivoting */
    void forwardElimination(QVector<double>& a, QVector<double>& b,
                            QVector<double>& c, QVector<double>& d,
                            int& pivotSwaps) const;

    /** @brief Vectorized backward substitution */
    QVector<double> backwardSubstitution(const QVector<double>& b,
                                         const QVector<double>& c,
                                         const QVector<double>& d) const;
};
