/**
 * @file ZoomFFT3.h
 * @brief Zoom-FFT3 — 多级细化+带通下采样
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class ZoomFFT3 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalTransforms = 0;
        int totalSamplesProcessed = 0;
        double zoomFactor = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ZoomFFT3(QObject* parent = nullptr);

    void setZoomRange(double centerFreq, double bandwidth, double sampleRate);
    void setZoomFactor(int factor);
    QVector<double> forward(const QVector<double>& signal);
    QVector<double> frequencies() const;

    int outputSize() const { return m_outputSize; }
    double resolution() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int inputSize, int outputSize, double resolution);

private:
    double m_centerFreq = 1000.0;
    double m_bandwidth = 500.0;
    double m_sampleRate = 44100.0;
    int m_zoomFactor = 8;
    int m_outputSize = 512;
    QVector<double> m_bandpassCoeffs;
    QVector<double> m_bandpassState;
    bool m_initialized = false;

    void designBandpass();
    QVector<double> bandpassFilter(const QVector<double>& input);
    QVector<double> decimate(const QVector<double>& input, int factor);

    Stats m_stats;
    double m_timeSum = 0.0;
};
