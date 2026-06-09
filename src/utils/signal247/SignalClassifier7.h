/**
 * @file SignalClassifier7.h
 * @brief 信号分类器(频谱特征提取+k近邻距离加权概率投票) — Signal Classifier with Spectral Feature Extraction and K-Nearest-Neighbor Voting with Distance-Weighted Probabilities
 *
 * 功能: 实现信号分类器(Signal Classifier)，通过频谱特征提取(spectral feature
 *       extraction)获取信号特征，使用k近邻(k-nearest-neighbor)算法进行分类，
 *       采用距离加权概率投票(distance-weighted probabilities)提高分类精度。
 *
 * 协作: FftEngine(FFT引擎) / WaveletTransform7(小波变换) / SignalDetector8(信号检测)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief 信号分类器(频谱特征提取+k近邻距离加权概率投票)
 */
class SignalClassifier7 : public QObject {
    Q_OBJECT

public:
    /** @brief Spectral feature vector */
    struct Features {
        double meanFreq = 0.0;         // Spectral centroid
        double peakFreq = 0.0;         // Peak frequency
        double bandwidth = 0.0;        // Spectral bandwidth
        double spectralRolloff = 0.0;  // Roll-off frequency
        double spectralFlatness = 0.0; // Geometric/arithmetic mean ratio
        double zeroCrossRate = 0.0;    // Zero crossing rate
        double rms = 0.0;             // Root mean square energy
        double spectralSkew = 0.0;    // Spectral skewness
        QVector<double> mfcc;         // Mel-frequency cepstral coefficients
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numClasses = 0;
        int numTemplates = 0;
        int kNeighbors = 0;
        int featureDim = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SignalClassifier7(QObject *parent = nullptr);
    ~SignalClassifier7() override;

    /** @brief Set k for k-NN classification */
    void setK(int k);

    /** @brief Set sample rate for feature extraction */
    void setSampleRate(int sr);

    /** @brief Add a labeled template to the database */
    void addTemplate(const QString& label, const QVector<double>& signal);

    /** @brief Extract spectral features from a signal */
    Features extractFeatures(const QVector<double>& signal) const;

    /** @brief Classify a signal, returns label */
    QString classify(const QVector<double>& signal);

    /** @brief Classify with probability scores for each class */
    QVector<QPair<QString, double>> classifyWithProbability(const QVector<double>& signal);

    /** @brief Get all class labels */
    QVector<QString> classes() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void classificationCompleted(const QString& label, double confidence, double timeMs);

private:
    int m_k = 5;
    int m_sampleRate = 44100;

    /** @brief Template entry */
    struct Template {
        QString label;
        Features features;
    };

    QVector<Template> m_templates;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute FFT magnitude spectrum */
    QVector<double> magnitudeSpectrum(const QVector<double>& signal) const;

    /** @brief Compute Euclidean distance between feature vectors */
    double featureDistance(const Features& a, const Features& b) const;

    /** @brief Flatten features to vector for distance computation */
    QVector<double> flattenFeatures(const Features& f) const;

    /** @brief Radix-2 FFT helper */
    void fft1D(QVector<double>& re, QVector<double>& im) const;

    /** @brief Compute Mel filter bank energies */
    QVector<double> melEnergies(const QVector<double>& spectrum) const;

    /** @brief DCT-II for MFCC computation */
    QVector<double> dctII(const QVector<double>& input, int numCoeffs) const;
};
