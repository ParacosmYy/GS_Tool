/**
 * @file SignalSynchronizer3.h
 * @brief 信号同步器(导频音相关+载波频偏估计与校正) — Signal Synchronizer via Pilot-Tone Correlation with Carrier Frequency Offset Estimation and Correction
 *
 * 功能: 实现基于导频音相关的信号同步，支持载波频偏估计、
 *       频偏校正和定时同步。
 *
 * 协作: CostasLoop4(Costas环) / PLL5(锁相环) / FrameSync6(帧同步)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 信号同步器(导频音相关+载波频偏估计与校正)
 */
class SignalSynchronizer3 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalSyncs = 0;
        int signalLength = 0;
        double estimatedCFO = 0.0;
        double correlationPeak = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SignalSynchronizer3(QObject *parent = nullptr);
    ~SignalSynchronizer3() override;

    void setPilotTone(double freqHz, double sampleRate);
    void setSearchRange(double maxFreqOffsetHz);
    void setCorrelationThreshold(double threshold);

    /** @brief Synchronize signal: estimate and correct CFO */
    QVector<double> synchronize(const QVector<double>& signal);

    /** @brief Generate pilot tone reference */
    QVector<double> generatePilot(int length) const;

    /** @brief Cross-correlate signal with pilot */
    QVector<double> crossCorrelate(const QVector<double>& signal,
                                    const QVector<double>& pilot) const;

    /** @brief Estimate carrier frequency offset */
    double estimateCFO(const QVector<double>& signal) const;

    /** @brief Correct carrier frequency offset */
    QVector<double> correctCFO(const QVector<double>& signal, double cfo) const;

    /** @brief Find timing offset via correlation peak */
    int findTimingOffset(const QVector<double>& correlation) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void syncCompleted(double cfo, int timingOffset, double timeMs);

private:
    double m_pilotFreqHz = 1000.0;
    double m_sampleRate = 48000.0;
    double m_maxFreqOffsetHz = 500.0;
    double m_corrThreshold = 0.5;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute complex-valued correlation at specific frequency */
    double correlateAtFreq(const QVector<double>& signal, double freq) const;

    /** @brief Parabolic interpolation for sub-sample peak */
    double parabolicInterp(double y0, double y1, double y2) const;
};
