/**
 * @file GoertzelAlgorithm2.h
 * @brief Goertzel算法2 — 多频并行检测+滑窗
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class GoertzelAlgorithm2 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalDetections = 0;
        int totalSamplesProcessed = 0;
        int numFrequencies = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GoertzelAlgorithm2(QObject* parent = nullptr);

    void setSampleRate(double sampleRate);
    void setBlockSize(int blockSize);
    void addTargetFrequency(double freq);
    void addTargetFrequencies(const QVector<double>& freqs);
    void clearFrequencies();

    QVector<double> compute(const QVector<double>& signal);
    QVector<double> magnitudes() const { return m_magnitudes; }
    QVector<double> phases() const { return m_phases; }
    int detectDTMF(const QVector<double>& signal) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void detectionCompleted(int numFreqs, double peakMagnitude);

private:
    double m_sampleRate = 44100.0;
    int m_blockSize = 256;
    QVector<double> m_targetFreqs;
    QVector<double> m_magnitudes;
    QVector<double> m_phases;

    QPair<double,double> singleGoertzel(const QVector<double>& signal,
                                         double targetFreq) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
