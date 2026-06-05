/**
 * @file Vocoder.h
 * @brief 声码器 — 通道声码器+载波调制
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class Vocoder : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalProcessCalls = 0;
        int totalSamplesProcessed = 0;
        int numChannels = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Vocoder(int numChannels = 16, QObject* parent = nullptr);

    void setSampleRate(double sampleRate);
    void setNumChannels(int channels);
    QVector<double> process(const QVector<double>& modulator,
                            const QVector<double>& carrier);
    QVector<double> envelope(int channel) const;
    QVector<double> channelEnergies() const;

    int numChannels() const { return m_numChannels; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int samples, double energy);

private:
    int m_numChannels = 16;
    double m_sampleRate = 44100.0;
    QVector<double> m_freqLow;
    QVector<double> m_freqHigh;
    QVector<double> m_envelopeState;
    QVector<double> m_channelEnergy;
    QVector<QVector<double>> m_bpCoeffsA;
    QVector<QVector<double>> m_bpCoeffsB;

    void designFilterBank();
    QVector<double> bandpass(const QVector<double>& input, int ch);

    Stats m_stats;
    double m_timeSum = 0.0;
};
