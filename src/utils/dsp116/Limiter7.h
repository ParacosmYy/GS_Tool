#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Limiter7 - 限幅器第7代实现
 *
 * 提供砖墙限幅功能，确保输出信号不超过设定阈值，
 * 支持 lookahead 缓冲、攻击/释放时间及增益指示。
 */
class Limiter7 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalProcessFrames = 0; double avgProcessingTimeMs = 0.0; };
    explicit Limiter7(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 处理音频帧进行限幅
     * @param inputFrame 输入音频采样帧
     * @return 限幅后的音频帧
     */
    QVector<double> processFrame(const QVector<double>& inputFrame);

    /**
     * @brief 设置限幅器参数
     * @param ceilingDb 输出上限 (dB)
     * @param thresholdDb 限幅阈值 (dB)
     * @param releaseMs 释放时间 (ms)
     */
    void setParameters(double ceilingDb, double thresholdDb, double releaseMs);

    /**
     * @brief 设置 lookahead 时间（延迟补偿）
     * @param lookaheadMs lookahead 时间 (ms)
     */
    void setLookahead(double lookaheadMs);

    /**
     * @brief 获取当前增益缩减量
     * @return 增益缩减值 (dB)
     */
    double getGainReduction() const;

signals:
    void limitingCompleted(int frameCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
