/**
 * @file NewtonRaphson.h
 * @brief Newton-Raphson根求解引擎 — 解析/数值微分根查找
 *
 * 功能: 使用Newton-Raphson迭代法求解非线性方程f(x)=0的根。
 *       支持解析导数(精确)和数值差分(近似)两种模式，
 *       内置收敛检测和迭代上限保护。
 *
 * 协作: HornerScheme(多项式求导) / GaussLegendre(积分方程根)
 */
#ifndef NEWTONRAPHSON_H
#define NEWTONRAPHSON_H

#include <QObject>
#include <functional>

/**
 * @brief Newton-Raphson根求解引擎
 */
class NewtonRaphson : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalSolves = 0;            ///< 累计求解次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit NewtonRaphson(QObject* parent = nullptr);

    /**
     * @brief 使用解析导数求解f(x)=0
     * @param f 目标函数
     * @param df 目标函数的解析导数
     * @param x0 初始猜测值
     * @param tol 收敛容差(默认1e-10)
     * @param maxIter 最大迭代次数(默认100)
     * @return 求得的根
     */
    double solve(std::function<double(double)> f,
                 std::function<double(double)> df,
                 double x0,
                 double tol = 1e-10,
                 int maxIter = 100);

    /**
     * @brief 使用数值差分近似导数求解f(x)=0
     * @param f 目标函数
     * @param x0 初始猜测值
     * @param tol 收敛容差(默认1e-10)
     * @param maxIter 最大迭代次数(默认100)
     * @param h 差分步长(默认1e-7)
     * @return 求得的根
     */
    double solveNumerical(std::function<double(double)> f,
                          double x0,
                          double tol = 1e-10,
                          int maxIter = 100,
                          double h = 1e-7);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 找到根信号 @param root 根值 @param iterations 迭代次数 */
    void rootFound(double root, int iterations);

private:
    /**
     * @brief 核心迭代求解
     * @param f 目标函数
     * @param df 导数函数
     * @param x0 初始值
     * @param tol 容差
     * @param maxIter 最大迭代次数
     * @return QPair(根, 迭代次数)
     */
    QPair<double, int> iterate(std::function<double(double)> f,
                                std::function<double(double)> df,
                                double x0, double tol, int maxIter);

    Stats m_stats;              ///< 统计信息
    double m_timeSumMs;         ///< 累计耗时(ms)
};

#endif // NEWTONRAPHSON_H
