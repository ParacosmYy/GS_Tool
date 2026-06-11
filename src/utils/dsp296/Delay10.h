/**
 * @file Delay10.h
 * @brief 延迟效果(节拍同步调制与乒乓立体声路由实现节奏回声与空间延迟效果) — Delay with Tempo-synced Modulation and Ping-pong Stereo Routing for Rhythmic Echo and Spatial Delay Effects
 *
 * 功能: 实现延迟效果(delay)，采用节拍同步调制(tempo-synced modulation)
 *       与乒乓立体声路由(ping-pong stereo routing)实现节奏回声与空间延迟效果(rhythmic echo and spatial delay effects)。
 *
 * 协作: Chorus8(合唱效果) / Reverb9(混响) / Compressor7(动态压缩)
 */
#pragma once

#include <QObject>
#include <QVector>

class Delay10 : public QObject {
    Q_OBJECT

public:
    /** @brief Stereo sample frame */
    struct StereoFrame {
        double left = 0.0;
        double right = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalProcess = 0;
        int bufferSize = 0;
        double tempoBpm = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Delay10(QObject *parent = nullptr);
    ~Delay10() override;

    void setTempo(double bpm);
    void setDelayTime(double seconds);
    void setFeedback(double fb);
    void setMix(double wet);
    void setPingPong(bool enabled);

    /** @brief Process interleaved stereo samples */
    QVector<double> process(const QVector<double>& input);

    /** @brief Process raw stereo frames */
    QVector<StereoFrame> processFrames(const QVector<StereoFrame>& input);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processDone(int frames, double timeMs);

private:
    double m_tempoBpm = 120.0;
    double m_delayTime = 0.375;
    double m_feedback = 0.4;
    double m_mix = 0.3;
    bool m_pingPong = true;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Circular delay buffers for L and R channels */
    QVector<double> m_bufferL;
    QVector<double> m_bufferR;

    /** @brief Write positions for each buffer */
    int m_writePos = 0;
    int m_bufferLen = 0;

    /** @brief LFO phase for tempo-synced modulation */
    double m_lfoPhase = 0.0;

    /** @brief Initialize delay buffers */
    void initBuffers();

    /** @brief Read from circular buffer with fractional delay */
    double readBuffer(const QVector<double>& buf, double delaySamples) const;

    /** @brief Advance LFO and return modulation offset */
    double lfoModulation();

    /** @brief Apply low-pass filter to feedback path */
    double lowpass(double input, double& state) const;
};
