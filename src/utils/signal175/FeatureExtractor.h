/**
 * @file FeatureExtractor.h
 * @brief 音频特征提取(MFCC/频谱质心/滚降/通量/过零率/RMS) — Audio Feature Extraction: MFCC, Spectral Centroid/Rolloff/Flux, Zero-Crossing Rate, RMS
 *
 * 功能: 实现音频特征提取，支持MFCC、频谱质心、频谱滚降、频谱通量、
 *       过零率、RMS能量等常用特征。
 *
 * 协作: SplitRadixFFT(FFT) / SpectralRepair(频谱修复) / WienerFilter5(维纳滤波)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 音频特征提取器
 */
class FeatureExtractor : public QObject {
    Q_OBJECT

public:
    /** @brief 特征集 */
    struct Features {
        double rms = 0.0;                 ///< RMS能量
        double zeroCrossingRate = 0.0;    ///< 过零率
        double spectralCentroid = 0.0;    ///< 频谱质心
        double spectralRolloff = 0.0;     ///< 频谱滚降(95%)
        double spectralFlux = 0.0;        ///< 频谱通量
        QVector<double> mfcc;             ///< MFCC系数
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalExtractions = 0;     ///< 累计提取次数
        double avgProcessingTimeMs = 0.0; ///< 平均耗时(ms)
    };

    explicit FeatureExtractor(QObject *parent = nullptr);
    ~FeatureExtractor() override;

    void setSampleRate(double rate);
    void setMfccBins(int bins);
    void setMfccCoeffs(int coeffs);

    /**
     * @brief 从时域信号提取全部特征
     * @param signal 时域信号
     * @return 特征集
     */
    Features extract(const QVector<double>& signal);

    /** @brief 计算RMS能量 */
    static double computeRms(const QVector<double>& signal);

    /** @brief 计算过零率 */
    static double computeZeroCrossingRate(const QVector<double>& signal);

    /** @brief 计算频谱质心 */
    static double computeSpectralCentroid(const QVector<double>& magnitude,
                                          double sampleRate);

    /** @brief 计算频谱滚降 */
    static double computeSpectralRolloff(const QVector<double>& magnitude,
                                         double sampleRate, double threshold = 0.95);

    /** @brief 计算频谱通量 */
    static double computeSpectralFlux(const QVector<double>& magnitude,
                                      const QVector<double>& prevMagnitude);

    /** @brief 计算MFCC */
    QVector<double> computeMfcc(const QVector<double>& signal);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void extractionCompleted(int numFeatures);

private:
    /** @brief 预加重滤波 */
    static QVector<double> preEmphasis(const QVector<double>& signal, double alpha = 0.97);

    /** @brief 汉宁窗 */
    static QVector<double> hanningWindow(int n);

    /** @brief Mel刻度转换 */
    static double hzToMel(double hz);
    static double melToHz(double mel);

    /** @brief 构建Mel滤波器组 */
    QVector<QVector<double>> buildMelFilterBank(int fftSize) const;

    /** @brief DCT-II */
    static QVector<double> dctII(const QVector<double>& input);

    /** @brief 简单FFT(用于内部计算) */
    static void simpleFFT(QVector<double>& real, QVector<double>& imag);

    double m_sampleRate = 44100.0;
    int m_mfccBins = 26;
    int m_mfccCoeffs = 13;

    Stats m_stats;
    double m_timeSum = 0.0;
};
