/**
 * @file FeatureExtractor2.h
 * @brief 特征提取(MFCC梅尔频率倒谱系数+Delta/Delta-Delta特征) — Feature Extraction with Mel-Frequency Cepstral Coefficients and Delta/Delta-Delta Features
 *
 * 功能: 实现MFCC特征提取，支持梅尔滤波器组、DCT倒谱变换、
 *       Delta/Delta-Delta差分特征和能量特征。
 *
 * 协作: FftEngine2(FFT) / WindowFunction3(窗函数) / SignalGenerator4(信号生成)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief MFCC特征提取器(梅尔频率倒谱+差分特征)
 */
class FeatureExtractor2 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalExtractions = 0;
        int frameSize = 0;
        int numCoeffs = 0;
        int numMelBins = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief 特征向量集 */
    struct FeatureSet {
        QVector<QVector<double>> mfcc;          // [frame][coeff]
        QVector<QVector<double>> delta;         // first derivative
        QVector<QVector<double>> deltaDelta;    // second derivative
        QVector<double> energy;                 // log energy per frame
    };

    explicit FeatureExtractor2(QObject *parent = nullptr);
    ~FeatureExtractor2() override;

    void setSampleRate(int sr);
    void setNumCoeffs(int n);
    void setNumMelBins(int n);
    void setFrameSize(int size);
    void setHopSize(int hop);

    /** @brief 提取完整特征集 */
    FeatureSet extract(const QVector<double>& audio);

    /** @brief 计算梅尔滤波器组能量 */
    QVector<double> melFilterBank(const QVector<double>& powerSpectrum) const;

    /** @brief DCT-II变换取倒谱系数 */
    QVector<double> dctCepstral(const QVector<double>& logMelEnergies) const;

    /** @brief 计算Delta差分特征 */
    QVector<QVector<double>> computeDelta(
        const QVector<QVector<double>>& features, int N = 2) const;

    /** @brief Hz转Mel */
    double hzToMel(double hz) const;

    /** @brief Mel转Hz */
    double melToHz(double mel) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void extractionCompleted(int numFrames, int numFeatures);

private:
    int m_sampleRate = 16000;
    int m_numCoeffs = 13;
    int m_numMelBins = 26;
    int m_frameSize = 512;
    int m_hopSize = 256;

    Stats m_stats;
    double m_timeSum = 0.0;

    QVector<QVector<double>> m_melWeights; // precomputed mel filter weights

    /** @brief Build mel filter bank weights */
    void buildMelFilterBank();

    /** @brief Compute power spectrum from frame */
    QVector<double> powerSpectrum(const QVector<double>& frame) const;

    /** @brief Apply Hamming window */
    QVector<double> hammingWindow(const QVector<double>& frame) const;

    /** @brief Radix-2 FFT (magnitude only) */
    void fft(QVector<double>& re, QVector<double>& im) const;
};
