/**
 * @file Rk2Solver.h
 * @brief 二阶Runge-Kutta求解器 — Midpoint/Ralston方法
 *
 * 功能: 提供两种二阶Runge-Kutta方法(Midpoint法和Ralston法)
 *       求解常微分方程初值问题 y' = f(t, y)。
 *       Midpoint法具有更好的对称性，Ralston法具有最小截断误差界。
 *
 * 协作: DataInterpolator(解插值) / WaveformGenerator(波形ODE求解)
 */
#ifndef RK2SOLVER_H
#define RK2SOLVER_H

#include <QObject>
#include <QVector>
#include <QPair>
#include <functional>

/**
 * @brief 二阶Runge-Kutta求解器
 */
class Rk2Solver : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalSolves = 0;            ///< 累计求解次数
        quint64 totalSteps = 0;             ///< 累计步数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit Rk2Solver(QObject* parent = nullptr);

    /**
     * @brief Midpoint法求解ODE
     * @param f 右端函数 f(t, y)
     * @param y0 初始值y(t0)
     * @param t0 初始时间
     * @param tf 终止时间
     * @param h 步长(>0)
     * @return 解的轨迹 QVector<QPair(t, y)>
     *
     * Midpoint法(RK2中点法):
     *   k1 = f(t_n, y_n)
     *   k2 = f(t_n + h/2, y_n + h/2 * k1)
     *   y_{n+1} = y_n + h * k2
     * 局部截断误差: O(h³)
     */
    QVector<QPair<double, double>> midpoint(
        std::function<double(double, double)> f,
        double y0, double t0, double tf, double h);

    /**
     * @brief Ralston法求解ODE
     * @param f 右端函数 f(t, y)
     * @param y0 初始值y(t0)
     * @param t0 初始时间
     * @param tf 终止时间
     * @param h 步长(>0)
     * @return 解的轨迹 QVector<QPair(t, y)>
     *
     * Ralston法(最小误差界RK2):
     *   k1 = f(t_n, y_n)
     *   k2 = f(t_n + 2h/3, y_n + 2h/3 * k1)
     *   y_{n+1} = y_n + h/4 * (k1 + 3*k2)
     * 局部截断误差: O(h³)，但误差常数小于Midpoint法
     */
    QVector<QPair<double, double>> ralston(
        std::function<double(double, double)> f,
        double y0, double t0, double tf, double h);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 求解完成信号 @param steps 总步数 @param finalTime 终止时间 */
    void solveCompleted(int steps, double finalTime);

private:
    Stats m_stats;              ///< 统计信息
    double m_timeSumMs;         ///< 累计耗时(ms)
};

#endif // RK2SOLVER_H
