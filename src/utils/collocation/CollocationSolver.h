/**
 * @file CollocationSolver.h
 * @brief 配置法求解器 — 边值问题数值求解
 *
 * 功能: 使用配置法(Collocation Method)求解两点边值问题:
 *       y'' = f(t, y, y'), a <= t <= b, y(a) = ya, y(b) = yb
 *       基于三次样条配置，在等距节点上构造近似解。
 *
 * 协作: Rk2Solver(初值问题对比验证) / DataInterpolator(解插值)
 */
#ifndef COLLOCATIONSOLVER_H
#define COLLOCATIONSOLVER_H

#include <QObject>
#include <QVector>
#include <QPair>
#include <functional>

/**
 * @brief 配置法求解器 — 边值问题
 */
class CollocationSolver : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalSolves = 0;            ///< 累计求解次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit CollocationSolver(QObject* parent = nullptr);

    /**
     * @brief 求解二阶边值问题 y'' = f(t, y, y')
     * @param f 右端函数 f(t, y, y')，t为自变量，y为函数值，y'为导数
     * @param a 区间左端点
     * @param b 区间右端点
     * @param ya 左边界条件 y(a)
     * @param yb 右边界条件 y(b)
     * @param n 网格点数(包括端点，最小4)
     * @return 解的轨迹 QVector<QPair(t, y)>
     *
     * 使用有限差分离散化 + Newton迭代求解非线性方程组。
     * 对于线性问题一次迭代即可收敛，非线性问题自动迭代至收敛。
     */
    QVector<QPair<double, double>> solve(
        std::function<double(double, double, double)> f,
        double a, double b, double ya, double yb, int n = 20);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 求解完成信号 @param meshPoints 网格点数 */
    void solveCompleted(int meshPoints);

private:
    /**
     * @brief Thomas算法求解三对角线性方程组
     * @param lower 下对角线(n-1个元素)
     * @param main 主对角线(n个元素)
     * @param upper 上对角线(n-1个元素)
     * @param rhs 右端向量(n个元素)
     * @return 解向量
     */
    QVector<double> thomasSolve(const QVector<double>& lower,
                                 const QVector<double>& mainDiag,
                                 const QVector<double>& upper,
                                 const QVector<double>& rhs);

    Stats m_stats;              ///< 统计信息
    double m_timeSumMs;         ///< 累计耗时(ms)
};

#endif // COLLOCATIONSOLVER_H
