/**
 * @file PitchDetector8.h
 * @brief 基频检测器(倒谱法+能量零交叉率清浊音判决) — Pitch Detector with Cepstrum Method and Voiced/Unvoiced Classification via Energy and Zero-crossing Rate
 *
 * 功能: 实现基频检测器(Pitch Detector)，采用倒谱法(cepstrum method)
 *       检测基频，通过能量(energy)和零交叉率(zero-crossing rate)
 *       进行清浊音判决(voiced/unvoiced classification)。
 *
 * 协作: FFT4(FFT) / Autocorrelation3(自相关) / WindowFunction5(窗函数)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 基频检测器(倒谱法+能量零交叉率清浊音判决)
 */
class PitchDetector8 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int framesAnalyzed = 0;
        int voicedFrames = 0;
        int unvoicedFrames = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Per-frame analysis result */
    struct FrameResult {
        double pitchHz = 0.0;
        bool isVoiced = false;
        double energy = 0.0;
        double zeroCrossingRate = 0.0;
        double confidence = 0.0;
    };

    explicit PitchDetector8(QObject *parent = nullptr);
    ~PitchDetector8() override;

    /** @brief Set sample rate */
    void setSampleRate(double rate);

    /** @brief Set pitch search range in Hz */
    void setPitchRange(double minHz, double maxHz);

    /** @brief Set frame size in samples */
    void setFrameSize(int samples);

    /** @brief Set hop size in samples */
    void setHopSize(int samples);

    /** @brief Set energy threshold for voiced decision */
    void setEnergyThreshold(double threshold);

    /** @brief Set ZCR threshold for voiced decision */
    void setZCRThreshold(double threshold);

    /** @brief Analyze single frame */
    FrameResult analyzeFrame(const QVector<double>& frame);

    /** @brief Analyze entire signal, returns per-frame results */
    QVector<FrameResult> analyze(const QVector<double>& signal);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void frameAnalyzed(double pitchHz, bool voiced, double timeMs);
    void analysisCompleted(int totalFrames, int voicedCount, double timeMs);

private:
    double m_sampleRate = 44100.0;
    double m_minPitch = 50.0;
    double m_maxPitch = 500.0;
    int m_frameSize = 2048;
    int m_hopSize = 512;
    double m_energyThreshold = 0.01;
    double m_zcrThreshold = 0.15;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute log power spectrum */
    QVector<double> logPowerSpectrum(const QVector<double>& frame) const;

    /** @brief Compute real cepstrum */
    QVector<double> cepstrum(const QVector<double>& frame) const;

    /** @brief Compute frame energy (RMS) */
    double computeEnergy(const QVector<double>& frame) const;

    /** @brief Compute zero-crossing rate */
    double computeZCR(const QVector<double>& frame) const;

    /** @brief Find peak in cepstrum within pitch range */
    double findPitchPeak(const QVector<double>& ceps) const;

    /** @brief Apply Hanning window */
    QVector<double> applyWindow(const QVector<double>& frame) const;
};
