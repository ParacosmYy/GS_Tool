/**
 * @file PitchDetector10.h
 * @brief 基频检测器(谐波乘积谱与多基频估计) — Pitch Detector with Harmonic Product Spectrum and Multi-pitch Estimation via Spectral GCD for Polyphonic F0 Analysis
 *
 * 功能: 实现基频检测器(pitch detector)，采用谐波乘积谱(harmonic product spectrum)
 *       与多基频估计(multi-pitch estimation via spectral GCD)实现复调F0分析(polyphonic F0 analysis)。
 *
 * 协作: SlidingDFT11(滑动DFT) / Autocorrelation8(自相关) / Cepstrum9(倒谱)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 基频检测器(谐波乘积谱与多基频估计)
 */
class PitchDetector10 : public QObject {
    Q_OBJECT

public:
    /** @brief Pitch detection configuration */
    struct PitchConfig {
        double sampleRate = 44100.0;
        double minFreq = 50.0;        // Minimum F0 (Hz)
        double maxFreq = 2000.0;      // Maximum F0 (Hz)
        int fftSize = 2048;
        int numHarmonics = 5;         // Harmonics for HPS
        double confidenceThreshold = 0.3;
        bool multiPitch = false;      // Enable multi-pitch estimation
    };

    /** @brief Single pitch estimate */
    struct PitchEstimate {
        double frequency = 0.0;
        double confidence = 0.0;
        double clarity = 0.0;         // Harmonic clarity measure
        bool voiced = false;
    };

    /** @brief Multi-pitch result */
    struct PitchResult {
        PitchEstimate primary;
        QVector<PitchEstimate> multipitch;  // Additional F0 candidates
        QVector<double> hpsSpectrum;        // Harmonic product spectrum
        int frameIndex = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int fftSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit PitchDetector10(QObject *parent = nullptr);
    ~PitchDetector10() override;

    void setConfig(const PitchConfig& cfg);

    /** @brief Detect pitch from time-domain signal */
    PitchResult detect(const QVector<double>& frame);

    /** @brief Detect multiple F0 candidates (polyphonic) */
    QVector<PitchEstimate> detectMultiPitch(const QVector<double>& frame);

    /** @brief Compute harmonic product spectrum */
    QVector<double> computeHPS(const QVector<double>& magnitude) const;

    /** @brief Compute spectral GCD for multi-pitch estimation */
    QVector<double> spectralGCD(const QVector<double>& magnitude,
                                 const QVector<double>& freqAxis) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void pitchDetected(int frame, double freq, double conf, double timeMs);

private:
    PitchConfig m_config;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief In-place radix-2 FFT */
    void fft(QVector<double>& re, QVector<double>& im) const;

    /** @brief Bit-reverse index */
    int bitReverse(int x, int bits) const;

    /** @brief Find peaks in HPS spectrum */
    QVector<int> findPeaks(const QVector<double>& spectrum, int maxPeaks) const;

    /** @brief Convert FFT bin index to frequency */
    double binToFreq(int bin) const;

    /** @brief Convert frequency to nearest FFT bin */
    int freqToBin(double freq) const;

    /** @brief Compute clarity from harmonic amplitudes */
    double computeClarity(const QVector<double>& magnitude,
                          int fundBin, int numHarmonics) const;
};
