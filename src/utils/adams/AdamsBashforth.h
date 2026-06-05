/**
 * @file AdamsBashforth.h
 * @brief Adams-Bashforth多步法 — 常微分方程数值求解
 *
 * 功能: 支持1~4阶Adams-Bashforth显式多步法求解ODE初值问题，
 *       使用RK4作为启动步骤。
 *
 * 协作: RungeKuttaSolver(RK启动) / PredictorCorrector(预估-校正)
 */
#ifndef ADAMSBASHFORTH_H
#define ADAMSBASHFORTH_H

#include <QObject>
#include <QVector>
#include <QPair>
#include <functional>

/**
 * @brief Adams-Bashforth多步ODE求解器
 */
class AdamsBashforth : public QObject {
    Q_OBJECT

public:
    /** @brief ODE右端函数类型 f(t,y) */
    using OdeFunc = std::function<double(double, double)>;

    /** @brief 统计 */
    struct Stats {
        quint64 totalSolves = 0;            ///< 累计求解次数
        quint64 totalSteps = 0;             ///< 累计步数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit AdamsBashforth(QObject* parent = nullptr);

    /**
     * @brief 求解ODE y' = f(t,y)
     * @param f 右端函数
     * @param y0 初值
     * @param t0 起始时间
     * @param tf 终止时间
     * @param h 步长
     * @param order 阶数(1~4)
     * @return (t, y)解序列
     */
    QVector<QPair<double, double>> solve(OdeFunc f, double y0,
                                          double t0, double tf,
                                          double h, int order = 4);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成 @param steps 总步数 @param finalTime 最终时间 */
    void solveCompleted(int steps, double finalTime);

private:
    /** @brief RK4单步(用于启动) */
    double rk4Step(OdeFunc f, double t, double y, double h) const;

    Stats m_stats;              ///< 统计信息
    double m_timeSum = 0.0;     ///< 累计耗时
};

#endif // ADAMSBASHFORTH_H
