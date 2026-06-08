/**
 * @file NoiseGate6.h
 * @brief 噪声门(最小统计谱底估计+心理声学掩蔽阈值门控) — Noise Gate with Spectral Floor Estimation via Minimum Statistics and Psychoacoustic Masking Threshold Gating
 *
 * 功能: 实现噪声门(noise gate)，采用最小统计(minimum statistics)方法估计功率谱底
 *       (spectral floor)，并利用心理声学掩蔽阈值(psychoacoustic masking threshold)
 *       进行自适应门控(gating)决策。
 *
 * 协作: WienerFilter5(维纳滤波) / SpectralSubtraction4(谱减法) / AdaptiveFilter3(自适应滤波)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 噪声门(最小统计谱底估计+心理声学掩蔽阈值门控)
 */
class NoiseGate6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int frameSize = 0;
        int fftSize = 0;
        double noiseFloorDb = 0.0;
        double avgMaskThreshold = 0.0;
        int framesProcessed = 0;
        int framesGated = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit NoiseGate6(QObject *parent = nullptr);
    ~NoiseGate6() override;

    /** @brief Configure frame size and FFT size */
    bool configure(int frameSize, int fftSize = 0);

    /** @brief Process single frame, returns gated output */
    QVector<double> process(const QVector<double>& frame);

    /** @brief Process entire signal buffer */
    QVector<double> processBuffer(const QVector<double>& buffer);

    /** @brief Get current noise floor estimate per bin */
    QVector<double> noiseFloor() const;

    /** @brief Get current masking thresholds */
    QVector<double> maskingThresholds() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void frameProcessed(int frameIdx, double snrDb, bool gated);
    void bufferCompleted(int frames, int gated, double timeMs);

private:
    int m_frameSize = 512;
    int m_fftSize = 512;
    int m_halfFft = 256;

    // Minimum statistics state
    QVector<double> m_psd;              // current power spectral density
    QVector<double> m_noiseEstimate;    // min-stat noise floor estimate
    QVector<double> m_psdMin;           // running minimum of PSD
    QVector<double> m_psdMinHist;       // minimum over history window
    int m_minHistLen = 50;              // history length for min tracking
    int m_minHistIdx = 0;

    // Psychoacoustic masking
    QVector<double> m_spreadFn;         // spreading function
    QVector<double> m_maskThreshold;    // masking threshold per bin
    double m_absThreshold = 1e-10;      // absolute hearing threshold

    // State
    QVector<double> m_window;           // analysis window (Hann)
    QVector<double> m_twReal;           // precomputed twiddle factors
    QVector<double> m_twImag;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Apply Hann window */
    void applyWindow(QVector<double>& frame) const;

    /** @brief In-place FFT (power-of-2) */
    void fft(QVector<double>& re, QVector<double>& im) const;

    /** @brief In-place IFFT */
    void ifft(QVector<double>& re, QVector<double>& im) const;

    /** @brief Update minimum statistics noise estimate */
    void updateMinStatistics(const QVector<double>& psd);

    /** @brief Compute psychoacoustic masking threshold */
    void computeMaskingThreshold(const QVector<double>& psd);

    /** @brief Precompute Hann window and twiddle factors */
    void precompute();

    /** @brief Initialize spreading function (simplified) */
    void initSpreadingFunction();
};
