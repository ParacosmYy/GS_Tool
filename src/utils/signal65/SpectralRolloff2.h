#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class SpectralRolloff2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalComputations = 0; int totalFrames = 0; double avgProcessingTimeMs = 0.0; };
    explicit SpectralRolloff2(QObject* parent = nullptr);
    void setSampleRate(double sr);
    void setFFTSize(int n);
    void setRolloffPercent(double pct);
    double compute(const QVector<double>& spectrum);
    double rolloffFreq() const { return m_rolloffFreq; }
    double rolloffBin() const { return m_rolloffBin; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void computed(double freq, double bin);
private:
    double m_sampleRate = 44100.0; int m_fftSize = 1024; double m_pct = 0.85;
    double m_rolloffFreq = 0.0; double m_rolloffBin = 0.0;
    Stats m_stats; double m_timeSum = 0.0;
};
