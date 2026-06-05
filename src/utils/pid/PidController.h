/**
 * @file PidController.h
 * @brief PID控制器 — 经典比例/积分/微分控制算法
 *
 * 功能: 支持位置式/增量式PID，抗积分饱和，微分滤波，
 *       统计运行次数/稳态误差/超调量/调节时间。
 */
#ifndef PIDCONTROLLER_H
#define PIDCONTROLLER_H

#include <QObject>
#include <QVector>

/**
 * @class PidController
 * @brief 通用PID控制器，适用于串口设备参数闭环调节
 */
class PidController : public QObject {
    Q_OBJECT
public:
    /** PID模式 */
    enum class Mode {
        Positional, ///< 位置式PID
        Incremental ///< 增量式PID
    };

    /** PID参数 */
    struct PidParams {
        double kp = 1.0;       ///< 比例增益
        double ki = 0.0;       ///< 积分增益
        double kd = 0.0;       ///< 微分增益
        double outputMin = -100.0; ///< 输出下限
        double outputMax = 100.0;  ///< 输出上限
    };

    /** 运行统计 */
    struct Stats {
        quint64 totalUpdates = 0;        ///< 总更新次数
        double  steadyStateError = 0.0;  ///< 稳态误差
        double  overshoot = 0.0;         ///< 超调量(%)
        double  settlingTimeMs = 0.0;    ///< 调节时间
        double  averageProcessingTimeMs = 0.0;
    };

    explicit PidController(QObject* parent = nullptr);

    /** 设置参数 */
    void setParams(const PidParams& params);
    void setMode(Mode m);
    void setSetpoint(double sp);
    void setIntegralLimit(double limit);
    void setDerivativeFilter(double alpha);

    /** 计算PID输出 */
    double update(double processVariable, double dtMs);

    /** 重置控制器状态 */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void outputComputed(double output, double error);
    void setpointChanged(double newSetpoint);

private:
    PidParams m_params;
    Mode m_mode;
    double m_setpoint;
    double m_integral;
    double m_prevError;
    double m_prevOutput;
    double m_integralLimit;
    double m_derivFilterAlpha;
    double m_filteredDerivative;
    bool m_firstUpdate;
    Stats m_stats;
    double m_timeSum;
};

#endif // PIDCONTROLLER_H
