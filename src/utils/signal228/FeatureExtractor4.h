/**
 * @file FeatureExtractor4.h
 * @brief 特征提取器(频谱质心/滚降/通量+统计矩管线+z-score归一化) — Feature Extractor with Spectral Centroid/Rolloff/Flux and Statistical Moment Pipeline with Z-Score Normalization
 *
 * 功能: 实现音频/信号特征提取管线，包括频谱质心、频谱滚降、频谱通量、
 *       时域统计矩（均值/方差/偏度/峰度）计算，并支持z-score归一化输出。
 *
 * 协作: FFTCore5(FFT核心) / WindowFunction4(窗函数) / FilterBank3(滤波器组)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 特征提取器(频谱特征+统计矩+z-score归一化)
 */
class FeatureExtractor4 : public QObject {
    Q_OBJECT

public:
    /** @brief Spectral features */
    struct SpectralFeatures {
        double centroid = 0.0;     // spectral centroid (Hz)
        double rolloff = 0.0;      // spectral rolloff 85% (Hz)
        double flux = 0.0;         // spectral flux
        double spread = 0.0;       // spectral spread
        double flatness = 0.0;     // spectral flatness
    };

    /** @brief Statistical moments */
    struct StatMoments {
        double mean = 0.0;
        double variance = 0.0;
        double stdDev = 0.0;
        double skewness = 0.0;
        double kurtosis = 0.0;
    };

    /** @brief Full feature vector for a frame */
    struct FrameFeatures {
        SpectralFeatures spectral;
        StatMoments timeDomain;
        StatMoments freqDomain;
        double rms = 0.0;
        double zcr = 0.0;          // zero crossing rate
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int sampleRate = 44100;
        int frameSize = 1024;
        int numFrames = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FeatureExtractor4(QObject *parent = nullptr);
    ~FeatureExtractor4() override;

    /** @brief Set analysis parameters */
    void setParameters(int sampleRate, int frameSize);

    /** @brief Extract features from single frame */
    FrameFeatures extractFrame(const QVector<double>& frame,
                               const QVector<double>& prevSpectrum) const;

    /** @brief Extract features from full signal with framing */
    QVector<FrameFeatures> extractAll(const QVector<double>& signal) const;

    /** @brief Compute spectral features from magnitude spectrum */
    SpectralFeatures computeSpectral(
        const QVector<double>& magnitude,
        const QVector<double>& prevMagnitude) const;

    /** @brief Compute statistical moments of a signal */
    StatMoments computeMoments(const QVector<double>& data) const;

    /** @brief Z-score normalize a feature matrix (frames x features) */
    QVector<QVector<double>> zscoreNormalize(
        const QVector<QVector<double>>& featureMatrix) const;

    /** @brief Compute RMS energy */
    double computeRMS(const QVector<double>& frame) const;

    /** @brief Compute zero crossing rate */
    double computeZCR(const QVector<double>& frame) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void extractionCompleted(int frames, double timeMs);

private:
    int m_sampleRate = 44100;
    int m_frameSize = 1024;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Apply Hann window */
    QVector<double> hannWindow(const QVector<double>& frame) const;

    /** @brief Compute magnitude spectrum via DFT */
    QVector<double> magnitudeSpectrum(const QVector<double>& windowed) const;

    /** @brief Frequency bin to Hz */
    double binToHz(int bin) const;
};
