#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief VoiceActivity6 - 语音活动检测第6代实现
 *
 * 检测音频信号中的语音段与静音/噪声段，
 * 支持能量阈值法、零交叉率法及基于特征的混合判决。
 */
class VoiceActivity6 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDetectOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit VoiceActivity6(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 检测单帧语音活动
     * @param frame 音频帧数据
     * @return true表示存在语音活动
     */
    bool detect(const QVector<double>& frame);

    /**
     * @brief 批量检测多帧语音活动
     * @param frames 音频帧序列
     * @return 各帧的语音/非语音标记
     */
    QVector<bool> detectBatch(const QVector<QVector<double>>& frames);

    /**
     * @brief 设置检测灵敏度
     * @param sensitivity 灵敏度级别 (low/medium/high)
     */
    void setSensitivity(const QString& sensitivity);

    /**
     * @brief 计算帧能量特征
     * @param frame 音频帧
     * @return 帧能量值 (dB)
     */
    double computeFrameEnergy(const QVector<double>& frame) const;

signals:
    void detectionCompleted(int voiceFrameCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
