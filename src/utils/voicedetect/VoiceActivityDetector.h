/**
 * @file VoiceActivityDetector.h
 * @brief 语音活动检测器 — 能量/过零率双门限
 *
 * 功能: 基于短时能量和过零率的双门限VAD，
 *       统计检测次数/有声帧数/静音帧数/耗时。
 */
#ifndef VOICEACTIVITYDETECTOR_H
#define VOICEACTIVITYDETECTOR_H

#include <QObject>
#include <QVector>

class VoiceActivityDetector : public QObject {
    Q_OBJECT
public:
    /** 检测统计 */
    struct Stats {
        quint64 totalDetections = 0;
        quint64 totalVoicedFrames = 0;
        quint64 totalSilentFrames = 0;
        double  averageProcessingTimeMs = 0.0;
    };

    /** 检测结果 */
    struct VadResult {
        QVector<int> voicedFrames;     ///< 有声帧索引
        QVector<int> silentFrames;     ///< 静音帧索引
        QVector<double> frameEnergy;   ///< 每帧能量
        QVector<double> frameZcr;      ///< 每帧过零率
        int totalFrames = 0;           ///< 总帧数
    };

    explicit VoiceActivityDetector(QObject* parent = nullptr);

    /** @brief 设置能量阈值 @param threshold 阈值(0~1归一化) */
    void setEnergyThreshold(double threshold);

    /** @brief 设置过零率阈值 @param threshold 阈值(0~1归一化) */
    void setZcrThreshold(double threshold);

    /** @brief 设置帧参数 @param frameSize 帧长 @param hopSize 步进 */
    void setFrameParams(int frameSize, int hopSize);

    /** @brief 执行VAD检测 @param data 音频数据(归一化) @return 检测结果 */
    VadResult detect(const QVector<double>& data);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void detectionCompleted(int voicedFrames, int totalFrames);

private:
    double computeFrameEnergy(const QVector<double>& data,
                              int start, int length) const;
    double computeFrameZcr(const QVector<double>& data,
                           int start, int length) const;

    double m_energyThreshold;
    double m_zcrThreshold;
    int m_frameSize;
    int m_hopSize;
    Stats m_stats;
    double m_timeSum;
};

#endif // VOICEACTIVITYDETECTOR_H
