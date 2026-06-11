/**
 * @file Deesser10.h
 * @brief 去齿音(频谱质心跟踪齿音检测与自适应频率选择性增益衰减) — De-esser with Sibilance Detection via Spectral Centroid Tracking and Adaptive Frequency-selective Gain Reduction
 *
 * 功能: 实现去齿音(De-esser)，采用频谱质心跟踪(spectral centroid tracking)
 *       与自适应频率选择性增益衰减(adaptive frequency-selective gain reduction)。
 *
 * 协作: Compressor20(压缩器) / Equalizer19(均衡器) / Limiter18(限幅器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 去齿音(频谱质心跟踪齿音检测与自适应频率选择性增益衰减)
 */
class Deesser10 : public QObject {
    Q_OBJECT

public:
    /** @brief De-esser parameters */
    struct Params {
        double frequency = 6000.0;      // Sibilance center frequency (Hz)
        double bandwidth = 3000.0;      // Detection bandwidth (Hz)
        double threshold = -20.0;       // Detection threshold (dB)
        double ratio = 4.0;             // Reduction ratio
        double attack = 1.0;            // Attack time (ms)
        double release = 50.0;          // Release time (ms)
        double mix = 1.0;               // Wet/dry mix
    };

    /** @brief Processing result */
    struct ProcessResult {
        QVector<double> output;
        QVector<double> gainReduction;  // Per-block gain reduction (dB)
        QVector<double> centroidTrack;  // Per-block spectral centroid (Hz)
        int sibilanceFrames = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int totalFrames = 0;
        int sibilanceDetected = 0;
        double avgReductionDb = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Deesser10(QObject *parent = nullptr);
    ~Deesser10() override;

    void setParams(const Params& p);
    void setSampleRate(double rate);
    void setFFTSize(int size);

    /** @brief Process audio and reduce sibilance */
    ProcessResult process(const QVector<double>& input);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processDone(int frames, int sibilant, double avgReduction, double timeMs);

private:
    Params m_params;
    double m_sampleRate = 44100.0;
    int m_fftSize = 1024;
    Stats m_stats;
    double m_timeSum = 0.0;

    double m_envelope = 1.0;             // Current gain envelope

    /** @brief Compute spectral centroid of a magnitude spectrum */
    double spectralCentroid(const QVector<double>& mag, const QVector<double>& freqs) const;

    /** @brief Compute magnitude spectrum via windowed FFT */
    void computeSpectrum(const QVector<double>& frame,
                         QVector<double>& mag, QVector<double>& freqs);

    /** @brief Apply spectral gain to a frame */
    void applyGain(QVector<double>& frame, double gainDb);

    /** @brief In-place Cooley-Tukey FFT (power-of-2) */
    void fft(QVector<double>& re, QVector<double>& im, int n);

    /** @brief Bit-reversal permutation */
    void bitReverse(QVector<double>& re, QVector<double>& im, int n);

    /** @brief Hann window */
    static double hann(int i, int N);
};
