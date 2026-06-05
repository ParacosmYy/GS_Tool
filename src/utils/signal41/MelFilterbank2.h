/**
 * @file MelFilterbank2.h
 * @brief Mel滤波器组2 — MFCC特征提取+三角/Slaney滤波
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class MelFilterbank2 : public QObject
{
    Q_OBJECT

public:
    enum FilterType { Triangular, Slaney };

    struct Stats {
        int totalComputations = 0;
        int totalFrames = 0;
        int numFilters = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MelFilterbank2(QObject* parent = nullptr);

    void setParameters(int fftSize, int numFilters, double sampleRate,
                       double lowFreq = 0.0, double highFreq = 0.0,
                       FilterType type = Triangular);
    QVector<double> compute(const QVector<double>& powerSpectrum);
    QVector<double> dct(const QVector<double>& melSpectrum, int numCoeffs = 13) const;
    QVector<double> melFrequencies() const;

    int fftSize() const { return m_fftSize; }
    int numFilters() const { return m_numFilters; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void computationCompleted(int numFilters, double energy);

private:
    int m_fftSize = 512;
    int m_numFilters = 26;
    double m_sampleRate = 44100.0;
    double m_lowFreq = 0.0;
    double m_highFreq = 0.0;
    FilterType m_filterType = Triangular;
    QVector<QVector<double>> m_filterBank;
    QVector<double> m_melFreqs;
    bool m_initialized = false;

    double hzToMel(double hz) const;
    double melToHz(double mel) const;
    void buildFilterBank();

    Stats m_stats;
    double m_timeSum = 0.0;
};
