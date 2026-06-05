#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class SpectralFlatness2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalComputations = 0; int totalFrames = 0; double avgProcessingTimeMs = 0.0; };
    explicit SpectralFlatness2(QObject* parent = nullptr);
    void setFFTSize(int n);
    void setBandRange(int loBin, int hiBin);
    double compute(const QVector<double>& spectrum);
    double flatness() const { return m_flatness; }
    double geometricMean() const { return m_geoMean; }
    double arithmeticMean() const { return m_ariMean; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void computed(double flatness, double wienerEntropy);
private:
    int m_fftSize = 1024; int m_loBin = 0; int m_hiBin = 512;
    double m_flatness = 0.0; double m_geoMean = 0.0; double m_ariMean = 0.0;
    Stats m_stats; double m_timeSum = 0.0;
};
