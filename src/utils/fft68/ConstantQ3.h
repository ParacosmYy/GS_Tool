#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class ConstantQ3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalTransforms = 0; int totalBins = 0; double avgProcessingTimeMs = 0.0; };
    explicit ConstantQ3(QObject* parent = nullptr);
    void setMinFreq(double fmin);
    void setMaxFreq(double fmax);
    void setBinsPerOctave(int bpo);
    void setSampleRate(double sr);
    QVector<double> forward(const QVector<double>& signal);
    int numBins() const { return m_numBins; }
    double qualityFactor() const { return m_Q; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void transformCompleted(int bins, double Q);
private:
    double m_fmin = 55.0; double m_fmax = 7040.0; int m_bpo = 24; double m_sr = 44100.0;
    int m_numBins = 0; double m_Q = 0.0;
    QVector<QVector<double>> m_kernels;
    void computeKernels();
    Stats m_stats; double m_timeSum = 0.0;
};
