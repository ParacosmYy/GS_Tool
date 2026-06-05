/**
 * @file PredictorCorrector.h
 * @brief 预估-校正法 — Adams-Bashforth-Moulton多步ODE求解
 *
 * 功能: 使用AB4预估+AM3校正的PECE格式求解ODE初值问题，
 *       比纯显式法精度更高且稳定性更好。
 *
 * 协作: AdamsBashforth(显式多步) / RungeKuttaSolver(RK启动)
 */
#ifndef PREDICTORCORRECTOR_H
#define PREDICTORCORRECTOR_H

#include <QObject>
#include <QVector>
#include <QPair>
#include <functional>

/**
 * @brief 预估-校正ODE求解器(AB4-AM3 PECE)
 */
class PredictorCorrector : public QObject {
    Q_OBJECT

public:
    /** @brief ODE右端函数类型 */
    using OdeFunc = std::function<double(double, double)>;

    /** @brief 统计 */
    struct Stats {
        quint64 totalSolves = 0;            ///< 累计求解次数
        quint64 totalSteps = 0;             ///< 累计步数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit PredictorCorrector(QObject* parent = nullptr);

    /**
     * @brief 求解ODE y' = f(t,y)
     * @param f 右端函数
     * @param y0 初值
     * @param t0 起始时间
     * @param tf 终止时间
     * @param h 步长
     * @return (t, y)解序列
     */
    QVector<QPair<double, double>> solve(OdeFunc f, double y0,
                                          double t0, double tf,
                                          double h);

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

#endif // PREDICTORCORRECTOR_H
