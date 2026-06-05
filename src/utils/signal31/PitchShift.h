/**
 * @file PitchShift.h
 * @brief 变调器 — 相位声码器/频域拉伸/重采样/音高缩放
 */
#pragma once
#include <QObject>
#include <QVector>
class PitchShift : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalShifts = 0;
        int totalSamplesProcessed = 0;
        double avgProcessingTimeMs = 0.0;
    };
    explicit PitchShift(QObject* parent = nullptr);
    void setShiftFactor(double factor);
    void setWindowSize(int samples);
    void setHopSize(int samples);
    void setSampleRate(double rate);
    QVector<double> process(const QVector<double>& input);
    QVector<double> pitchShift(const QVector<double>& input, double semitones);
    void reset();
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void shiftComplete(int inputSamples, int outputSamples);
private:
    void forwardFFT(QVector<double>& real, QVector<double>& imag);
    void inverseFFT(QVector<double>& real, QVector<double>& imag);
    double m_shiftFactor = 1.0;
    int m_windowSize = 2048;
    int m_hopSize = 512;
    double m_sampleRate = 44100.0;
    QVector<double> m_prevPhase;
    QVector<double> m_synPhase;
    Stats m_stats;
    double m_timeSum = 0.0;
};
