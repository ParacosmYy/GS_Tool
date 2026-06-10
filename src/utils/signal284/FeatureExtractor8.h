/**
 * @file FeatureExtractor8.h
 * @brief 特征提取器(谱质心与谱通量特征及过零率的音频内容分类) — Feature Extractor with Spectral Centroid and Spectral Flux Features with Zero-crossing Rate for Audio Content Classification
 *
 * 功能: 实现特征提取器(feature extractor)，采用谱质心(spectral centroid)
 *       与谱通量(spectral flux)特征及过零率(zero-crossing rate)实现音频内容分类(audio content classification)。
 *
 * 协作: FFT10(FFT) / WindowFunction7(窗函数) / PitchDetector6(音高检测)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 特征提取器(谱质心与谱通量特征及过零率)
 */
class FeatureExtractor8 : public QObject {
    Q_OBJECT

public:
    /** @brief Audio feature vector */
    struct FeatureVector {
        double spectralCentroid = 0.0;
        double spectralFlux = 0.0;
        double zeroCrossingRate = 0.0;
        double rmsEnergy = 0.0;
        double spectralRolloff = 0.0;
        double bandwidth = 0.0;
    };

    /** @brief Full extraction result */
    struct ExtractionResult {
        QVector<FeatureVector> frameFeatures;
        FeatureVector globalFeatures;
        int numFrames = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int frameSize = 512;
        int sampleRate = 44100;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FeatureExtractor8(QObject *parent = nullptr);
    ~FeatureExtractor8() override;

    /** @brief Set frame size for analysis */
    void setFrameSize(int size);

    /** @brief Set sample rate */
    void setSampleRate(int rate);

    /** @brief Set hop size (frame shift in samples) */
    void setHopSize(int hop);

    /** @brief Extract features from entire audio signal */
    ExtractionResult extract(const QVector<double>& audio);

    /** @brief Compute spectral centroid from magnitude spectrum */
    double spectralCentroid(const QVector<double>& magnitude) const;

    /** @brief Compute spectral flux between consecutive frames */
    double spectralFlux(const QVector<double>& magCurr,
                         const QVector<double>& magPrev) const;

    /** @brief Compute zero-crossing rate of a frame */
    double zeroCrossingRate(const QVector<double>& frame) const;

    /** @brief Compute RMS energy of a frame */
    double rmsEnergy(const QVector<double>& frame) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void frameDone(int frameIdx, double centroid, double zcr, double timeMs);
    void extractionDone(int numFrames, double globalCentroid, double timeMs);

private:
    int m_frameSize = 512;
    int m_sampleRate = 44100;
    int m_hopSize = 256;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Hann window coefficients */
    QVector<double> m_window;

    /** @brief Precompute Hann window */
    void precomputeWindow();

    /** @brief Compute magnitude spectrum via DFT */
    QVector<double> computeMagnitude(const QVector<double>& frame) const;

    /** @brief Compute spectral rolloff (freq below which 85% of energy) */
    double spectralRolloff(const QVector<double>& magnitude) const;

    /** @brief Compute spectral bandwidth around centroid */
    double spectralBandwidth(const QVector<double>& magnitude, double centroid) const;
};
