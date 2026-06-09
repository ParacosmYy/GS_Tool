/**
 * @file WaveletDenoiser14.h
 * @brief 小波去噪器(硬软混合阈值尺度间相关性信号主导重建) — Wavelet Denoiser with Hard/Soft Hybrid Thresholding and Inter-scale Correlation for Signal-dominated Reconstruction
 *
 * 功能: 实现小波去噪器(wavelet denoiser)，采用硬/软混合阈值(hard/
 *       soft hybrid thresholding)和尺度间相关性(inter-scale
 *       correlation)实现信号主导重建(signal-dominated reconstruction)。
 *
 * 协作: WaveletTransform12(小波变换) / FIRFilter8(FIR滤波) / Compressor10(压缩)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 小波去噪器(硬软混合阈值尺度间相关性信号主导重建)
 */
class WaveletDenoiser14 : public QObject {
    Q_OBJECT

public:
    /** @brief Thresholding mode */
    enum ThresholdMode {
        Soft = 0,       // Soft thresholding
        Hard = 1,       // Hard thresholding
        Hybrid = 2      // Hard/soft hybrid
    };
    Q_ENUM(ThresholdMode)

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int signalLength = 0;
        int decomposeLevels = 0;
        double inputSNR = 0.0;
        double outputSNR = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit WaveletDenoiser14(QObject *parent = nullptr);
    ~WaveletDenoiser14() override;

    /** @brief Set decomposition levels */
    void setLevels(int levels);

    /** @brief Set threshold mode */
    void setThresholdMode(ThresholdMode mode);

    /** @brief Set threshold scaling factor */
    void setThresholdScale(double scale);

    /** @brief Denoise input signal */
    QVector<double> denoise(const QVector<double>& signal);

    /** @brief Estimate noise level (MAD of finest detail) */
    double estimateNoise(const QVector<double>& signal) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void denoisingCompleted(int levels, double inputNoise, double outputNoise, double timeMs);

private:
    int m_levels = 4;
    ThresholdMode m_mode = Hybrid;
    double m_thresholdScale = 1.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Haar wavelet forward transform (1 level) */
    void haarForward(const QVector<double>& input,
                     QVector<double>& approx, QVector<double>& detail) const;

    /** @brief Haar wavelet inverse transform (1 level) */
    QVector<double> haarInverse(const QVector<double>& approx,
                                  const QVector<double>& detail) const;

    /** @brief Universal threshold (VisuShrink) */
    double universalThreshold(double sigma, int n) const;

    /** @brief Apply hybrid threshold to coefficients */
    void applyThreshold(QVector<double>& coeffs, double threshold) const;

    /** @brief Compute inter-scale correlation */
    QVector<double> interScaleCorrelation(const QVector<double>& parent,
                                            const QVector<double>& child) const;
};
