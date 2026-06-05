#pragma once
#include <QObject>
#include <QVector>
class SpectralSubtract2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalReductions = 0; int totalFramesProcessed = 0; double avgProcessingTimeMs = 0.0; };
    explicit SpectralSubtract2(QObject* parent = nullptr);
    void setSampleRate(double rate); void setFFTSize(int size);
    void setOversubtraction(double alpha); void setFloor(double beta);
    void estimateNoise(const QVector<double>& noise);
    QVector<double> reduce(const QVector<double>& input);
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void reductionComplete(int samples, double snr);
private:
    double m_sampleRate = 44100.0; int m_fftSize = 2048;
    double m_alpha = 2.0; double m_beta = 0.01;
    QVector<double> m_noiseSpectrum;
    Stats m_stats; double m_timeSum = 0.0;
};
