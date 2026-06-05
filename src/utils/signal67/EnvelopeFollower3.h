#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class EnvelopeFollower3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalProcessings = 0; int totalSamples = 0; double avgProcessingTimeMs = 0.0; };
    explicit EnvelopeFollower3(QObject* parent = nullptr);
    void setSampleRate(double sr);
    void setAttackTime(double ms);
    void setReleaseTime(double ms);
    void setMode(const QString& mode);
    QVector<double> process(const QVector<double>& input);
    double currentLevel() const { return m_level; }
    double peakLevel() const { return m_peak; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void processingCompleted(int samples, double peak);
private:
    double m_sampleRate = 44100.0; double m_attack = 10.0; double m_release = 100.0;
    QString m_mode = "peak"; double m_level = 0.0; double m_peak = 0.0;
    Stats m_stats; double m_timeSum = 0.0;
};
