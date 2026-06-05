#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class EnvelopeDetect4 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDetections = 0; int totalSamples = 0; double avgProcessingTimeMs = 0.0; };
    explicit EnvelopeDetect4(QObject* parent = nullptr);
    void setMethod(const QString& method);
    void setSampleRate(double sr);
    QVector<double> detect(const QVector<double>& signal);
    double peakLevel() const { return m_peak; }
    double rmsLevel() const { return m_rms; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void detectionCompleted(int samples, double peak);
private:
    QString m_method = "hilbert"; double m_sampleRate = 44100.0;
    double m_peak = 0.0; double m_rms = 0.0;
    QVector<double> hilbertEnvelope(const QVector<double>& sig) const;
    QVector<double> peakEnvelope(const QVector<double>& sig) const;
    Stats m_stats; double m_timeSum = 0.0;
};
