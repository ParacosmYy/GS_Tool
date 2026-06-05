#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Gate6 - 噪声门第6代实现
 *
 * 提供动态噪声门处理功能，支持阈值/攻击/释放参数调节、
 * 范围控制及侧链触发模式。
 */
class Gate6 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalProcessFrames = 0; double avgProcessingTimeMs = 0.0; };
    explicit Gate6(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 处理音频帧进行噪声门控制
     * @param inputFrame 输入音频采样帧
     * @return 门控后的音频帧
     */
    QVector<double> processFrame(const QVector<double>& inputFrame);

    /**
     * @brief 设置噪声门参数
     * @param thresholdDb 门限阈值 (dB)
     * @param attackMs 攻击时间 (ms)
     * @param releaseMs 释放时间 (ms)
     * @param rangeDb 衰减范围 (dB)
     */
    void setParameters(double thresholdDb, double attackMs, double releaseMs, double rangeDb);

    /**
     * @brief 获取当前门状态（开/关）
     * @return true表示门开启（信号通过），false表示门关闭
     */
    bool isGateOpen() const;

    /**
     * @brief 设置保持时间（防止快速开关抖动）
     * @param holdMs 保持时间 (ms)
     */
    void setHoldTime(double holdMs);

signals:
    void gateStateChanged(bool isOpen);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
