#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class MelSpectrogram3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalComputations = 0; int totalFrames = 0; double avgProcessingTimeMs = 0.0; };
    explicit MelSpectrogram3(QObject* parent = nullptr);
    void setSampleRate(double sr);
    void setFFTSize(int n);
    void setNumMelBins(int bins);
    void setFreqRange(double fmin, double fmax);
    QVector<QVector<double>> compute(const QVector<double>& signal);
    int numMelBins() const { return m_numBins; }
    int numFrames() const { return m_numFrames; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void computed(int frames, int bins);
private:
    double m_sr = 44100.0; int m_fftSize = 2048; int m_numBins = 128;
    double m_fmin = 0.0; double m_fmax = 22050.0; int m_numFrames = 0;
    QVector<QVector<double>> m_filterBank;
    void buildMelFilterBank();
    double hzToMel(double hz) const;
    double melToHz(double mel) const;
    Stats m_stats; double m_timeSum = 0.0;
};
