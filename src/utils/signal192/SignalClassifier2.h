/**
 * @file SignalClassifier2.h
 * @brief 信号分类器(统计矩+谱熵特征) — Signal Classifier with Statistical Moments (Skewness/Kurtosis) and Spectral Entropy Features
 *
 * 功能: 实现信号分类器，支持统计矩特征(偏度/峰度)、
 *       谱熵(Spectral Entropy)特征提取、多类别分类和特征向量输出。
 *
 * 协作: SignalGenerator8(信号生成) / FftEngine3(FFT引擎) / WaveletTransform5(小波变换)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QString>

/**
 * @brief 信号分类器(统计矩+谱熵特征)
 */
class SignalClassifier2 : public QObject {
    Q_OBJECT

public:
    /** @brief Signal type classification labels */
    enum SignalType {
        SineWave = 0,
        SquareWave = 1,
        TriangleWave = 2,
        SawtoothWave = 3,
        WhiteNoise = 4,
        Impulse = 5,
        AmModulated = 6,
        FmModulated = 7,
        Unknown = 8
    };

    /** @brief Extracted feature vector */
    struct Features {
        double mean = 0.0;
        double variance = 0.0;
        double skewness = 0.0;
        double kurtosis = 0.0;
        double rms = 0.0;
        double zeroCrossRate = 0.0;
        double spectralEntropy = 0.0;
        double spectralCentroid = 0.0;
        double spectralFlatness = 0.0;
        double peakFactor = 0.0;
    };

    /** @brief Classification result */
    struct Result {
        SignalType type = Unknown;
        double confidence = 0.0;
        QVector<QPair<SignalType, double>> scores;
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalClassifications = 0;
        int signalLength = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SignalClassifier2(QObject *parent = nullptr);
    ~SignalClassifier2() override;

    void setSampleRate(double sr);
    void setFftSize(int N);

    /** @brief Classify a signal segment */
    Result classify(const QVector<double>& signal);

    /** @brief Extract statistical + spectral features */
    Features extractFeatures(const QVector<double>& signal) const;

    /** @brief Convert SignalType to string */
    static QString signalTypeName(SignalType type);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void classificationCompleted(int type, double confidence, double timeMs);

private:
    double m_sampleRate = 44100.0;
    int m_fftSize = 1024;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute statistical moments */
    void computeMoments(const QVector<double>& sig, Features& f) const;

    /** @brief Compute spectral entropy from power spectrum */
    double computeSpectralEntropy(const QVector<double>& power) const;

    /** @brief Compute FFT magnitude spectrum */
    QVector<double> computeSpectrum(const QVector<double>& sig) const;

    /** @brief Score signal against known templates */
    QVector<QPair<SignalType, double>> scoreTemplates(
        const Features& f) const;
};
