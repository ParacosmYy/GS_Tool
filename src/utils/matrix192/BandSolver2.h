/**
 * @file BandSolver2.h
 * @brief 带状矩阵求解器(Thomas块分解+主元增长控制) — Band Solver for Banded Matrices with Thomas-Based Block Decomposition and Pivot Growth Control
 *
 * 功能: 实现带状矩阵线性系统求解器，支持Thomas算法块分解、
 *       部分主元选取、主元增长控制和多右端向量求解。
 *
 * 协作: LU-Decomposition4(LU分解) / Cholesky5(Cholesky分解) / TridiagonalSolver3(三对角求解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 带状矩阵求解器(Thomas块分解+主元增长控制)
 */
class BandSolver2 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSolves = 0;
        int matrixSize = 0;
        int bandwidth = 0;
        double pivotGrowth = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BandSolver2(QObject *parent = nullptr);
    ~BandSolver2() override;

    /** @brief 设置带状矩阵(紧凑存储: 每行存2*bandwidth+1个元素) */
    void setMatrix(int n, int halfBand,
                   const QVector<QVector<double>>& band);

    /** @brief 求解Ax=b */
    QVector<double> solve(const QVector<double>& rhs);

    /** @brief 求解多右端向量 */
    QVector<QVector<double>> solveMulti(
        const QVector<QVector<double>>& rhsSet);

    /** @brief 计算主元增长因子 */
    double pivotGrowthFactor() const { return m_pivotGrowth; }

    /** @brief 获取分解后的L/U带 */
    QVector<QVector<double>> factors() const { return m_factored; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int n, double pivotGrowth, double timeMs);

private:
    int m_n = 0;
    int m_halfBand = 0;
    bool m_factored = false;
    double m_pivotGrowth = 1.0;

    QVector<QVector<double>> m_band;    // Compact band storage
    QVector<QVector<double>> m_factoredBand; // LU factored form
    QVector<int> m_pivotIndices;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Thomas-based LU factorization with partial pivoting */
    void factorize();

    /** @brief Forward/backward substitution */
    QVector<double> substitute(const QVector<double>& rhs) const;
};
