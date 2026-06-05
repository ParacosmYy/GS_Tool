/**
 * @file EnvelopeDetect3.h
 * @brief 包络检测3 — 多方法包络+能量跟踪
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class EnvelopeDetect3 : public QObject
{
    Q_OBJECT

public:
    enum Method { Hilbert, Rectification, Squaring, PeakHold };

    struct Stats {
        int totalDetections = 0;
        int totalSamplesProcessed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit EnvelopeDetect3(QObject* parent = nullptr);

    void setMethod(Method method);
    void setSampleRate(double sampleRate);
    void setSmoothing(double timeConstant);
    QVector<double> detect(const QVector<double>& signal);
    double peakEnvelope() const { return m_peakEnvelope; }
    double rmsEnvelope() const { return m_rmsEnvelope; }

    Method method() const { return m_method; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void detectionCompleted(int samples, double peak, double rms);

private:
    Method m_method = Hilbert;
    double m_sampleRate = 44100.0;
    double m_smoothing = 0.01;
    double m_peakEnvelope = 0.0;
    double m_rmsEnvelope = 0.0;

    QVector<double> hilbertEnvelope(const QVector<double>& signal);
    QVector<double> rectificationEnvelope(const QVector<double>& signal);
    QVector<double> squaringEnvelope(const QVector<double>& signal);
    QVector<double> peakHoldEnvelope(const QVector<double>& signal);

    Stats m_stats;
    double m_timeSum = 0.0;
};
