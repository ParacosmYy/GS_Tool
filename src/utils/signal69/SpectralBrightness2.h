#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class SpectralBrightness2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalComputations = 0; int totalFrames = 0; double avgProcessingTimeMs = 0.0; };
    explicit SpectralBrightness2(QObject* parent = nullptr);
    void setSampleRate(double sr);
    void setFFTSize(int n);
    void setCutoffFreq(double freq);
    double compute(const QVector<double>& spectrum);
    double brightness() const { return m_brightness; }
    double lowEnergy() const { return m_lowE; }
    double highEnergy() const { return m_highE; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void computed(double brightness, double ratio);
private:
    double m_sampleRate = 44100.0; int m_fftSize = 1024; double m_cutoff = 4000.0;
    double m_brightness = 0.0; double m_lowE = 0.0; double m_highE = 0.0;
    Stats m_stats; double m_timeSum = 0.0;
};
