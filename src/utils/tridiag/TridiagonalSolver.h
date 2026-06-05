/**
 * @file TridiagonalSolver.h
 * @brief Thomas 算法 — 三对角线性方程组求解
 *
 * 功能: 以 O(n) 时间复杂度求解三对角方程 Ax = rhs，
 *       适用于样条插值、有限差分法 PDE 离散等场景。
 *
 * 协作: BandMatrixSolver(带状矩阵) / SplineInterpolation(样条)
 */
#ifndef TRIDIAGONALSOLVER_H
#define TRIDIAGONALSOLVER_H

#include <QObject>
#include <QVector>

/**
 * @brief 三对角方程组求解器(Thomas 算法)
 */
class TridiagonalSolver : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSolves = 0;          ///< 累计求解次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit TridiagonalSolver(QObject* parent = nullptr);

    /**
     * @brief 求解三对角方程组
     * @param lower 下对角线(长度 n-1)
     * @param mainDiag 主对角线(长度 n)
     * @param upper 上对角线(长度 n-1)
     * @param rhs 右端向量(长度 n)
     * @return 解向量(长度 n)
     */
    QVector<double> solve(QVector<double> lower,
                          QVector<double> mainDiag,
                          QVector<double> upper,
                          QVector<double> rhs);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成 @param size 方程组规模 */
    void solveCompleted(int size);

private:
    Stats  m_stats;          ///< 统计信息
    double m_timeSum = 0.0;  ///< 累计耗时
};

#endif // TRIDIAGONALSOLVER_H
