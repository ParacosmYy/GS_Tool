#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class SpectralCentroid2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalComputations = 0; int totalFrames = 0; double avgProcessingTimeMs = 0.0; };
    explicit SpectralCentroid2(QObject* parent = nullptr);
    void setSampleRate(double sr);
    void setFFTSize(int n);
    double compute(const QVector<double>& spectrum);
    double centroid() const { return m_centroid; }
    double bandwidth() const { return m_bandwidth; }
    double spread() const { return m_spread; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void computed(double centroid, double bandwidth);
private:
    double m_sampleRate = 44100.0; int m_fftSize = 1024;
    double m_centroid = 0.0; double m_bandwidth = 0.0; double m_spread = 0.0;
    Stats m_stats; double m_timeSum = 0.0;
};
