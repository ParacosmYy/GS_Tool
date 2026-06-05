/**
 * @file PhaseCorrelator2.h
 * @brief 相位相关器2 — 亚像素配准+频域位移估计
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class PhaseCorrelator2 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalCorrelations = 0;
        int totalPixelsProcessed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit PhaseCorrelator2(QObject* parent = nullptr);

    QPair<double,double> correlate(const QVector<double>& ref,
                                   const QVector<double>& target,
                                   int width, int height);
    QVector<double> crossPowerSpectrum() const { return m_cps; }
    double peakValue() const { return m_peakValue; }
    double confidence() const { return m_confidence; }

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void correlationCompleted(double dx, double dy, double confidence);

private:
    QVector<double> m_cps;
    double m_peakValue = 0.0;
    double m_confidence = 0.0;

    void fft2D(QVector<double>& real, QVector<double>& imag, int w, int h);
    void ifft2D(QVector<double>& real, QVector<double>& imag, int w, int h);
    QPair<double,double> subpixelPeak(const QVector<double>& surface,
                                       int w, int h, int px, int py) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
