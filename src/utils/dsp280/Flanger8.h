/**
 * @file Flanger8.h
 * @brief 镶边效果器(可变延迟深度调制与立体声交叉反馈的维度扫频效果) — Flanger with Variable Delay Depth Modulation and Stereo Cross-feedback for Dimensional Sweep Effects
 *
 * 功能: 实现镶边效果器(Flanger)，采用可变延迟深度调制(variable delay depth modulation)
 *       与立体声交叉反馈(stereo cross-feedback)实现维度扫频效果(dimensional sweep effects)。
 *
 * 协作: Chorus7(合唱) / Phaser6(移相) / Delay5(延迟)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 镶边效果器(可变延迟深度调制与立体声交叉反馈)
 */
class Flanger8 : public QObject {
    Q_OBJECT

public:
    /** @brief Flanger parameter set */
    struct Params {
        double rate = 0.5;          // LFO frequency (Hz)
        double depth = 0.5;         // Modulation depth (0..1)
        double feedback = 0.7;      // Feedback gain (0..0.95)
        double mix = 0.5;           // Wet/dry mix (0..1)
        double stereoPhase = 90.0;  // Stereo LFO phase offset (degrees)
        double crossFeed = 0.3;     // Stereo cross-feedback amount
        int baseDelay = 3;          // Base delay in samples
    };

    /** @brief Processing result for stereo output */
    struct StereoBuffer {
        QVector<double> left;
        QVector<double> right;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numSamples = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Flanger8(QObject *parent = nullptr);
    ~Flanger8() override;

    /** @brief Set flanger parameters */
    void setParams(const Params& params);

    /** @brief Get current parameters */
    const Params& params() const { return m_params; }

    /** @brief Set sample rate */
    void setSampleRate(double rate);

    /** @brief Process mono input to stereo flanged output */
    StereoBuffer process(const QVector<double>& input);

    /** @brief Process stereo input to stereo flanged output */
    StereoBuffer processStereo(const QVector<double>& inL, const QVector<double>& inR);

    /** @brief Reset delay lines and LFO phase */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingDone(int samples, double timeMs);

private:
    Params m_params;
    double m_sampleRate = 44100.0;
    Stats m_stats;
    double m_timeSum = 0.0;

    // Delay line buffers
    static constexpr int MAX_DELAY = 4096;
    QVector<double> m_delayL;
    QVector<double> m_delayR;
    int m_writePos = 0;

    // LFO state
    double m_phaseL = 0.0;
    double m_phaseR = 0.0;

    // Cross-feedback state
    double m_crossL = 0.0;
    double m_crossR = 0.0;

    /** @brief LFO output: sine wave at given phase */
    double lfo(double phase) const;

    /** @brief Fractional delay read with linear interpolation */
    double readDelay(const QVector<double>& buffer, double delaySamples) const;

    /** @brief Advance LFO phase */
    double advancePhase(double phase) const;
};
