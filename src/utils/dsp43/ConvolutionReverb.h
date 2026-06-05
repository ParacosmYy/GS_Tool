/**
 * @file ConvolutionReverb.h
 * @brief 卷积混响 — FFT快速卷积+IR加载
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class ConvolutionReverb : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalProcessCalls = 0;
        int totalSamplesProcessed = 0;
        int irLength = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ConvolutionReverb(QObject* parent = nullptr);

    void setSampleRate(double sampleRate);
    void setImpulseResponse(const QVector<double>& ir);
    void setWetDry(double wet, double dry);
    QVector<double> process(const QVector<double>& input);
    QVector<double> impulseResponse() const { return m_ir; }

    int blockSize() const { return m_blockSize; }
    int irLength() const { return m_ir.size(); }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int samples, double wetLevel);

private:
    double m_sampleRate = 44100.0;
    double m_wet = 0.5;
    double m_dry = 1.0;
    int m_blockSize = 2048;
    QVector<double> m_ir;
    QVector<double> m_overlapBuffer;

    QVector<double> fftConvolve(const QVector<double>& a,
                                 const QVector<double>& b) const;
    void fft(QVector<double>& real, QVector<double>& imag) const;
    void ifft(QVector<double>& real, QVector<double>& imag) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
