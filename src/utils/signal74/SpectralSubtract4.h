#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class SpectralSubtract4 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDenoisings = 0; int totalFrames = 0; double avgProcessingTimeMs = 0.0; };
    explicit SpectralSubtract4(QObject* parent = nullptr);
    void setFFTSize(int n);
    void setOverSubtraction(double factor);
    void setSpectralFloor(double floor);
    void setNoiseEstimate(const QVector<double>& noise);
    QVector<double> denoise(const QVector<double>& signal);
    QVector<double> noiseEstimate() const { return m_noise; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void denoisingCompleted(int frames, double snr);
private:
    int m_fftSize = 512; double m_overSub = 2.0; double m_floor = 0.02;
    QVector<double> m_noise;
    Stats m_stats; double m_timeSum = 0.0;
};
