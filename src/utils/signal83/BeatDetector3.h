#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 节拍检测器
 *
 * 从音频信号中检测节拍位置和BPM，支持在线和离线模式。
 */
class BeatDetector3 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalBeatsDetected = 0;
        int totalBuffersAnalyzed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BeatDetector3(QObject* parent = nullptr);

    /** @brief 检测节拍位置(采样点索引) */
    QVector<int> detectBeats(const QVector<double>& samples, double sampleRate);

    /** @brief 估计BPM值 */
    double estimateBPM(const QVector<double>& samples, double sampleRate);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void beatDetected(int sampleIndex, double confidence);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_lastBPM = 0.0;
};
