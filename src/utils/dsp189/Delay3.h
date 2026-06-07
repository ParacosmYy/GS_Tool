/**
 * @file Delay3.h
 * @brief 音频延迟线(节拍同步+乒乓立体声+反馈滤波+抽头调制) — Audio Delay Line with Tempo-Sync, Ping-Pong Stereo, Feedback Filter and Tap Modulation
 *
 * 功能: 实现音频延迟效果器，支持节拍同步(BPM)、乒乓立体声、
 *       反馈低通/高通滤波、多抽头调制和线性插值延迟时间。
 *
 * 协作: FIRFilter3(FIR滤波) / IIRFilter4(IIR滤波) / Delay2(基本延迟)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 音频延迟线(节拍同步+乒乓立体声+反馈滤波)
 */
class Delay3 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSamples = 0;
        double feedbackGain = 0.0;
        double wetMix = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief 反馈滤波器类型 */
    enum FilterType { None, LowPass, HighPass };

    explicit Delay3(QObject *parent = nullptr);
    ~Delay3() override;

    void setDelayTimeMs(double ms);
    void setBpm(double bpm);
    void setTempoSync(bool sync);
    void setFeedback(double gain);
    void setWetMix(double mix);
    void setPingPong(bool enabled);
    void setFilterType(FilterType type);
    void setFilterCutoff(double cutoff);
    void setTapModulation(double depth, double rate);
    void setSampleRate(double sr);

    /** @brief 处理单声道样本 */
    double processSample(double input);

    /** @brief 处理立体声样本对(返回左右声道) */
    QPair<double, double> processStereo(double inL, double inR);

    /** @brief 批量处理单声道 */
    QVector<double> processBlock(const QVector<double>& input);

    void flush();
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int numSamples, double timeMs);

private:
    double m_sampleRate = 44100.0;
    double m_delayMs = 250.0;
    double m_bpm = 120.0;
    bool m_tempoSync = false;
    double m_feedback = 0.4;
    double m_wetMix = 0.3;
    bool m_pingPong = false;
    FilterType m_filterType = None;
    double m_filterCutoff = 4000.0;
    double m_modDepth = 0.0;
    double m_modRate = 0.5;
    double m_modPhase = 0.0;

    QVector<double> m_bufferL;
    QVector<double> m_bufferR;
    int m_writePos = 0;
    int m_delaySamples = 0;
    double m_filterStateL = 0.0;
    double m_filterStateR = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute delay in samples from current settings */
    void updateDelaySamples();

    /** @brief Read from circular buffer with linear interpolation */
    double readInterp(const QVector<double>& buf, double delaySamples) const;

    /** @brief Apply feedback filter to sample */
    double applyFilter(double input, double& state) const;
};
