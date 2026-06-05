#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief MultibandComp6 - 多段动态压缩器第6代实现
 *
 * 将信号分为多个频段分别进行独立压缩处理，
 * 支持可配置分频点、每段独立参数及交叉馈送补偿。
 */
class MultibandComp6 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalProcessFrames = 0; double avgProcessingTimeMs = 0.0; };
    explicit MultibandComp6(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 处理音频帧进行多段压缩
     * @param inputFrame 输入音频采样帧
     * @return 压缩后的音频帧
     */
    QVector<double> processFrame(const QVector<double>& inputFrame);

    /**
     * @brief 设置分频点
     * @param crossoverFreqs 分频点频率列表 (Hz)
     * @param sampleRate 采样率 (Hz)
     */
    void setCrossovers(const QVector<double>& crossoverFreqs, double sampleRate);

    /**
     * @brief 设置指定频段的压缩参数
     * @param bandIndex 频段索引
     * @param thresholdDb 阈值 (dB)
     * @param ratio 压缩比
     * @param attackMs 攻击时间 (ms)
     * @param releaseMs 释放时间 (ms)
     */
    void setBandParameters(int bandIndex, double thresholdDb, double ratio,
                           double attackMs, double releaseMs);

    /**
     * @brief 获取各频段当前增益缩减量
     * @return 各频段的增益缩减值 (dB)
     */
    QVector<double> getBandGainReductions() const;

signals:
    void compressionCompleted(int frameCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
