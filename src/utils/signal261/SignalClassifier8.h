/**
 * @file SignalClassifier8.h
 * @brief 信号分类器(时频图像表征+归一化互相关模板匹配) — Signal Classifier with Time-Frequency Image Representation and Template Matching via Normalized Cross-Correlation
 *
 * 功能: 实现信号分类器(signal classifier)，将信号转换为时频图像
 *       (time-frequency image)表征，通过归一化互相关(normalized
 *       cross-correlation)模板匹配(template matching)实现信号识别。
 *
 * 协作: FFT4(FFT) / WaveletTransform7(小波变换) / Spectrogram6(频谱图)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 信号分类器(时频图像表征+归一化互相关模板匹配)
 */
class SignalClassifier8 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numTemplates = 0;
        int numClassifications = 0;
        int numCorrect = 0;
        double avgConfidence = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Classification result */
    struct Result {
        int label = -1;
        double confidence = 0.0;
        QVector<double> scores;    // Score per template
    };

    /** @brief Signal template with time-frequency image */
    struct Template {
        int label = -1;
        QString name;
        QVector<QVector<double>> tfImage;  // Time x Frequency
        int timeBins = 0;
        int freqBins = 0;
    };

    explicit SignalClassifier8(QObject *parent = nullptr);
    ~SignalClassifier8() override;

    /** @brief Set STFT window size for time-frequency transform */
    void setWindowSize(int size);

    /** @brief Set STFT hop size */
    void setHopSize(int hop);

    /** @brief Add a template for matching */
    void addTemplate(int label, const QString& name, const QVector<double>& signal);

    /** @brief Classify a signal */
    Result classify(const QVector<double>& signal);

    /** @brief Compute time-frequency image from signal */
    QVector<QVector<double>> computeTFImage(const QVector<double>& signal) const;

    /** @brief Normalized cross-correlation between two 2D images */
    double normalizedCrossCorrelation(const QVector<QVector<double>>& a,
                                       const QVector<QVector<double>>& b) const;

    /** @brief Get all templates */
    QVector<Template> templates() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void classificationCompleted(int label, double confidence, double timeMs);

private:
    int m_windowSize = 256;
    int m_hopSize = 128;
    int m_freqBins = 129;     // windowSize/2 + 1

    QVector<Template> m_templates;

    Stats m_stats;
    double m_timeSum = 0.0;
    double m_confidenceSum = 0.0;

    /** @brief Apply Hann window */
    void applyHannWindow(QVector<double>& frame) const;

    /** @brief Compute magnitude spectrum of a frame */
    QVector<double> magnitudeSpectrum(const QVector<double>& frame) const;

    /** @brief Resize image to match target dimensions via bilinear interp */
    QVector<QVector<double>> resizeImage(
        const QVector<QVector<double>>& img, int targetTime, int targetFreq) const;
};
