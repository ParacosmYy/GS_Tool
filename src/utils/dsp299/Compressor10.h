/**
 * @file Compressor10.h
 * @brief 动态压缩器(并行信号混合与自动增益补偿实现RMS/峰值检测模式切换的透明动态控制) — Compressor with Parallel Signal Blending and Auto-Gain Makeup with RMS/Peak Detection Mode Switching for Transparent Dynamic Control
 *
 * 功能: 实现动态压缩器(dynamic compressor)，采用并行信号混合(parallel signal blending)
 *       与自动增益补偿(auto-gain makeup)结合RMS/峰值检测模式切换(RMS/peak detection mode switching)实现透明动态控制(transparent dynamic control)。
 *
 * 协作: EnvelopeDetector(包络检测) / MultibandCompressor(多段压缩) / Limiter(限制器)
 */
#pragma once

#include <QObject>
#include <QVector>

class Compressor10 : public QObject {
    Q_OBJECT

public:
    /** @brief Detection mode for envelope tracking */
    enum class DetectionMode {
        RMS,      ///< Root-mean-square level detection
        Peak      ///< Instantaneous peak detection
    };

    /** @brief Compressor parameters */
    struct Parameters {
        double thresholdDb = -20.0;
        double ratio = 4.0;
        double kneeDb = 6.0;
        double attackMs = 10.0;
        double releaseMs = 100.0;
        double makeupGainDb = 0.0;
        double mixPercent = 100.0;  ///< Dry/wet blend (0=dry, 100=compressed)
        DetectionMode mode = DetectionMode::RMS;
    };

    /** @brief Gain reduction report per sample block */
    struct GainReduction {
        double inputLevelDb = -120.0;
        double outputLevelDb = -120.0;
        double reductionDb = 0.0;
        double gainDb = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalBlocks = 0;
        double peakReductionDb = 0.0;
        double avgReductionDb = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Compressor10(QObject *parent = nullptr);
    ~Compressor10() override;

    void setParameters(const Parameters& params);
    void setSampleRate(double rate);

    /** @brief Process a block of audio samples with parallel blending */
    QVector<double> process(const QVector<double>& input);

    /** @brief Get last gain reduction info */
    GainReduction lastGainReduction() const { return m_lastGR; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processDone(int samples, double peakReduction, double timeMs);

private:
    Parameters m_params;
    double m_sampleRate = 44100.0;
    double m_envelope = 0.0;       // current envelope state
    GainReduction m_lastGR;
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_reductionSum = 0.0;

    /** @brief Compute gain reduction for a given input level */
    double computeGainReduction(double inputDb) const;

    /** @brief Convert linear amplitude to dB */
    double linearToDb(double linear) const;

    /** @brief Convert dB to linear amplitude */
    double dbToLinear(double db) const;

    /** @brief Update envelope with attack/release ballistics */
    double updateEnvelope(double sample, double dt);

    /** @brief Compute auto makeup gain */
    double autoMakeupGain() const;

    /** @brief RMS level over a window */
    double rmsLevel(const QVector<double>& window) const;
};
