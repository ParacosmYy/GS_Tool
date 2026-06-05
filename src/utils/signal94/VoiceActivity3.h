#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 语音活动检测(VAD)工具类
 *
 * 基于能量阈值和帧分析检测语音活动状态，
 * 用于区分语音段和静音/噪声段。
 */
class VoiceActivity3 : public QObject {
    Q_OBJECT
public:
    /// 检测统计信息
    struct Stats {
        int totalDetections = 0;    ///< 总检测次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit VoiceActivity3(QObject* parent = nullptr);

    /** @brief 设置能量检测阈值(dB) */
    void setThreshold(double thresholdDb);

    /** @brief 设置分析帧大小(采样点数) */
    void setFrameSize(int size);

    /** @brief 对输入信号帧检测语音活动，返回是否为语音 */
    bool detect(const QVector<double>& frame);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 检测完成信号，返回是否为语音活动 */
    void detected(bool isVoice);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_threshold = -30.0;
    int m_frameSize = 512;
};
