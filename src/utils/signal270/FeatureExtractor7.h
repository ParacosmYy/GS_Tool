/**
 * @file FeatureExtractor7.h
 * @brief 特征提取(MFCC梅尔频率倒谱系数与Delta/Delta-Delta导数音频指纹) — Feature Extractor with Mel-Frequency Cepstral Coefficients and Delta/Delta-Delta Derivatives for Audio Fingerprinting
 *
 * 功能: 实现特征提取(Feature extraction)，采用MFCC梅尔频率倒谱系数(Mel-frequency cepstral
 *       coefficients)与Delta/Delta-Delta导数(Delta/Delta-Delta derivatives)用于音频指纹。
 *
 * 协作: WindowFunction8(窗函数) / FilterBank6(滤波器组) / PitchDetector7(音高检测)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 特征提取(MFCC梅尔频率倒谱系数与Delta/Delta-Delta导数音频指纹)
 */
class FeatureExtractor7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int frameSize = 0;
        int numFrames = 0;
        int numCoeffs = 0;
        int numMelBins = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FeatureExtractor7(QObject *parent = nullptr);
    ~FeatureExtractor7() override;

    /** @brief Set FFT frame size (must be power of 2) */
    void setFrameSize(int size);

    /** @brief Set number of MFCC coefficients */
    void setNumCoeffs(int num);

    /** @brief Set number of Mel filter banks */
    void setNumMelBins(int num);

    /** @brief Set sample rate */
    void setSampleRate(int rate);

    /** @brief Extract MFCC features from audio samples */
    QVector<QVector<double>> extractMFCC(const QVector<double>& audio);

    /** @brief Compute delta (first derivative) of feature matrix */
    QVector<QVector<double>> computeDelta(
        const QVector<QVector<double>>& features, int n = 2) const;

    /** @brief Compute delta-delta (second derivative) */
    QVector<QVector<double>> computeDeltaDelta(
        const QVector<QVector<double>>& features, int n = 2) const;

    /** @brief Compute full feature vector: MFCC + delta + delta-delta */
    QVector<QVector<double>> extractFullFeatures(const QVector<double>& audio);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void extractionCompleted(int numFrames, int numCoeffs, double timeMs);

private:
    int m_frameSize = 512;
    int m_numCoeffs = 13;
    int m_numMelBins = 26;
    int m_sampleRate = 44100;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Apply Hamming window */
    QVector<double> hammingWindow(const QVector<double>& frame) const;

    /** @brief Compute power spectrum via DFT */
    QVector<double> powerSpectrum(const QVector<double>& frame) const;

    /** @brief Build Mel filter bank (m_numMelBins filters) */
    QVector<QVector<double>> buildMelFilterBank(int fftSize) const;

    /** @brief Convert frequency to Mel scale */
    static double hzToMel(double hz);

    /** @brief Convert Mel scale to frequency */
    static double melToHz(double mel);

    /** @brief Apply DCT-II to get cepstral coefficients */
    QVector<double> dctII(const QVector<double>& input) const;

    /** @brief Frame audio into overlapping windows */
    QVector<QVector<double>> frameAudio(const QVector<double>& audio,
                                         int hopSize) const;
};
