#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 基音追踪器
 *
 * 基于自相关/YIN算法的基频(F0)连续追踪。
 */
class PitchTrack4 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalFramesTracked = 0;
        int totalVoicedFrames = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit PitchTrack4(QObject* parent = nullptr);

    /** @brief 追踪单帧基频，返回Hz(0表示无声) */
    double trackFrame(const QVector<double>& frame, double sampleRate);

    /** @brief 批量追踪基频序列 */
    QVector<double> trackSequence(const QVector<double>& samples, double sampleRate,
                                   int frameSize, int hopSize);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void pitchDetected(double freqHz, double confidence);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_lastPitch = 0.0;
};
