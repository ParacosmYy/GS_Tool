/**
 * @file HarmonicProduct2.h
 * @brief 谐波乘积谱增强 — 基频检测/多谐波/置信度/频域压缩
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QPair>
class HarmonicProduct2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalEstimates = 0; int totalFramesProcessed = 0; double avgProcessingTimeMs = 0.0; };
    explicit HarmonicProduct2(QObject* parent = nullptr);
    void setHarmonics(int n);
    void setSampleRate(double rate);
    void setFFTSize(int size);
    double estimate(const QVector<double>& spectrum);
    QVector<QPair<double,double>> estimateMulti(const QVector<double>& spectrum, int maxF0);
    double confidence() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void pitchEstimated(double frequency, double confidence);
private:
    int m_harmonics = 5; double m_sampleRate = 44100.0; int m_fftSize = 4096;
    double m_confidence = 0.0;
    Stats m_stats; double m_timeSum = 0.0;
};
