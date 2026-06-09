/**
 * @file FeatureExtractor6.h
 * @brief 特征提取(MFCC梅尔频率倒谱系数+delta/delta-delta导数语音特征) — Feature Extractor with Mel-Frequency Cepstral Coefficients and Delta/Delta-Delta Derivatives for Speech Features
 *
 * 功能: 实现特征提取器(feature extractor)，计算梅尔频率倒谱系数(MFCC,
 *       mel-frequency cepstral coefficients)通过梅尔滤波器组(mel filterbank)
 *       和DCT变换，支持delta/delta-delta一阶二阶导数(dynamic features)
 *       用于语音识别特征提取。
 *
 * 协作: FFT3(快速傅里叶变换) / WindowFunction2(窗函数) / DCT9(离散余弦变换)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 特征提取(MFCC+delta/delta-delta导数)
 */
class FeatureExtractor6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int sampleRate = 16000;
        int fftSize = 512;
        int numMelBins = 26;
        int numMfcc = 13;
        int numFrames = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FeatureExtractor6(int sampleRate = 16000, QObject *parent = nullptr);
    ~FeatureExtractor6() override;

    /** @brief Set FFT size (power of 2) */
    void setFftSize(int size);

    /** @brief Set number of mel filter bank bins */
    void setMelBins(int bins);

    /** @brief Set number of MFCC coefficients to output */
    void setMfccCount(int count);

    /** @brief Extract MFCC features from audio samples */
    QVector<QVector<double>> extract(const QVector<double>& audio);

    /** @brief Compute delta (first derivative) features */
    QVector<QVector<double>> delta(const QVector<QVector<double>>& features,
                                   int n = 2) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void extractionCompleted(int frames, int coefficients, double timeMs);

private:
    int m_sampleRate;
    int m_fftSize = 512;
    int m_melBins = 26;
    int m_mfccCount = 13;

    QVector<QVector<double>> m_melFilterbank;  // Mel filter weights

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build mel-spaced filter bank */
    void buildMelFilterbank();

    /** @brief Convert frequency (Hz) to mel scale */
    double hzToMel(double hz) const;

    /** @brief Convert mel scale to frequency (Hz) */
    double melToHz(double mel) const;

    /** @brief Apply Hamming window */
    QVector<double> hammingWindow(const QVector<double>& frame) const;

    /** @brief Compute power spectrum from windowed frame */
    QVector<double> powerSpectrum(const QVector<double>& frame) const;

    /** @brief Apply mel filter bank to power spectrum */
    QVector<double> applyFilterbank(const QVector<double>& spectrum) const;

    /** @brief DCT-II for cepstral coefficients */
    QVector<double> dctII(const QVector<double>& input) const;
};
