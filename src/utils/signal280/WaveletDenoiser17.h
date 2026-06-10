/**
 * @file WaveletDenoiser17.h
 * @brief 小波去噪器(多小波基与多分辨率阈值自适应的复合信号分析去噪) — Wavelet Denoiser with Multi-wavelet Basis and Multi-resolution Threshold Adaptation for Composite Signal Analysis
 *
 * 功能: 实现小波去噪器(Wavelet denoiser)，采用多小波基(multi-wavelet basis)
 *       与多分辨率阈值自适应(multi-resolution threshold adaptation)实现复合信号分析去噪(composite signal analysis)。
 *
 * 协作: WaveletTransform16(小波变换) / KalmanFilter15(卡尔曼滤波) / SavitzkyGolay14(SG滤波)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 小波去噪器(多小波基与多分辨率阈值自适应)
 */
class WaveletDenoiser17 : public QObject {
    Q_OBJECT

public:
    /** @brief Wavelet basis type */
    enum class Basis {
        Haar,
        Daubechies4,
        Daubechies8,
        Symlet6,
        Coiflet3
    };
    Q_ENUM(Basis)

    /** @brief Thresholding method */
    enum class ThresholdMethod {
        Soft,
        Hard,
        SemiSoft,
        Garrote
    };
    Q_ENUM(ThresholdMethod)

    /** @brief Denoising result */
    struct DenoiseResult {
        QVector<double> denoised;
        QVector<double> noiseEstimate;
        double inputSNR = 0.0;
        double outputSNR = 0.0;
        int numLevels = 0;
        double threshold = 0.0;
        int coefficientsZeroed = 0;
        int totalCoefficients = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int signalLength = 0;
        int numLevels = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit WaveletDenoiser17(QObject *parent = nullptr);
    ~WaveletDenoiser17() override;

    /** @brief Set wavelet basis */
    void setBasis(Basis b);

    /** @brief Set decomposition levels */
    void setLevels(int levels);

    /** @brief Set thresholding method */
    void setThresholdMethod(ThresholdMethod method);

    /** @brief Set manual threshold (0 = auto via universal threshold) */
    void setThreshold(double t);

    /** @brief Denoise input signal */
    DenoiseResult denoise(const QVector<double>& input);

    /** @brief Compute universal threshold (VisuShrink) */
    double universalThreshold(const QVector<double>& coefficients) const;

    /** @brief Compute level-dependent threshold (BayesShrink) */
    double bayesThreshold(const QVector<double>& coefficients, double noiseSigma) const;

    /** @brief Estimate noise sigma from finest-level coefficients */
    double estimateNoiseSigma(const QVector<double>& input) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void denoiseDone(int length, int levels, double threshold, double timeMs);

private:
    Basis m_basis = Basis::Daubechies4;
    int m_levels = 4;
    ThresholdMethod m_threshMethod = ThresholdMethod::Soft;
    double m_manualThreshold = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Get filter coefficients for current basis */
    void getLowPassFilter(QVector<double>& h) const;
    void getHighPassFilter(QVector<double>& g) const;

    /** @brief Single-level wavelet decomposition */
    void decompose(const QVector<double>& input, QVector<double>& approx,
                    QVector<double>& detail) const;

    /** @brief Single-level wavelet reconstruction */
    void reconstruct(const QVector<double>& approx, const QVector<double>& detail,
                      QVector<double>& output) const;

    /** @brief Apply thresholding to coefficients */
    double applyThreshold(double value, double threshold) const;

    /** @brief Pad signal to next power of 2 */
    QVector<double> padSignal(const QVector<double>& input, int& paddedLen) const;
};
