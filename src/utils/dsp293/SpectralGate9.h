/**
 * @file SpectralGate9.h
 * @brief 频谱门限(噪声底估计与时频掩蔽实现语音增强中的音乐噪声抑制) — Spectral Gate with Noise Floor Estimation and Time-frequency Masking for Musical Noise Reduction in Speech Enhancement
 *
 * 功能: 实现频谱门限(spectral gate)，采用噪声底估计(noise floor estimation)
 *       与时频掩蔽(time-frequency masking)实现语音增强中的音乐噪声抑制(musical noise reduction in speech enhancement)。
 *
 * 协作: SpectralSubtractor10(谱减法) / WienerFilter9(维纳滤波) / VoiceActivityDetector8(语音活动检测)
 */
#pragma once

#include <QObject>
#include <QVector>

class SpectralGate9 : public QObject {
    Q_OBJECT

public:
    /** @brief Gate result */
    struct GateResult {
        QVector<double> output;         // Enhanced time-domain signal
        QVector<double> noiseFloor;     // Estimated noise floor spectrum
        QVector<double> mask;           // Applied time-frequency mask
        double snrImprovementDb = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalFrames = 0;
        int fftSize = 0;
        double avgSnrImprovementDb = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SpectralGate9(QObject *parent = nullptr);
    ~SpectralGate9() override;

    void setFftSize(int size);
    void setOverwriteFactor(double factor);     // Overlap factor (0.0-0.75)
    void setNoiseThreshold(double thresh);       // Threshold above noise floor (dB)
    void setSpectralFloor(double floor);         // Spectral floor to avoid zeroing
    void setNoiseEstimationFrames(int frames);   // Frames for initial noise estimate

    /** @brief Process a frame of audio */
    GateResult process(const QVector<double>& frame);

    /** @brief Estimate noise from silence frames */
    void estimateNoise(const QVector<QVector<double>>& silenceFrames);

    /** @brief Reset noise estimate */
    void resetNoise();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void frameProcessed(int frameSize, double snrDb, double timeMs);

private:
    int m_fftSize = 512;
    double m_overlapFactor = 0.5;
    double m_noiseThreshold = 3.0;     // dB above noise floor
    double m_spectralFloor = 0.01;     // Minimum gain
    int m_noiseFrames = 10;
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_snrSum = 0.0;

    QVector<double> m_noiseSpectrum;   // Estimated noise power spectrum
    QVector<double> m_prevPhase;       // Previous frame phase for overlap-add
    QVector<double> m_window;          // Analysis window (Hann)
    QVector<double> m_prevTail;        // Overlap tail from previous frame
    bool m_noiseEstimated = false;

    /** @brief Generate Hann window */
    void generateWindow();

    /** @brief Compute magnitude and phase from complex spectrum */
    void computeSpectrum(const QVector<double>& frame,
                          QVector<double>& magnitude,
                          QVector<double>& phase) const;

    /** @brief Reconstruct time-domain from magnitude/phase */
    QVector<double> reconstructSignal(const QVector<double>& magnitude,
                                       const QVector<double>& phase) const;

    /** @brief Build spectral mask based on noise floor */
    QVector<double> buildMask(const QVector<double>& magnitude) const;

    /** @brief Simple DFT (for frame-sized inputs) */
    void dft(const QVector<double>& input,
             QVector<double>& real, QVector<double>& imag) const;

    /** @brief Inverse DFT */
    void idft(const QVector<double>& real, const QVector<double>& imag,
              QVector<double>& output) const;
};
