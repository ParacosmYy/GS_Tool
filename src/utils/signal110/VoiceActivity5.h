#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 语音活动检测器实现 (版本5)
 *
 * 基于能量、过零率和谱特征的语音/非语音分段，支持实时检测和自适应阈值。
 */
class VoiceActivity5 : public QObject {
    Q_OBJECT
public:
    /// 帧分类结果
    enum FrameClass { Silence = 0, Unvoiced = 1, Voiced = 2 };

    /// 统计信息结构
    struct Stats {
        int totalFrames = 0;            ///< 总分析帧数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
        double voiceRatio = 0.0;        ///< 语音帧比例
    };

    explicit VoiceActivity5(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 检测单帧语音活动
     * @param frame 音频帧采样数据
     * @return 帧分类结果
     */
    FrameClass detect(const QVector<double>& frame);

    /**
     * @brief 批量检测多帧
     * @param samples 连续采样数据
     * @param frameSize 帧大小
     * @param hopSize 帧步长
     * @return 各帧分类结果
     */
    QVector<FrameClass> detectMultiFrame(const QVector<double>& samples, int frameSize, int hopSize);

    /**
     * @brief 设置检测阈值
     * @param energyThresholdDb 能量阈值(dB)
     * @param zcrThreshold 过零率阈值
     */
    void setThresholds(double energyThresholdDb, double zcrThreshold);

    /**
     * @brief 启用/禁用自适应阈值模式
     * @param enabled 是否启用
     * @param adaptationRate 自适应学习率
     */
    void setAdaptive(bool enabled, double adaptationRate = 0.01);

signals:
    /// 检测完成信号
    void detectionCompleted(FrameClass result);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_energyThresholdDb = -40.0;
    double m_zcrThreshold = 0.15;
    bool m_adaptive = false;
    double m_adaptationRate = 0.01;
    double m_noiseEstimate = 0.0;
};
