/**
 * @file HarmonicProduct3.h
 * @brief 谐波积谱3 — 多基频+泛音跟踪
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class HarmonicProduct3 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalComputations = 0;
        int totalFrames = 0;
        int numHarmonics = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit HarmonicProduct3(QObject* parent = nullptr);

    void setSampleRate(double sampleRate);
    void setFFTSize(int fftSize);
    void setNumHarmonics(int n);
    QVector<double> compute(const QVector<double>& spectrum);
    double fundamentalFrequency() const { return m_fundamental; }
    QVector<double> harmonicFrequencies() const { return m_harmonics; }
    QVector<QPair<double,double>> detectMultiPitch(const QVector<double>& spectrum) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void pitchDetected(double freq, double confidence);

private:
    double m_sampleRate = 44100.0;
    int m_fftSize = 4096;
    int m_numHarmonics = 5;
    double m_fundamental = 0.0;
    QVector<double> m_harmonics;

    QVector<double> downsample(const QVector<double>& spec, int factor) const;
    double refinePeak(const QVector<double>& spec, int bin) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
