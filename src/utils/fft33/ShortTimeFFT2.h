/**
 * @file ShortTimeFFT2.h
 * @brief Short-time FFT enhanced - OLA/phase coherence
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QPair>
class ShortTimeFFT2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalTransforms = 0; int totalFramesProcessed = 0; double avgProcessingTimeMs = 0.0; };
    explicit ShortTimeFFT2(QObject* parent = nullptr);
    void setWindowSize(int size);
    void setHopSize(int hop);
    void setWindowType(int type);
    void setSampleRate(double rate);
    QVector<QVector<double>> forward(const QVector<double>& input);
    QVector<double> inverse(const QVector<QVector<double>>& frames);
    QVector<double> spectrogram(const QVector<double>& input);
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void transformComplete(int frames);
private:
    void fft(QVector<double>& real, QVector<double>& imag);
    QVector<double> designWindow(int size) const;
    int m_windowSize = 1024; int m_hopSize = 512; int m_windowType = 0; double m_sampleRate = 44100.0;
    Stats m_stats; double m_timeSum = 0.0;
};
