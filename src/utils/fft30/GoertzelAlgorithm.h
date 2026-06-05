/**
 * @file GoertzelAlgorithm.h
 * @brief Goertzel算法 — 单频率DFT/频点检测/DTMF解码
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QPair>
class GoertzelAlgorithm : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalDetections = 0;
        int totalSamplesProcessed = 0;
        double avgProcessingTimeMs = 0.0;
    };
    struct FreqResult {
        double frequency = 0.0;
        double magnitude = 0.0;
        double phase = 0.0;
        double power = 0.0;
    };
    explicit GoertzelAlgorithm(QObject* parent = nullptr);
    void setSampleRate(double rate);
    void addTargetFrequency(double freq);
    void addTargetFrequencies(const QVector<double>& freqs);
    FreqResult compute(const QVector<double>& samples, double targetFreq) const;
    QVector<FreqResult> computeAll(const QVector<double>& samples);
    QVector<QPair<double,double>> dtmfDetect(const QVector<double>& samples);
    void clearTargets();
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void detectionComplete(int numFreqs);
    void dtmfToneDetected(char tone, double confidence);
private:
    double m_sampleRate = 44100.0;
    QVector<double> m_targetFreqs;
    Stats m_stats;
    double m_timeSum = 0.0;
};
