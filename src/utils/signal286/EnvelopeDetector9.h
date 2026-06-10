/**
 * @file EnvelopeDetector9.h
 * @brief 包络检测器(峰值与RMS双模式及可配置动力学的综合幅度分析) — Envelope Detector with Peak and RMS Dual-mode and Configurable Ballistics for Comprehensive Amplitude Analysis
 *
 * 功能: 实现包络检测器(envelope detector)，采用峰值与RMS双模式(peak and RMS dual-mode)
 *       与可配置动力学(configurable ballistics)实现综合幅度分析(comprehensive amplitude analysis)。
 *
 * 协作: Limiter12(限幅器) / Compressor10(压缩器) / FIR8(FIR滤波器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 包络检测器(峰值与RMS双模式及可配置动力学)
 */
class EnvelopeDetector9 : public QObject {
    Q_OBJECT

public:
    /** @brief Detection mode */
    enum Mode {
        Peak = 0,       // Peak envelope follower
        RMS = 1,        // RMS envelope with windowing
        DualMode = 2    // Simultaneous peak and RMS
    };

    /** @brief Detector configuration */
    struct DetectorConfig {
        Mode mode = DualMode;
        double attack = 1.0;        // Attack time (ms)
        double release = 100.0;     // Release time (ms)
        double sampleRate = 44100.0;
        int rmsWindow = 256;
        bool logOutput = false;     // Output in dB
    };

    /** @brief Detection result */
    struct EnvelopeResult {
        QVector<double> envelopePeak;
        QVector<double> envelopeRMS;
        double peakAmplitude = 0.0;
        double rmsLevel = 0.0;
        double crestFactor = 0.0;   // Peak/RMS ratio
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int frameSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit EnvelopeDetector9(QObject *parent = nullptr);
    ~EnvelopeDetector9() override;

    void setConfig(const DetectorConfig& cfg);

    /** @brief Detect envelope of input signal */
    EnvelopeResult detect(const QVector<double>& input);

    /** @brief Process single sample (streaming mode) */
    double processSample(double sample);

    /** @brief Compute RMS of a window */
    double computeRMS(const QVector<double>& window) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void detectDone(int n, double peak, double rms, double timeMs);

private:
    DetectorConfig m_config;
    Stats m_stats;
    double m_timeSum = 0.0;

    double m_envelopePeak = 0.0;
    double m_envelopeRMS = 0.0;
    QVector<double> m_rmsBuffer;
    int m_rmsPos = 0;
    double m_rmsSum = 0.0;

    double m_attackCoeff = 0.0;
    double m_releaseCoeff = 0.0;

    /** @brief Update ballistics coefficients */
    void updateCoeffs();

    /** @brief Apply ballistics to a sample */
    double applyBallistics(double input, double state) const;
};
