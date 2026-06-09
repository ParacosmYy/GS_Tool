/**
 * @file FeatureExtractor5.h
 * @brief 特征提取(时域统计描述符+频谱质心信号表征) — Feature Extractor with Time-Domain Statistical Descriptors and Spectral Centroid for Signal Characterization
 *
 * 功能: 实现信号特征提取(feature extraction)，通过时域统计描述符(time-domain statistical
 *       descriptors)计算均值、方差、偏度、峰度、RMS等指标，结合频谱质心(spectral centroid)
 *       表征信号频率分布中心，实现多维信号特征表征与分类。
 *
 * 协作: BiquadFilter9(滤波) / FeatureScaler6(特征缩放) / AnomalyDetector8(异常检测)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 特征提取(时域统计描述符+频谱质心信号表征)
 */
class FeatureExtractor5 : public QObject {
    Q_OBJECT

public:
    /** @brief Complete feature vector for a signal segment */
    struct FeatureVector {
        double mean = 0.0;
        double variance = 0.0;
        double stdDev = 0.0;
        double skewness = 0.0;
        double kurtosis = 0.0;
        double rms = 0.0;
        double peak = 0.0;
        double peakToPeak = 0.0;
        double crestFactor = 0.0;
        double zeroCrossRate = 0.0;
        double spectralCentroid = 0.0;
        double spectralSpread = 0.0;
        double spectralRolloff = 0.0;
        double energy = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int frameSize = 0;
        int numExtractions = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FeatureExtractor5(QObject *parent = nullptr);
    ~FeatureExtractor5() override;

    /** @brief Set sample rate for spectral features */
    void setSampleRate(int sr);

    /** @brief Extract all features from a signal frame */
    FeatureVector extract(const QVector<double>& frame);

    /** @brief Extract features from multiple frames (batch) */
    QVector<FeatureVector> extractBatch(const QVector<QVector<double>>& frames);

    /** @brief Convert feature vector to flat array */
    static QVector<double> flatten(const FeatureVector& fv);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void extractionCompleted(int frameSize, double timeMs);

private:
    int m_sampleRate = 44100;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute time-domain statistical features */
    void computeTimeDomain(const QVector<double>& frame, FeatureVector& fv) const;

    /** @brief Compute spectral features via DFT */
    void computeSpectral(const QVector<double>& frame, FeatureVector& fv) const;

    /** @brief Simple DFT magnitude spectrum (for small frames) */
    QVector<double> magnitudeSpectrum(const QVector<double>& frame) const;
};
