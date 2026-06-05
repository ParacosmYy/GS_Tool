#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class Cepstrum3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalComputations = 0; int totalFrames = 0; double avgProcessingTimeMs = 0.0; };
    explicit Cepstrum3(QObject* parent = nullptr);
    void setFFTSize(int n);
    void setSampleRate(double sr);
    QVector<double> compute(const QVector<double>& frame);
    double fundamentalFrequency() const { return m_fundFreq; }
    double pitchQuality() const { return m_quality; }
    QVector<double> cepstrum() const { return m_cepstrum; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void computed(double freq, double quality);
private:
    int m_fftSize = 1024; double m_sampleRate = 44100.0;
    double m_fundFreq = 0.0; double m_quality = 0.0;
    QVector<double> m_cepstrum;
    Stats m_stats; double m_timeSum = 0.0;
};
