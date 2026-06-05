/**
 * @file SlidingDFT2.h
 * @brief 滑动DFT增强 — 递推SDFT/调制SDFT/共振器组/实时频谱
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QPair>
class SlidingDFT2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalUpdates = 0; int totalSamplesProcessed = 0; double avgProcessingTimeMs = 0.0; };
    explicit SlidingDFT2(QObject* parent = nullptr);
    void setFFTSize(int n);
    void setSampleRate(double rate);
    void update(double sample);
    QVector<double> magnitudes() const;
    QVector<double> phases() const;
    double magnitudeAt(int bin) const;
    double phaseAt(int bin) const;
    double frequency(int bin) const;
    void reset();
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void spectrumUpdated(int numBins);
private:
    void initCoefficients();
    int m_n = 256; double m_sampleRate = 44100.0;
    QVector<double> m_real, m_imag;
    QVector<double> m_coeffReal, m_coeffImag;
    Stats m_stats; double m_timeSum = 0.0;
};
