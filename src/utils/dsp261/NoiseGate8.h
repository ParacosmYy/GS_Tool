/**
 * @file NoiseGate8.h
 * @brief 噪声门(谱减法预处理自适应阈值噪声底跟踪) — Noise Gate with Spectral Subtraction Pre-processing and Adaptive Threshold via Noise Floor Tracking
 *
 * 功能: 实现噪声门(noise gate)信号处理器，采用谱减法预处理(spectral
 *       subtraction pre-processing)和自适应阈值(adaptive threshold)通过
 *       噪声底跟踪(noise floor tracking)实现动态噪声抑制。
 *
 * 协作: NoiseProfiler7(噪声分析) / FirFilter9(滤波器) / WaveletDenoise8(小波降噪)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 噪声门(谱减法预处理自适应阈值噪声底跟踪)
 */
class NoiseGate8 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int frameSize = 0;
        int numFramesProcessed = 0;
        double avgThreshold = 0.0;
        double noiseFloorDb = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit NoiseGate8(QObject *parent = nullptr);
    ~NoiseGate8() override;

    /** @brief Set frame size for spectral processing */
    void setFrameSize(int size);

    /** @brief Set base threshold in dB */
    void setBaseThresholdDb(double thresholdDb);

    /** @brief Set noise floor estimation alpha (0-1, lower = slower tracking) */
    void setNoiseAlpha(double alpha);

    /** @brief Set spectral subtraction over-subtraction factor */
    void setSpectralSubtractionFactor(double factor);

    /** @brief Learn noise profile from a noise-only segment */
    void learnNoiseProfile(const QVector<double>& noiseSegment);

    /** @brief Process a frame of audio samples */
    QVector<double> process(const QVector<double>& frame);

    /** @brief Get current adaptive threshold in dB */
    double currentThresholdDb() const;

    /** @brief Get estimated noise floor spectrum */
    QVector<double> noiseFloorSpectrum() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void frameProcessed(int frameIndex, double thresholdDb, double snrDb, double timeMs);
    void noiseProfileUpdated(double noiseFloorDb);

private:
    int m_frameSize = 512;
    double m_baseThresholdDb = -40.0;
    double m_noiseAlpha = 0.98;
    double m_subtractFactor = 2.0;
    double m_spectralFloor = 0.01;

    QVector<double> m_noiseSpectrum;
    QVector<double> m_window;
    double m_noiseFloor = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Apply Hann window to a frame */
    QVector<double> applyWindow(const QVector<double>& frame) const;

    /** @brief Compute magnitude spectrum via DFT */
    QVector<double> magnitudeSpectrum(const QVector<double>& windowed) const;

    /** @brief Spectral subtraction: clean = |X| - alpha * |N| */
    QVector<double> spectralSubtract(const QVector<double>& magnitude) const;

    /** @brief Update noise floor estimate from current frame energy */
    void updateNoiseFloor(const QVector<double>& magnitude);

    /** @brief Compute adaptive threshold from noise floor */
    double computeAdaptiveThreshold() const;

    /** @brief RMS energy in dB */
    double rmsDb(const QVector<double>& samples) const;

    /** @brief Reconstruct time-domain from modified spectrum + original phase */
    QVector<double> reconstruct(const QVector<double>& modifiedMag,
                                 const QVector<double>& originalFrame) const;

    /** @brief Initialize Hann window coefficients */
    void initWindow();
};
