/**
 * @file SpectralGate7.h
 * @brief 谱门控(Wiener滤波估计与噪声底噪减除宽带噪声抑制) — Spectral Gate with Wiener Filter Estimation and Noise Floor Subtraction for Broadband Noise Suppression
 *
 * 功能: 实现谱门控(spectral gate)，采用Wiener滤波估计(Wiener filter estimation)
 *       和噪声底噪减除(noise floor subtraction)实现宽带噪声抑制(broadband noise
 *       suppression)。
 *
 * 协作: FftEngine8(FFT引擎) / WienerFilter6(Wiener滤波器) / NoiseGate5(噪声门)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 谱门控(Wiener滤波估计与噪声底噪减除宽带噪声抑制)
 */
class SpectralGate7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int frameSize = 0;
        int fftSize = 0;
        double noiseReductionDb = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SpectralGate7(QObject *parent = nullptr);
    ~SpectralGate7() override;

    /** @brief Set frame size, FFT size, and noise estimation duration */
    void setParameters(int frameSize, int fftSize = 0, int noiseFrames = 10);

    /** @brief Estimate noise profile from a noise-only segment */
    void estimateNoiseProfile(const QVector<QVector<double>>& noiseFrames);

    /** @brief Process a single frame with spectral gating */
    QVector<double> processFrame(const QVector<double>& frame);

    /** @brief Process entire signal */
    QVector<double> process(const QVector<double>& signal);

    /** @brief Get current noise floor estimate */
    QVector<double> noiseProfile() const;

    /** @brief Set noise profile manually */
    void setNoiseProfile(const QVector<double>& profile);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void frameProcessed(int frameIdx, double snrDb, double timeMs);

private:
    int m_frameSize = 1024;
    int m_fftSize = 2048;
    int m_noiseFrames = 10;
    int m_overlap = 0;

    QVector<double> m_noiseMagnitude;  // Estimated noise spectrum
    QVector<double> m_noisePower;      // Noise power estimate
    QVector<double> m_window;          // Analysis window

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Apply FFT (simple DFT for real-valued signal) */
    void fft(const QVector<double>& real, const QVector<double>& imag,
             QVector<double>& outReal, QVector<double>& outImag) const;

    /** @brief Apply inverse FFT */
    void ifft(const QVector<double>& real, const QVector<double>& imag,
              QVector<double>& outReal, QVector<double>& outImag) const;

    /** @brief Compute magnitude spectrum */
    QVector<double> magnitude(const QVector<double>& re,
                               const QVector<double>& im) const;

    /** @brief Apply Hann window */
    QVector<double> applyWindow(const QVector<double>& frame) const;

    /** @brief Generate Hann window coefficients */
    QVector<double> createHannWindow(int size) const;

    /** @brief Compute Wiener gain for each frequency bin */
    QVector<double> wienerGain(const QVector<double>& signalPower) const;

    /** @brief Compute SNR in dB */
    double computeSnr(const QVector<double>& signal,
                       const QVector<double>& noise) const;
};
