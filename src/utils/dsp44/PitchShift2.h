/**
 * @file PitchShift2.h
 * @brief 变调2 — 相位声码器+共振峰保持
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class PitchShift2 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalProcessCalls = 0;
        int totalSamplesProcessed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit PitchShift2(QObject* parent = nullptr);

    void setSampleRate(double sampleRate);
    void setShiftFactor(double factor);
    void setFormantPreservation(bool preserve);
    QVector<double> process(const QVector<double>& input);
    QVector<double> processTimeStretch(const QVector<double>& input,
                                        double stretchFactor);

    double shiftFactor() const { return m_shiftFactor; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int samples, double factor);

private:
    double m_sampleRate = 44100.0;
    double m_shiftFactor = 1.0;
    bool m_formantPreserve = false;
    int m_fftSize = 4096;
    int m_hopSize = 1024;
    QVector<double> m_window;
    QVector<double> m_phaseAccum;

    void buildWindow();
    void fft(QVector<double>& real, QVector<double>& imag) const;
    void ifft(QVector<double>& real, QVector<double>& imag) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
