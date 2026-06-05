/**
 * @file PhaseLockedLoop.h
 * @brief 数字锁相环(Phase-Locked Loop) — 相位检测/环路滤波/VCO
 *
 * 功能: 实现基本数字PLL，包含相位检测器、一阶环路滤波器
 *       和压控振荡器(VCO)。支持频率/带宽配置与锁定状态检测。
 *       统计处理采样数/锁定次数/平均耗时。
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @class PhaseLockedLoop
 * @brief 数字锁相环 — 支持频率跟踪与锁定检测
 *
 * 通过相位检测器计算输入信号与本振相位差，
 * 经环路滤波器平滑后驱动VCO跟踪输入频率。
 * 当相位误差低于阈值时判定为锁定状态。
 */
class PhaseLockedLoop : public QObject
{
    Q_OBJECT

public:
    /** @brief 运行统计信息 */
    struct Stats {
        quint64 totalSamples = 0;      ///< 总处理采样点数
        quint64 totalLocks = 0;        ///< 累计锁定次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param parent QObject父对象
     */
    explicit PhaseLockedLoop(QObject* parent = nullptr);

    /**
     * @brief 处理单个输入采样点
     * @param inputSample 输入信号采样值
     * @return VCO当前输出相位(弧度)
     */
    double process(double inputSample);

    /**
     * @brief 设置目标频率
     * @param freq 目标频率(Hz)，必须为正值
     */
    void setFrequency(double freq);

    /**
     * @brief 设置环路带宽
     * @param bw 环路带宽系数(0.0~1.0)，越大响应越快
     */
    void setBandwidth(double bw);

    /** @brief 重置PLL状态(相位/频率/滤波器) */
    void reset();

    /** @brief 获取当前是否处于锁定状态 */
    bool isLocked() const { return m_locked; }

    /** @brief 获取当前VCO频率 */
    double currentFrequency() const { return m_vcoFreq; }

    /** @brief 获取当前相位误差 */
    double phaseError() const { return m_phaseError; }

    /** @brief 获取统计信息 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 锁定状态变化信号 @param locked 当前是否锁定 */
    void lockChanged(bool locked);

private:
    /** @brief 相位检测器: 计算输入与VCO输出的相位差 */
    double phaseDetector(double input, double vcoOutput) const;

    /** @brief 环路滤波器: 对相位误差进行低通滤波 */
    double loopFilter(double error);

    /** @brief VCO: 根据控制电压生成输出相位 */
    double vco(double controlVoltage);

    mutable Stats m_stats;       ///< 统计信息
    double m_timeSum = 0.0;      ///< 累计耗时

    double m_centerFreq = 1000.0; ///< 中心频率(Hz)
    double m_bandwidth = 0.1;     ///< 环路带宽系数
    double m_vcoFreq = 1000.0;    ///< VCO当前频率(Hz)
    double m_vcoPhase = 0.0;      ///< VCO当前相位(弧度)
    double m_phaseError = 0.0;    ///< 当前相位误差
    double m_filteredError = 0.0; ///< 滤波后误差
    bool   m_locked = false;      ///< 是否锁定
    int    m_lockCounter = 0;     ///< 锁定判定计数器

    static constexpr double kSampleRate = 48000.0; ///< 默认采样率
    static constexpr int    kLockThreshold = 50;   ///< 锁定判定阈值(连续采样数)
    static constexpr double kLockPhaseThreshold = 0.15; ///< 锁定相位误差阈值(弧度)
};
