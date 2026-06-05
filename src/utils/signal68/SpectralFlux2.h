#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class SpectralFlux2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalComputations = 0; int totalFrames = 0; double avgProcessingTimeMs = 0.0; };
    explicit SpectralFlux2(QObject* parent = nullptr);
    void setFFTSize(int n);
    void setHopSize(int hop);
    void setFluxType(const QString& type);
    QVector<double> compute(const QVector<double>& signal);
    double totalFlux() const { return m_totalFlux; }
    QVector<int> peaks() const { return m_peaks; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void computed(double flux, int peakCount);
private:
    int m_fftSize = 1024; int m_hopSize = 512; QString m_type = "positive";
    double m_totalFlux = 0.0; QVector<int> m_peaks;
    Stats m_stats; double m_timeSum = 0.0;
};
