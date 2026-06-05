/**
 * @file RungeKuttaSolver.h
 * @brief 常微分方程(ODE)求解器 — 四阶Runge-Kutta法
 *
 * 功能: 提供RK4和RK45自适应步长求解常微分方程组。
 *       支持单步推进和区间积分，带有误差估计。
 *
 * 协作: 依赖数学函数库，用于物理仿真和信号建模
 */
#ifndef RUNGEKUTTASOLVER_H
#define RUNGEKUTTASOLVER_H

#include <QObject>
#include <QVector>
#include <functional>

/**
 * @brief 常微分方程求解器 — RK4/RK45
 */
class RungeKuttaSolver : public QObject {
    Q_OBJECT

public:
    /** @brief ODE系统函数类型: f(t, y) -> dy/dt */
    using OdeFunc = std::function<QVector<double>(double, const QVector<double>&)>;

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalSteps = 0;          ///< 累计步数
        quint64 totalRejected = 0;        ///< 被拒绝步数(自适应)
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    /** @brief 积分结果 */
    struct SolveResult {
        QVector<double> tValues;          ///< 时间序列
        QVector<QVector<double>> yValues; ///< 状态序列
        int acceptedSteps = 0;            ///< 接受的步数
        int rejectedSteps = 0;            ///< 拒绝的步数
    };

    explicit RungeKuttaSolver(QObject* parent = nullptr);

    /** @brief RK4单步推进
     *  @param func ODE右端函数
     *  @param t 当前时间
     *  @param y 当前状态
     *  @param dt 步长
     *  @return 下一步状态 */
    QVector<double> step(const OdeFunc& func, double t,
                         const QVector<double>& y, double dt);

    /** @brief RK4区间积分
     *  @param func ODE右端函数
     *  @param t0 起始时间
     *  @param t1 终止时间
     *  @param y0 初始状态
     *  @param dt 步长
     *  @return 积分结果 */
    SolveResult solve(const OdeFunc& func, double t0, double t1,
                      const QVector<double>& y0, double dt);

    /** @brief RK45自适应步长积分
     *  @param func ODE右端函数
     *  @param t0 起始时间
     *  @param t1 终止时间
     *  @param y0 初始状态
     *  @param dtInit 初始步长
     *  @param tol 误差容限
     *  @return 积分结果 */
    SolveResult solveAdaptive(const OdeFunc& func, double t0, double t1,
                              const QVector<double>& y0,
                              double dtInit, double tol);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 单步完成 @param stepIndex 步序号 @param t 当前时间 */
    void stepCompleted(int stepIndex, double t);

    /** @brief 积分完成 @param steps 总步数 */
    void solveCompleted(int steps);

private:
    /** @brief RK45单步(Dormand-Prince) @return (y5, y4) 用于误差估计 */
    QPair<QVector<double>, QVector<double>> step45(
        const OdeFunc& func, double t,
        const QVector<double>& y, double dt);

    double m_timeSum;   ///< 处理时间累加器
    Stats m_stats;      ///< 统计信息
};

#endif // RUNGEKUTTASOLVER_H
