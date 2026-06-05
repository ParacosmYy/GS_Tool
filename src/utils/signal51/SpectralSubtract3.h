#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class SpectralSubtract3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalProcessCalls = 0; int totalFrames = 0; double avgProcessingTimeMs = 0.0; };
    explicit SpectralSubtract3(QObject* parent = nullptr);
    void setSampleRate(double sr);
    void setFFTSize(int n);
    void setOversubtraction(double factor);
    void setFloor(double db);
    void learnNoise(const QVector<double>& noiseFrame);
    QVector<double> process(const QVector<double>& frame);
    bool noiseLearned() const { return m_noiseLearned; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void processingCompleted(double noiseReduction);
private:
    double m_sampleRate = 44100.0; int m_fftSize = 2048;
    double m_oversub = 1.5; double m_floor = -40.0;
    QVector<double> m_noiseProfile; bool m_noiseLearned = false;
    void fft(QVector<double>& re, QVector<double>& im) const;
    void ifft(QVector<double>& re, QVector<double>& im) const;
    Stats m_stats; double m_timeSum = 0.0;
};
