/**
 * @file TridiagonalSolver.h
 * @brief 三对角矩阵求解器(Thomas算法+周期边界变体) — Thomas Algorithm for Tridiagonal Systems with Periodic Boundary Condition Variant
 *
 * 功能: 实现Thomas算法求解三对角线性方程组，支持标准形式和
 *       周期性边界条件(Sherman-Morrison)变体。
 *
 * 协作: GaussianElim4(高斯消元) / SvdSolver3(SVD) / SparseSolver2(稀疏求解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 三对角方程组求解器
 */
class TridiagonalSolver : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSolves = 0;          ///< 累计求解次数
        double avgProcessingTimeMs = 0.0; ///< 平均耗时(ms)
        int lastSize = 0;                 ///< 最近方程组大小
        bool lastPeriodic = false;        ///< 最近是否周期边界
    };

    explicit TridiagonalSolver(QObject *parent = nullptr);
    ~TridiagonalSolver() override;

    /**
     * @brief 求解标准三对角系统 Ax=d
     * @param lower 下对角线(n-1个元素)
     * @param main 主对角线(n个元素)
     * @param upper 上对角线(n-1个元素)
     * @param rhs 右端向量(n个元素)
     * @return 解向量
     */
    QVector<double> solve(const QVector<double>& lower,
                          const QVector<double>& main,
                          const QVector<double>& upper,
                          const QVector<double>& rhs);

    /**
     * @brief 求解周期边界三对角系统
     * @param lower 下对角线(n个元素, lower[0]为A[n-1,0])
     * @param main 主对角线(n个元素)
     * @param upper 上对角线(n个元素, upper[n-1]为A[0,n-1])
     * @param rhs 右端向量(n个元素)
     * @return 解向量
     */
    QVector<double> solvePeriodic(const QVector<double>& lower,
                                   const QVector<double>& main,
                                   const QVector<double>& upper,
                                   const QVector<double>& rhs);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int size, bool periodic);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
};
