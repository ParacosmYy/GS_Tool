#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class HarmonicProduct2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDetections = 0; int totalFrames = 0; double avgProcessingTimeMs = 0.0; };
    explicit HarmonicProduct2(QObject* parent = nullptr);
    void setSampleRate(double sr);
    void setNumHarmonics(int n);
    void setFFTSize(int n);
    double detect(const QVector<double>& frame);
    double fundamentalFreq() const { return m_fundFreq; }
    QVector<double> hpsSpectrum() const { return m_hps; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void pitchDetected(double freq, double confidence);
private:
    double m_sampleRate = 44100.0; int m_numHarm = 5; int m_fftSize = 4096;
    double m_fundFreq = 0.0; QVector<double> m_hps;
    Stats m_stats; double m_timeSum = 0.0;
};
