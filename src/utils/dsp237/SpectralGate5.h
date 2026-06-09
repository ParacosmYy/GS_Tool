/**
 * @file SpectralGate5.h
 * @brief 频谱门限(幅度阈值二值掩码+时频平滑抑制音乐噪声) — Spectral Gate with Magnitude-Threshold Binary Masking and Time-Frequency Smoothing for Musical Noise Reduction
 *
 * 功能: 实现频谱门限降噪(Spectral gating)，采用幅度阈值二值掩码(magnitude-threshold binary masking)
 *       和时频平滑(time-frequency smoothing)技术，有效抑制音乐噪声伪影(musical noise artifacts)，
 *       支持频谱减法(spectral subtraction)和维纳滤波(Wiener filtering)模式。
 *
 * 协作: FFT引擎(FFT) / SpectralSubtractor4(频谱减法) / WienerFilter3(维纳滤波)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 频谱门限(幅度阈值二值掩码+时频平滑抑制音乐噪声)
 */
class SpectralGate5 : public QObject {
    Q_OBJECT

public:
    /** @brief Gate mode */
    enum GateMode { BinaryMask = 0, SoftSpectralSubtract = 1, WienerFilter = 2 };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int frameSize = 0;
        int hopSize = 0;
        int numFrames = 0;
        double noiseFloor = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SpectralGate5(QObject *parent = nullptr);
    ~SpectralGate5() override;

    /** @brief Set FFT frame size (power of 2) */
    void setFrameSize(int size);

    /** @brief Set hop size between frames */
    void setHopSize(int hop);

    /** @brief Set gate threshold in dB */
    void setThreshold(double thresholdDb);

    /** @brief Set smoothing factor [0..1] for time-frequency smoothing */
    void setSmoothing(double factor);

    /** @brief Set noise estimation time in seconds */
    void setNoiseEstimationTime(double seconds);

    /** @brief Set gate mode */
    void setMode(GateMode mode);

    /** @brief Process audio buffer and return denoised signal */
    QVector<double> process(const QVector<double>& input);

    /** @brief Estimate noise profile from noise-only segment */
    void estimateNoise(const QVector<double>& noiseSegment);

    /** @brief Get noise spectrum estimate */
    QVector<double> noiseSpectrum() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void frameProcessed(int frame, double snr);
    void processingCompleted(int frames, double timeMs);

private:
    int m_frameSize = 1024;
    int m_hopSize = 512;
    double m_thresholdDb = -30.0;
    double m_smoothing = 0.85;
    double m_noiseTime = 0.5;
    GateMode m_mode = SoftSpectralSubtract;

    QVector<double> m_noiseSpectrum;
    QVector<double> m_window;
    QVector<double> m_prevMask;    // previous frame mask for temporal smoothing
    QVector<double> m_inputBuffer;
    QVector<double> m_outputBuffer;
    QVector<QVector<double>> m_overlapBuf;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute Hann window */
    void computeWindow();

    /** @brief FFT (radix-2 Cooley-Tukey) */
    void fft(QVector<double>& re, QVector<double>& im, bool inverse);

    /** @brief Compute magnitude and phase from complex spectrum */
    void toPolar(const QVector<double>& re, const QVector<double>& im,
                 QVector<double>& mag, QVector<double>& phase) const;

    /** @brief Compute complex from magnitude and phase */
    void toComplex(const QVector<double>& mag, const QVector<double>& phase,
                   QVector<double>& re, QVector<double>& im) const;

    /** @brief Apply binary mask with smoothing */
    QVector<double> applyGate(const QVector<double>& magnitude) const;

    /** @brief Time-frequency smooth mask to reduce musical noise */
    QVector<double> smoothMask(const QVector<double>& mask) const;

    /** @brief Overlap-add synthesis */
    void overlapAdd(const QVector<double>& frame, int pos);

    /** @brief Compute frame SNR */
    double computeSNR(const QVector<double>& clean, const QVector<double>& noisy) const;
};
