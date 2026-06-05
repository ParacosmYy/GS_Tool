#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief PitchDetector3 - 基音频率检测器
 *
 * 结合自相关和AMDF方法的基音检测，支持
 * 实时逐帧分析，输出频率(Hz)和置信度。
 */
class PitchDetector3 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalFramesAnalyzed = 0;
        int totalPitchChanges = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit PitchDetector3(QObject* parent = nullptr);

    /** @brief 设置采样率(Hz) */
    void setSampleRate(double sampleRate);

    /** @brief 检测帧的基音频率(Hz)，无基音返回0 */
    double detectPitch(const QVector<double>& frame);

    /** @brief 检测基音并返回频率+置信度对 */
    QPair<double, double> detectPitchWithConfidence(const QVector<double>& frame);

    /** @brief 设置频率搜索范围(Hz) */
    void setFrequencyRange(double minHz, double maxHz);

    /** @brief 获取上一帧的基音周期(样本数) */
    int lastPeriod() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void pitchDetected(double frequencyHz, double confidence);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_sampleRate = 44100.0;
    int m_lastPeriod = 0;
    double m_minHz = 50.0;
    double m_maxHz = 800.0;
};
