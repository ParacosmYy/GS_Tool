/**
 * @file FeatureExtractor3.h
 * @brief 特征提取(Mel频率带能量+Delta-Delta加速度系数流水线) — Feature Extractor with Mel-Frequency Band Energy and Delta-Delta Acceleration Coefficient Pipeline
 *
 * 功能: 实现Mel频率特征提取，支持Mel滤波器组能量计算、
 *       Delta和Delta-Delta加速度系数流水线。
 *
 * 协作: WindowFunction5(窗函数) / FFT4(快速傅里叶变换) / FeatureExtractor2(特征提取)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 特征提取(Mel频率带能量+Delta-Delta加速度系数流水线)
 */
class FeatureExtractor3 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalFrames = 0;
        int fftSize = 0;
        int numMelBands = 0;
        int numFeatures = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Complete feature frame */
    struct FeatureFrame {
        QVector<double> melEnergies;     // Mel band energies
        QVector<double> melCoefficients; // DCT-compressed coefficients
        QVector<double> delta;           // First-order delta
        QVector<double> deltaDelta;      // Second-order delta-delta
    };

    explicit FeatureExtractor3(QObject *parent = nullptr);
    ~FeatureExtractor3() override;

    /** @brief Initialize with sample rate, FFT size, and Mel bands */
    void init(double sampleRate, int fftSize, int numMelBands);

    /** @brief Extract features from a single frame */
    FeatureFrame extractFrame(const QVector<double>& frame) const;

    /** @brief Extract features from a sequence of frames (with delta) */
    QVector<FeatureFrame> extractSequence(const QVector<QVector<double>>& frames) const;

    /** @brief Compute power spectrum from frame */
    QVector<double> powerSpectrum(const QVector<double>& frame) const;

    /** @brief Compute Mel filter bank energies */
    QVector<double> melFilterEnergies(const QVector<double>& powerSpec) const;

    /** @brief Compute DCT of Mel energies (MFCC) */
    QVector<double> dctMelCoefficients(const QVector<double>& melEnergies, int numCoeffs) const;

    /** @brief Compute delta coefficients from sequence */
    static QVector<QVector<double>> computeDelta(
        const QVector<QVector<double>>& features, int N = 2);

    /** @brief Hz to Mel conversion */
    static double hzToMel(double hz);

    /** @brief Mel to Hz conversion */
    static double melToHz(double mel);

    /** @brief Get Mel filter bank */
    QVector<QVector<double>> melFilterBank() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void extractionCompleted(int frames, int features, double timeMs);

private:
    double m_sampleRate = 44100.0;
    int m_fftSize = 512;
    int m_numMelBands = 26;
    int m_numCoeffs = 13;

    // Precomputed Mel filter bank
    QVector<QVector<double>> m_melFilters;
    QVector<int> m_filterStart;
    QVector<int> m_filterEnd;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build triangular Mel filter bank */
    void buildMelFilterBank();

    /** @brief Apply Hamming window */
    static QVector<double> applyHamming(const QVector<double>& frame);
};
