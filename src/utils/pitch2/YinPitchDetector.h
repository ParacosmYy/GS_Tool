/**
 * @file YinPitchDetector.h
 * @brief YIN基频检测 — 自相关基频估计算法
 *
 * 功能: YIN算法实现基频(F0)检测，比简单自相关更精确，
 *       支持置信度输出，统计检测次数/耗时。
 */
#ifndef YINPITCHDETECTOR_H
#define YINPITCHDETECTOR_H

#include <QObject>
#include <QVector>

class YinPitchDetector : public QObject {
    Q_OBJECT
public:
    /** 基频检测结果 */
    struct PitchResult {
        double frequency;       ///< 检测到的基频(Hz)，0表示无声
        double confidence;      ///< 置信度[0,1]
        bool   isVoiced;        ///< 是否有声
    };

    struct Stats {
        quint64 totalDetections = 0;
        quint64 totalVoiced = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit YinPitchDetector(int bufferSize = 2048,
                                QObject* parent = nullptr);

    /** @brief 检测基频 @param signal 输入信号 @param sampleRate 采样率 @return 检测结果 */
    PitchResult detect(const QVector<double>& signal, double sampleRate);

    /** @brief 设置频率范围 @param minFreq 最低频率 @param maxFreq 最高频率 */
    void setFrequencyRange(double minFreq = 50.0, double maxFreq = 2000.0);

    /** @brief 设置置信度阈值 @param threshold 阈值[0,1] */
    void setConfidenceThreshold(double threshold = 0.3);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void pitchDetected(double frequency, double confidence);

private:
    int m_bufferSize;
    double m_minFreq;
    double m_maxFreq;
    double m_threshold;
    Stats m_stats;
    double m_timeSum;
};

#endif // YINPITCHDETECTOR_H
