/**
 * @file SpectralWhitening2.h
 * @brief 谱白化2 — 频谱平坦化+自适应增益
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class SpectralWhitening2 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalProcessCalls = 0;
        int totalFramesProcessed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SpectralWhitening2(QObject* parent = nullptr);

    void setFFTSize(int fftSize);
    void setStrength(double strength);
    void setAdaptive(bool adaptive);
    QVector<double> process(const QVector<double>& spectrum);
    QVector<double> computeWhiteningCurve(const QVector<double>& spectrum) const;
    double spectralFlatness(const QVector<double>& spectrum) const;
    double spectralCentroid(const QVector<double>& spectrum) const;

    int fftSize() const { return m_fftSize; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(double flatnessBefore, double flatnessAfter);

private:
    int m_fftSize = 2048;
    double m_strength = 0.5;
    bool m_adaptive = true;
    QVector<double> m_avgSpectrum;
    int m_frameCount = 0;

    QVector<double> smoothSpectrum(const QVector<double>& spec) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
