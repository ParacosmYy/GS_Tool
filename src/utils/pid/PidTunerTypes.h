/**
 * @file PidTunerTypes.h
 * @brief PID调试器基础类型定义 — 参数、响应类型、仿真结果结构体
 * @author EmbedDebug Team
 * @date 2026-06-06
 *
 * PidSimulator 与 PidTunerWidget 共用的数据结构。
 * 独立头文件，无 QObject 依赖，便于单元测试引用。
 */

#ifndef PIDTUNERTYPES_H
#define PIDTUNERTYPES_H

#include <QVector>
#include <QString>

/**
 * @brief PID控制器参数集合
 */
struct PidParams {
    double kp         = 1.0;    ///< 比例增益
    double ki         = 0.0;    ///< 积分增益
    double kd         = 0.0;    ///< 微分增益
    double setpoint   = 1.0;    ///< 目标设定值
    double outputMin  = -100.0; ///< 输出下限
    double outputMax  =  100.0; ///< 输出上限
    double sampleTime = 0.01;   ///< 采样周期(秒)
};

/**
 * @brief 激励信号类型
 */
enum class PidResponseType {
    Step,   ///< 阶跃响应
    Ramp,   ///< 斜坡响应
    Sine,   ///< 正弦响应
    Square, ///< 方波响应
    Impulse ///< 脉冲响应
};

/**
 * @brief PID仿真结果 — 时序数据 + 性能指标
 */
struct PidResponse {
    QVector<double> timePoints;         ///< 时间轴(秒)
    QVector<double> setpointData;       ///< 设定值序列
    QVector<double> processData;        ///< 过程变量序列
    QVector<double> outputData;         ///< 控制器输出序列
    QVector<double> errorData;          ///< 误差序列

    double riseTime            = 0.0;   ///< 上升时间(秒)
    double settlingTime        = 0.0;   ///< 调节时间(秒)
    double overshoot           = 0.0;   ///< 超调量(百分比)
    double steadyStateError    = 0.0;   ///< 稳态误差
    double integralAbsoluteError = 0.0; ///< 绝对误差积分(IAE)
};

#endif // PIDTUNERTYPES_H
