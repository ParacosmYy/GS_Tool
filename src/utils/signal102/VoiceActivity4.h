#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 语音活动检测器
 *
 * 基于能量和过零率特征检测音频中的语音段，
 * 用于语音通信中的静音抑制和语音增强前处理。
 */
class VoiceActivity4 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats {
        int totalFrames = 0;         ///< 已处理帧数
        int voiceFrames = 0;         ///< 语音帧数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit VoiceActivity4(QObject* parent = nullptr);

    /** @brief 设置能量阈值(dB) */
    void setThreshold(double thresholdDb);
    /** @brief 设置分析窗口大小 */
    void setWindowSize(int size);
    /** @brief 检测语音活动，返回各帧是否有语音 */
    QVector<bool> detect(const QVector<double>& samples);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 检测完成，返回是否检测到语音 */
    void detected(bool hasVoice);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_threshold = -30.0;
    int m_windowSize = 480;
};
