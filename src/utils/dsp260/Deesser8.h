/**
 * @file Deesser8.h
 * @brief 去齿音器(频谱通量唇齿音检测4-10kHz频段感知压缩) — De-esser with Frequency-aware Compression and Sibilance Detection via Spectral Flux in 4-10kHz Band
 *
 * 功能: 实现去齿音器(de-esser)，采用频谱通量(spectral flux)在
 *       4-10kHz频段进行唇齿音(sibilance)检测，结合频率感知压缩
 *       (frequency-aware compression)实现自然齿音抑制。
 *
 * 协作: Compressor10(动态压缩) / Equalizer8(均衡器) / SpectralGate9(频谱门)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 去齿音器(频谱通量唇齿音检测4-10kHz频段感知压缩)
 */
class Deesser8 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numFramesProcessed = 0;
        int numSibilanceDetected = 0;
        double avgReductionDb = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Deesser8(QObject *parent = nullptr);
    ~Deesser8() override;

    /** @brief Set sample rate */
    void setSampleRate(double rate);

    /** @brief Set detection threshold in dB */
    void setThreshold(double thresholdDb);

    /** @brief Set compression ratio for sibilant regions */
    void setRatio(double ratio);

    /** @brief Set attack time in ms */
    void setAttack(double ms);

    /** @brief Set release time in ms */
    void setRelease(double ms);

    /** @brief Process a frame of audio samples */
    QVector<double> process(const QVector<double>& input);

    /** @brief Detect sibilance level in a frame */
    double detectSibilance(const QVector<double>& frame) const;

    /** @brief Get current gain reduction in dB */
    double gainReduction() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void sibilanceDetected(int frame, double level, double reduction);
    void frameProcessed(int frame, double timeMs);

private:
    double m_sampleRate = 44100.0;
    double m_thresholdDb = -20.0;
    double m_ratio = 4.0;
    double m_attackMs = 1.0;
    double m_releaseMs = 50.0;
    int m_fftSize = 1024;

    double m_gainReductionDb = 0.0;
    double m_envelope = 0.0;
    int m_frameCount = 0;

    QVector<double> m_prevMagnitude;   // Previous frame magnitude for spectral flux
    QVector<double> m_window;

    Stats m_stats;
    double m_timeSum = 0.0;
    double m_reductionSum = 0.0;

    /** @brief Apply Hann window */
    void applyWindow(QVector<double>& frame) const;

    /** @brief Compute magnitude spectrum */
    QVector<double> magnitudeSpectrum(const QVector<double>& frame) const;

    /** @brief Compute spectral flux in sibilance band */
    double spectralFlux(const QVector<double>& mag) const;

    /** @brief Compute gain from envelope */
    double computeGain(double inputDb) const;

    /** @brief Simple DFT for magnitude (avoid FFT dependency) */
    void dft(const QVector<double>& input, QVector<double>& real, QVector<double>& imag) const;
};
