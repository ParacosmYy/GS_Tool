/**
 * @file PidSimulator.h
 * @brief PID仿真引擎 — 一阶被控对象建模 + 抗积分饱和PID闭环仿真
 *
 * 功能:
 *   - 一阶惯性系统 (可选纯滞后) 作为被控对象
 *   - 位置式PID控制，支持抗积分饱和与微分滤波
 *   - 可配置噪声注入、仿真时长、采样周期
 *   - 自动计算上升时间 / 调节时间 / 超调量 / 稳态误差 / IAE
 *   - 统计: quint64 计数器追踪仿真次数与计算步数
 *
 * @author EmbedDebug Team
 * @date 2026-06-06
 */

#ifndef PIDSIMULATOR_H
#define PIDSIMULATOR_H

#include <QObject>
#include <QVector>
#include "utils/pid/PidTunerTypes.h"

/**
 * @class PidSimulator
 * @brief PID闭环仿真器，支持一阶被控对象 + 纯滞后 + 噪声注入
 */
class PidSimulator : public QObject {
    Q_OBJECT
public:
    /** 被控对象 (Plant) 参数 */
    struct PlantParams {
        double gain         = 1.0;  ///< 静态增益 K
        double timeConstant = 1.0;  ///< 时间常数 T (秒)
        double delay        = 0.0;  ///< 纯滞后 τ (秒)
        double noiseStddev  = 0.0;  ///< 测量噪声标准差
    };

    /** 仿真配置 */
    struct SimConfig {
        double duration     = 10.0;  ///< 仿真总时长 (秒)
        double sampleTime   = 0.01;  ///< 采样周期 (秒)
        double windupLimit  = 100.0; ///< 积分饱和限幅
        double derivFilterAlpha = 0.1; ///< 微分滤波系数 α
    };

    /** 运行统计 */
    struct Stats {
        quint64 simulationsRun  = 0; ///< 已完成的仿真次数
        quint64 stepsComputed   = 0; ///< 累计计算步数
    };

    explicit PidSimulator(QObject* parent = nullptr);

    /** 设置被控对象参数 */
    void setPlantParams(const PlantParams& params);

    /** 设置仿真配置 */
    void setSimConfig(const SimConfig& config);

    /**
     * @brief 执行一步完整仿真
     * @param pid   PID参数集合
     * @param type  激励信号类型
     * @return 完整的PID响应结果 (时序 + 性能指标)
     */
    PidResponse simulate(const PidParams& pid, PidResponseType type = PidResponseType::Step);

    /** 获取运行统计 */
    const Stats& stats() const { return m_stats; }

    /** 重置统计计数器 */
    void resetStatistics();

signals:
    /** 仿真完成时发射，携带输出和控制器输出序列 */
    void simulationComplete(const QVector<double>& output,
                            const QVector<double>& control);

private:
    /** 生成激励信号设定值序列 */
    QVector<double> generateSetpoint(PidResponseType type, int steps) const;

    /** 计算性能指标并填充到 response */
    void computeMetrics(PidResponse& resp, double setpoint) const;

    /** 一阶惯性环节 (含可选纯滞后) 一步更新 */
    double updatePlant(double controlOutput);

    PlantParams m_plant;
    SimConfig   m_config;
    Stats       m_stats;

    /* 运行时状态 — 每次 simulate() 前重置 */
    double m_plantState;
    double m_integral;
    double m_prevError;
    double m_filteredDeriv;
    int    m_delayIndex;
    QVector<double> m_delayBuffer;
};

#endif // PIDSIMULATOR_H
