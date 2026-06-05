#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class EnvelopeDetect5 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDetections = 0; int totalFrames = 0; double avgProcessingTimeMs = 0.0; };
    explicit EnvelopeDetect5(QObject* parent = nullptr);
    void setSampleRate(double sr);
    void setAttackTime(double ms);
    void setReleaseTime(double ms);
    void setMethod(const QString& method);
    QVector<double> detect(const QVector<double>& signal);
    double peakEnvelope() const { return m_peak; }
    double rmsLevel() const { return m_rms; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void detected(double peak, double rms);
private:
    double m_sampleRate = 44100.0; double m_attack = 1.0; double m_release = 50.0;
    QString m_method = "hilbert"; double m_peak = 0.0; double m_rms = 0.0;
    QVector<double> hilbertEnvelope(const QVector<double>& sig);
    QVector<double> peakEnvelope(const QVector<double>& sig);
    Stats m_stats; double m_timeSum = 0.0;
};
