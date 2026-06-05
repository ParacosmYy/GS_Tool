/**
 * @file ChirpZ3.h
 * @brief Chirp-Z变换3 — 任意等高线+精细化FFT
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class ChirpZ3 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalTransforms = 0;
        int totalPointsProcessed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ChirpZ3(QObject* parent = nullptr);

    void setParameters(double A_real, double A_imag,
                       double W_real, double W_imag,
                       int M);
    QVector<double> transform(const QVector<double>& real,
                               const QVector<double>& imag);
    QVector<double> frequencies() const;
    QVector<double> zoom(double fLow, double fHigh, int numPoints,
                          double sampleRate);

    int outputSize() const { return m_M; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int inputSize, int outputSize);

private:
    double m_Areal = 1.0;
    double m_Aimag = 0.0;
    double m_Wreal = 1.0;
    double m_Wimag = 0.0;
    int m_M = 256;

    void fft(QVector<double>& real, QVector<double>& imag, int n) const;
    int nextPow2(int n) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
