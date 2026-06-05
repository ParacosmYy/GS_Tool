#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Expander6 - 动态范围扩展器第6代实现
 *
 * 提供向下扩展功能，用于降低噪声门限以下信号电平，
 * 支持RMS/峰值检测、可调扩展比及启动/释放时间控制。
 */
class Expander6 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalProcessFrames = 0; double avgProcessingTimeMs = 0.0; };
    explicit Expander6(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 处理音频帧进行动态扩展
     * @param inputFrame 输入音频采样帧
     * @return 扩展处理后的音频帧
     */
    QVector<double> processFrame(const QVector<double>& inputFrame);

    /**
     * @brief 设置扩展器参数
     * @param thresholdDb 阈值 (dB)
     * @param ratio 扩展比
     * @param attackMs 启动时间 (ms)
     * @param releaseMs 释放时间 (ms)
     */
    void setParameters(double thresholdDb, double ratio, double attackMs, double releaseMs);

    /**
     * @brief 获取当前增益缩减量
     * @return 增益缩减值 (dB)
     */
    double getGainReduction() const;

    /**
     * @brief 启用/禁用侧链检测模式
     * @param enabled 是否启用侧链
     * @param sidechainSignal 侧链信号
     */
    void setSidechain(bool enabled, const QVector<double>& sidechainSignal = {});

signals:
    void expansionCompleted(int frameCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
