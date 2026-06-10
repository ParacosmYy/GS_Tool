/**
 * @file Delay9.h
 * @brief 延迟效果(多拍节奏同步模式与乒乓立体声路由的节奏空间延迟效果) — Delay with Multi-tap Tempo-synced Pattern and Ping-pong Stereo Routing for Rhythmic Spatial Delay Effects
 *
 * 功能: 实现延迟效果(delay effect)，采用多拍节奏同步模式(multi-tap tempo-synced pattern)
 *       与乒乓立体声路由(ping-pong stereo routing)实现节奏空间延迟效果(rhythmic spatial delay effects)。
 *
 * 协作: Chorus8(合唱) / Reverb7(混响) / FilterBank6(滤波器组)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 延迟效果(多拍节奏同步模式与乒乓立体声路由)
 */
class Delay9 : public QObject {
    Q_OBJECT

public:
    /** @brief Stereo sample pair */
    struct StereoSample {
        double left = 0.0;
        double right = 0.0;
    };

    /** @brief Delay tap configuration */
    struct TapConfig {
        double beatOffset = 0.0;
        double gain = 0.7;
        double pan = 0.0;  // -1.0 (left) to +1.0 (right)
        bool pingPong = false;
    };

    /** @brief Processing result */
    struct ProcessResult {
        int framesProcessed = 0;
        double peakLevel = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int sampleRate = 44100;
        double bpm = 120.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Delay9(QObject *parent = nullptr);
    ~Delay9() override;

    /** @brief Set sample rate in Hz */
    void setSampleRate(int rate);

    /** @brief Set tempo in BPM for tempo-synced patterns */
    void setTempo(double bpm);

    /** @brief Set feedback amount (0.0 to 0.95) */
    void setFeedback(double fb);

    /** @brief Set mix ratio dry/wet (0.0 = dry, 1.0 = wet) */
    void setMix(double mix);

    /** @brief Configure delay taps */
    void setTaps(const QVector<TapConfig>& taps);

    /** @brief Process mono input to stereo output with ping-pong routing */
    QVector<StereoSample> process(const QVector<double>& input);

    /** @brief Process stereo input with ping-pong routing */
    QVector<StereoSample> processStereo(const QVector<StereoSample>& input);

    /** @brief Reset delay buffers */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processDone(int frames, double peak, double timeMs);

private:
    int m_sampleRate = 44100;
    double m_bpm = 120.0;
    double m_feedback = 0.5;
    double m_mix = 0.4;
    Stats m_stats;
    double m_timeSum = 0.0;

    QVector<TapConfig> m_taps;
    QVector<QVector<double>> m_delayBuffers;  // Per-tap circular buffers
    QVector<int> m_writePos;                   // Write positions per tap

    /** @brief Calculate delay samples from beat offset */
    int beatToSamples(double beatOffset) const;

    /** @brief Pan gain computation */
    StereoSample panGain(double pan, double sample) const;

    /** @brief Read from circular buffer at offset position */
    double readBuffer(int tapIdx, int offset) const;

    /** @brief Write to circular buffer */
    void writeBuffer(int tapIdx, double value);

    /** @brief Initialize buffers from current tap configuration */
    void initBuffers();
};
