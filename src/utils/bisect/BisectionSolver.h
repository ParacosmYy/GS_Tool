/**
 * @file BisectionSolver.h
 * @brief 二分法/割线法/试位法 — 非线性方程求根
 *
 * 功能: 提供三种经典一元方程求根方法:
 *       二分法(保证收敛)、割线法(超线性)、试位法(改进二分)。
 *
 * 协作: NewtonRaphson(牛顿法) / LaguerreSolver(多项式求根)
 */
#ifndef BISECTIONSOLVER_H
#define BISECTIONSOLVER_H

#include <QObject>
#include <functional>

/**
 * @brief 二分法/割线法/试位法求根器
 */
class BisectionSolver : public QObject {
    Q_OBJECT

public:
    /** @brief 一元函数类型 */
    using Func = std::function<double(double)>;

    /** @brief 统计 */
    struct Stats {
        quint64 totalSolves = 0;            ///< 累计求解次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit BisectionSolver(QObject* parent = nullptr);

    /**
     * @brief 二分法求根(保证收敛)
     * @param f 目标函数
     * @param a 区间左端点
     * @param b 区间右端点
     * @param tol 收敛容差
     * @param maxIter 最大迭代次数
     * @return 近似根
     */
    double bisection(Func f, double a, double b,
                     double tol = 1e-12, int maxIter = 100);

    /**
     * @brief 割线法求根(超线性收敛)
     * @param f 目标函数
     * @param x0 第一个初始点
     * @param x1 第二个初始点
     * @param tol 收敛容差
     * @param maxIter 最大迭代次数
     * @return 近似根
     */
    double secant(Func f, double x0, double x1,
                  double tol = 1e-12, int maxIter = 100);

    /**
     * @brief 试位法求根(改进二分)
     * @param f 目标函数
     * @param a 区间左端点
     * @param b 区间右端点
     * @param tol 收敛容差
     * @param maxIter 最大迭代次数
     * @return 近似根
     */
    double falsePosition(Func f, double a, double b,
                         double tol = 1e-12, int maxIter = 100);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求根完成 @param root 近似根 @param iterations 迭代次数 */
    void rootFound(double root, int iterations);

private:
    Stats m_stats;              ///< 统计信息
    double m_timeSum = 0.0;     ///< 累计耗时
};

#endif // BISECTIONSOLVER_H
