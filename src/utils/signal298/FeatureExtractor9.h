/**
 * @file FeatureExtractor9.h
 * @brief 特征提取(MFCC梅尔频率倒谱系数计算与Delta/加速度导数实现音频指纹) — Feature Extractor with Mel-frequency Cepstral Coefficient Computation and Delta/Acceleration Derivatives for Audio Fingerprinting
 *
 * 功能: 实现特征提取(feature extraction)，采用MFCC梅尔频率倒谱系数计算(MFCC computation)
 *       与Delta/加速度导数(delta/acceleration derivatives)实现音频指纹(audio fingerprinting)。
 *
 * 协作: WHT11(沃尔什-哈达玛变换) / DST12(离散正弦变换) / PitchDetector7(音高检测)
 */
#pragma once

#include <QObject>
#include <QVector>

class FeatureExtractor9 : public QObject {
    Q_OBJECT

public:
    /** @brief MFCC feature vector with derivatives */
    struct FeatureVector {
        QVector<double> mfcc;
        QVector<double> delta;
        QVector<double> acceleration;
        double energy = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalExtractions = 0;
        int frameSize = 0;
        int numCoeffs = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FeatureExtractor9(QObject *parent = nullptr);
    ~FeatureExtractor9() override;

    void setSampleRate(int rate);
    void setNumCoeffs(int n);
    void setNumFilters(int n);

    /** @brief Extract MFCC features from a single frame */
    FeatureVector extract(const QVector<double>& frame);

    /** @brief Extract features from multiple frames */
    QVector<FeatureVector> extractMulti(const QVector<QVector<double>>& frames);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void extractionDone(int coeffs, double energy, double timeMs);

private:
    int m_sampleRate = 44100;
    int m_numCoeffs = 13;
    int m_numFilters = 26;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Precomputed mel filterbank */
    QVector<QVector<double>> m_filterBank;

    /** @brief Precomputed DCT matrix */
    QVector<QVector<double>> m_dctMatrix;

    /** @brief Hamming window */
    QVector<double> m_window;

    /** @brief Build mel-spaced filterbank */
    void buildFilterBank();

    /** @brief Build DCT-II matrix */
    void buildDCT();

    /** @brief Apply Hamming window */
    QVector<double> applyWindow(const QVector<double>& frame) const;

    /** @brief Compute power spectrum via DFT */
    QVector<double> powerSpectrum(const QVector<double>& frame) const;

    /** @brief Convert frequency to mel scale */
    double freqToMel(double f) const;

    /** @brief Convert mel to frequency */
    double melToFreq(double m) const;

    /** @brief Compute delta coefficients */
    QVector<double> computeDelta(const QVector<QVector<double>>& features, int n) const;
};
