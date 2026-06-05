#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief PitchTrack8 - 音高追踪第8代实现
 *
 * 实时追踪音频信号的基频（F0），支持YIN、AMDF、
   自相关及概率模型等多种音高估计算法。
 */
class PitchTrack8 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalTrackOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit PitchTrack8(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 使用YIN算法检测音高
     * @param frame 音频帧数据
     * @param sampleRate 采样率 (Hz)
     * @return 检测到的基频 (Hz)，无音调返回0
     */
    double detectYIN(const QVector<double>& frame, double sampleRate);

    /**
     * @brief 使用AMDF算法检测音高
     * @param frame 音频帧数据
     * @param sampleRate 采样率 (Hz)
     * @return 检测到的基频 (Hz)，无音调返回0
     */
    double detectAMDF(const QVector<double>& frame, double sampleRate);

    /**
     * @brief 批量音高追踪，返回逐帧基频序列
     * @param frames 分帧后的音频数据
     * @param sampleRate 采样率 (Hz)
     * @return 各帧的基频 (Hz)
     */
    QVector<double> trackSequence(const QVector<QVector<double>>& frames,
                                  double sampleRate);

    /**
     * @brief 设置音高检测范围
     * @param minFreqHz 最低频率 (Hz)
     * @param maxFreqHz 最高频率 (Hz)
     */
    void setFrequencyRange(double minFreqHz, double maxFreqHz);

signals:
    void trackingCompleted(int frameCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
