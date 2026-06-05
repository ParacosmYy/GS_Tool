#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class Chromagram2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalComputations = 0; int totalFrames = 0; double avgProcessingTimeMs = 0.0; };
    explicit Chromagram2(QObject* parent = nullptr);
    void setSampleRate(double sr);
    void setFFTSize(int n);
    void setReferenceFreq(double freq);
    QVector<double> compute(const QVector<double>& frame);
    int dominantNote() const { return m_domNote; }
    double chromaEnergy() const { return m_energy; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void chromaComputed(int note, double energy);
private:
    double m_sampleRate = 44100.0; int m_fftSize = 8192; double m_refFreq = 440.0;
    int m_domNote = 0; double m_energy = 0.0;
    Stats m_stats; double m_timeSum = 0.0;
};
