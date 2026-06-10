/**
 * @file SignalClassifier9.h
 * @brief 信号分类器(统计矩特征与K近邻决策调制识别) — Signal Classifier with Statistical Moment Features and K-Nearest-Neighbor Decision for Modulation Recognition
 *
 * 功能: 实现信号分类器(signal classifier)，采用统计矩特征(statistical moment features)
 *       与K近邻决策(K-nearest-neighbor decision)实现调制识别(modulation recognition)。
 *
 * 协作: WaveletTransform11(小波变换) / Cyclostationary12(循环平稳) / Constellation13(星座图)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 信号分类器(统计矩特征与K近邻决策调制识别)
 */
class SignalClassifier9 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numTemplates = 0;
        int numClassified = 0;
        int numCorrect = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SignalClassifier9(QObject *parent = nullptr);
    ~SignalClassifier9() override;

    /** @brief Set k for k-NN classifier */
    void setK(int k);

    /** @brief Add a labeled template (modulation type + feature vector) */
    void addTemplate(const QString& label, const QVector<double>& features);

    /** @brief Train: batch-add templates from dataset */
    void train(const QVector<QPair<QString, QVector<double>>>& dataset);

    /** @brief Classify a signal by extracting moments and running k-NN */
    QString classify(const QVector<double>& signal) const;

    /** @brief Extract statistical moment features from signal */
    QVector<double> extractFeatures(const QVector<double>& signal) const;

    /** @brief Get k-NN distance for last classification */
    double lastDistance() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void classificationDone(const QString& label, double distance, double timeMs);

private:
    int m_k = 5;

    /** @brief Stored template */
    struct Template {
        QString label;
        QVector<double> features;
    };

    QVector<Template> m_templates;

    // Feature dimension (central moments + higher moments + zero-crossing rate + peak features)
    static constexpr int FEATURE_DIM = 10;

    Stats m_stats;
    double m_timeSum = 0.0;
    mutable double m_lastDistance = 0.0;

    /** @brief Compute n-th central moment of a signal */
    double centralMoment(const QVector<double>& signal, int order) const;

    /** @brief Compute zero-crossing rate */
    double zeroCrossingRate(const QVector<double>& signal) const;

    /** @brief Euclidean distance between two feature vectors */
    double euclideanDist(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief k-NN vote: return majority label among k nearest */
    QString knnVote(const QVector<QPair<double, int>>& distances) const;
};
