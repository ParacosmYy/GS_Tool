/**
 * @file ConstantQ3.h
 * @brief 常数Q变换3 — 多分辨率时频+Chordino
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class ConstantQ3 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalTransforms = 0;
        int totalFrames = 0;
        int totalBins = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ConstantQ3(QObject* parent = nullptr);

    void setParameters(double minFreq, double maxFreq, int binsPerOctave,
                       double sampleRate, double threshold = 0.01);
    QVector<QVector<double>> forward(const QVector<double>& signal, int hopSize = 512);
    QVector<double> chroma(const QVector<QVector<double>>& cqSpectrum) const;
    QVector<double> frequencies() const;
    QVector<double> qualityFactors() const;

    int totalBins() const { return m_totalBins; }
    int octaves() const { return m_octaves; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int frames, int bins);

private:
    double m_minFreq = 55.0;
    double m_maxFreq = 7040.0;
    int m_binsPerOctave = 12;
    double m_sampleRate = 44100.0;
    double m_threshold = 0.01;
    int m_octaves = 7;
    int m_totalBins = 84;
    int m_fftSize = 8192;

    QVector<QVector<double>> m_kernels;
    QVector<int> m_fftBins;
    QVector<double> m_freqs;

    void buildKernels();

    Stats m_stats;
    double m_timeSum = 0.0;
};
