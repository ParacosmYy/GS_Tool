/**
 * @file TridiagonalSolver.h
 * @brief 三对角线性方程组求解器 — Thomas算法
 *
 * 功能: 使用Thomas算法高效求解三对角方程组，复杂度O(N)。
 *       支持循环三对角系统(Sherman-Morrison公式)。
 *
 * 协作: BandMatrix(带状矩阵求解) / DataInterpolator(样条插值)
 * @author Serial Tool Team
 * @date 2026-06-05
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QtGlobal>

/**
 * @class TridiagonalSolver
 * @brief 三对角线性方程组求解器
 *
 * 使用Thomas算法(追赶法)求解标准三对角方程组，
 * 以及Sherman-Morrison公式求解循环三对角方程组。
 * 所有对角线向量长度必须一致，否则返回空解。
 */
class TridiagonalSolver : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息结构体 */
    struct Stats {
        quint64 totalSolves = 0;            ///< 累计求解次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit TridiagonalSolver(QObject *parent = nullptr);

    /**
     * @brief 求解标准三对角方程组 (Thomas算法)
     * @param lower 下对角线向量(长度 n-1 或 n，首元素不用)
     * @param main 主对角线向量(长度 n)
     * @param upper 上对角线向量(长度 n-1 或 n，末元素不用)
     * @param rhs 右端项向量(长度 n)
     * @return 解向量，失败返回空
     */
    QVector<double> solve(const QVector<double> &lower,
                          const QVector<double> &main,
                          const QVector<double> &upper,
                          const QVector<double> &rhs);

    /**
     * @brief 求解循环三对角方程组 (Sherman-Morrison公式)
     * @param lower 下对角线向量(长度 n，lower[0]为矩阵左下角元素)
     * @param main 主对角线向量(长度 n)
     * @param upper 上对角线向量(长度 n，upper[n-1]为矩阵右上角元素)
     * @param rhs 右端项向量(长度 n)
     * @return 解向量，失败返回空
     */
    QVector<double> solveCyclic(const QVector<double> &lower,
                                const QVector<double> &main,
                                const QVector<double> &upper,
                                const QVector<double> &rhs);

    /** @brief 获取统计信息 @return 常量引用 */
    const Stats &stats() const { return m_stats; }

    /** @brief 重置统计计数器 */
    void resetStatistics();

signals:
    /** @brief 求解完成信号 @param systemSize 方程组规模 @param cyclic 是否循环系统 */
    void solveCompleted(int systemSize, bool cyclic);

private:
    Stats m_stats;          ///< 统计信息
    double m_timeSum = 0.0; ///< 处理时间累加器
};
