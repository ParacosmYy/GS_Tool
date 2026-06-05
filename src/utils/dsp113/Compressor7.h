#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Compressor7 - 动态范围压缩器第7代实现
 *
 * 提供多段动态压缩功能，支持RMS/峰值检测模式、
 * 可调攻击/释放时间、软/硬拐点及增益补偿。
 */
class Compressor7 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalProcessFrames = 0; double avgProcessingTimeMs = 0.0; };
    explicit Compressor7(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 处理音频帧进行动态压缩
     * @param inputFrame 输入音频采样帧
     * @return 压缩后的音频帧
     */
    QVector<double> processFrame(const QVector<double>& inputFrame);

    /**
     * @brief 设置压缩器参数
     * @param thresholdDb 阈值 (dB)
     * @param ratio 压缩比
     * @param attackMs 攻击时间 (ms)
     * @param releaseMs 释放时间 (ms)
     */
    void setParameters(double thresholdDb, double ratio, double attackMs, double releaseMs);

    /**
     * @brief 获取当前增益缩减量
     * @return 增益缩减值 (dB)
     */
    double getGainReduction() const;

    /**
     * @brief 设置软/硬拐点模式
     * @param softKnee 是否启用软拐点
     * @param kneeWidthDb 拐点宽度 (dB)
     */
    void setKneeMode(bool softKnee, double kneeWidthDb = 6.0);

signals:
    void compressionCompleted(int frameCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
