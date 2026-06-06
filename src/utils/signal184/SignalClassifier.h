/**
 * @file SignalClassifier.h
 * @brief 信号分类器(时域特征ZCR/RMS/波峰因子+频域特征频谱质心/带宽) — Signal Classifier with Time-domain Features (ZCR/RMS/Crest) and Frequency-domain Features (Spectral Centroid/Bandwidth)
 *
 * 功能: 实现信号分类器，支持时域特征提取(ZCR/RMS/波峰因子)、
 *       频域特征提取(频谱质心/带宽)和基于特征的KNN分类。
 *
 * 协作: FFT3(FFT) / Goertzel5(Goertzel) / WaveletTransform6(小波变换)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QString>

/**
 * @brief 信号分类器(时域+频域特征提取+KNN分类)
 */
class SignalClassifier : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalClassifications = 0;
        int numClasses = 0;
        int numTemplates = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Extracted feature vector */
    struct Features {
        double zcr = 0.0;              ///< Zero crossing rate
        double rms = 0.0;              ///< Root mean square energy
        double crest = 0.0;            ///< Crest factor (peak/RMS)
        double spectralCentroid = 0.0; ///< Spectral centroid (Hz)
        double bandwidth = 0.0;        ///< Spectral bandwidth (Hz)
        double spectralRolloff = 0.0;  ///< Spectral rolloff (Hz)
    };

    explicit SignalClassifier(QObject *parent = nullptr);
    ~SignalClassifier() override;

    void setSampleRate(double rate);
    void setKNNNeighbors(int k);

    /** @brief 提取时域特征 */
    Features extractFeatures(const QVector<double>& signal) const;

    /** @brief 训练：添加模板信号 */
    void addTemplate(const QString& label, const QVector<double>& signal);

    /** @brief 分类信号 */
    QString classify(const QVector<double>& signal);

    /** @brief 计算ZCR */
    double zeroCrossingRate(const QVector<double>& signal) const;

    /** @brief 计算RMS */
    double rmsEnergy(const QVector<double>& signal) const;

    /** @brief 计算波峰因子 */
    double crestFactor(const QVector<double>& signal) const;

    /** @brief 计算频谱质心 */
    double spectralCentroid(const QVector<double>& signal) const;

    /** @brief 计算频谱带宽 */
    double spectralBandwidth(const QVector<double>& signal) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void classificationCompleted(const QString& label, double confidence);

private:
    double m_sampleRate = 44100.0;
    int m_knnK = 3;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Template entry */
    struct Template {
        QString label;
        Features features;
    };

    QVector<Template> m_templates;

    /** @brief Euclidean distance between feature vectors */
    double featureDistance(const Features& a, const Features& b) const;

    /** @brief Compute spectral rolloff */
    double spectralRolloff(const QVector<double>& signal) const;
};
