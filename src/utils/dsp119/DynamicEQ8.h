#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief DynamicEQ8 - 动态均衡器第8代实现
 *
 * 结合参数均衡与动态处理的智能均衡器，
 * 根据信号电平自动调节指定频段的增益。
 */
class DynamicEQ8 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalProcessFrames = 0; double avgProcessingTimeMs = 0.0; };
    explicit DynamicEQ8(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 处理音频帧进行动态均衡
     * @param inputFrame 输入音频采样帧
     * @return 均衡后的音频帧
     */
    QVector<double> processFrame(const QVector<double>& inputFrame);

    /**
     * @brief 添加一个动态EQ频段
     * @param frequencyHz 中心频率 (Hz)
     * @param gainDb 静态增益 (dB)
     * @param Q 品质因数
     * @param thresholdDb 动态阈值 (dB)
     * @param rangeDb 动态范围 (dB)
     */
    void addBand(double frequencyHz, double gainDb, double Q,
                 double thresholdDb, double rangeDb);

    /**
     * @brief 设置侧链信号源
     * @param sidechainFrame 侧链音频帧
     */
    void setSidechain(const QVector<double>& sidechainFrame);

    /**
     * @brief 移除指定频段
     * @param bandIndex 频段索引
     * @return 是否移除成功
     */
    bool removeBand(int bandIndex);

signals:
    void equalizationCompleted(int frameCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
