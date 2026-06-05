#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class SpectralContrast2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalComputations = 0; int totalFrames = 0; double avgProcessingTimeMs = 0.0; };
    explicit SpectralContrast2(QObject* parent = nullptr);
    void setSampleRate(double sr);
    void setFFTSize(int n);
    void setNumBands(int bands);
    QVector<double> compute(const QVector<double>& spectrum);
    int numBands() const { return m_numBands; }
    QVector<double> valleyEnergies() const { return m_valleys; }
    QVector<double> peakEnergies() const { return m_peaks; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void computed(int bands, double avgContrast);
private:
    double m_sampleRate = 44100.0; int m_fftSize = 1024; int m_numBands = 6;
    QVector<double> m_valleys; QVector<double> m_peaks;
    Stats m_stats; double m_timeSum = 0.0;
};
