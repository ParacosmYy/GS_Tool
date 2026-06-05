#pragma once
#include <QObject>
#include <QVector>
/** @brief Envelope detect 2 - Hilbert/peak/RMS/log envelope */
class EnvelopeDetect2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDetections = 0; int totalSamplesProcessed = 0; double avgProcessingTimeMs = 0.0; };
    enum Method { Hilbert, PeakDetect, RMS, LogRMS };
    explicit EnvelopeDetect2(QObject* parent = nullptr);
    void setMethod(Method method); void setAttackTime(double ms);
    void setReleaseTime(double ms); void setSampleRate(double rate);
    QVector<double> detect(const QVector<double>& input);
    double detectOne(double sample);
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void detectionComplete(int samples);
private:
    Method m_method = Hilbert; double m_attack = 1.0; double m_release = 50.0;
    double m_sampleRate = 44100.0; double m_envelope = 0.0;
    Stats m_stats; double m_timeSum = 0.0;
};
