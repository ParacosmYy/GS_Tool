/**
 * @file CostasLoop.h
 * @brief Costas环 — 载波频率/相位恢复
 */
#pragma once

#include <QObject>

/**
 * @class CostasLoop
 * @brief 二阶Costas环，用于从复数基带信号中恢复载波频率和相位
 *
 * 适用于BPSK/QPSK等调制信号的载波同步。
 * 内部使用比例-积分(PI)滤波器作为环路滤波器。
 */
class CostasLoop : public QObject {
    Q_OBJECT
public:
    /** @brief 统计信息结构体 */
    struct Stats {
        quint64 totalSamples = 0;           ///< 总处理采样数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
    };

    explicit CostasLoop(QObject* parent = nullptr);

    /**
     * @brief 处理一对I/Q采样点
     * @param iSample 同相分量
     * @param qSample 正交分量
     * @return 经相位旋转后的同相输出（解调后的基带）
     */
    double process(double iSample, double qSample);

    /**
     * @brief 设置环路带宽
     * @param bw 带宽系数 (0 < bw < 1)，值越大锁定越快但噪声越大
     */
    void setBandwidth(double bw);

    /** @brief 重置环路状态 */
    void reset();

    /** @brief 获取当前估计频率(归一化, 单位: 弧度/采样) */
    double frequency() const { return m_freq; }

    /** @brief 获取当前估计相位(弧度) */
    double phase() const { return m_phase; }

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 频率更新信号 @param freq 当前估计频率 */
    void frequencyUpdated(double freq);

private:
    Stats   m_stats;
    double  m_timeSum = 0.0;   ///< 累计处理时间

    double  m_phase   = 0.0;   ///< 当前相位估计(弧度)
    double  m_freq    = 0.0;   ///< 当前频率估计(弧度/采样)
    double  m_bw      = 0.01;  ///< 环路带宽系数
    double  m_alpha   = 0.0;   ///< PI滤波器比例增益
    double  m_beta    = 0.0;   ///< PI滤波器积分增益

    /** @brief 根据带宽重新计算PI增益 */
    void updateGains();
};
