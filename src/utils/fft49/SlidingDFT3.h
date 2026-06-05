/**
 * @file SlidingDFT3.h
 * @brief 滑动DFT3 — 稳定化递推+频率跟踪
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class SlidingDFT3 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalUpdates = 0;
        int totalSamplesProcessed = 0;
        int numBins = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SlidingDFT3(QObject* parent = nullptr);

    void setParameters(int fftSize, double sampleRate);
    void pushSample(double sample);
    QVector<double> magnitudes() const;
    QVector<double> phases() const;
    double magnitudeAt(double freq) const;
    double phaseAt(double freq) const;
    QVector<double> trackFrequency(double freq, const QVector<double>& signal);

    int fftSize() const { return m_fftSize; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void frequencyTracked(double freq, double magnitude);

private:
    int m_fftSize = 256;
    double m_sampleRate = 44100.0;
    QVector<double> m_realPart;
    QVector<double> m_imagPart;
    QVector<double> m_cosCoeff;
    QVector<double> m_sinCoeff;
    QVector<double> m_circularBuffer;
    int m_bufferIdx = 0;
    bool m_initialized = false;

    void computeCoefficients();

    Stats m_stats;
    double m_timeSum = 0.0;
};
