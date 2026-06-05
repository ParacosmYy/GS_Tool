#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class BeatDetector2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDetections = 0; int totalFrames = 0; double avgProcessingTimeMs = 0.0; };
    explicit BeatDetector2(QObject* parent = nullptr);
    void setSampleRate(double sr);
    void setTempoRange(double minBpm, double maxBpm);
    QVector<double> detect(const QVector<double>& signal);
    double estimatedTempo() const { return m_tempo; }
    QVector<double> beatTimes() const { return m_beats; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void beatDetected(double time, double tempo);
private:
    double m_sampleRate = 44100.0; double m_minBpm = 60.0; double m_maxBpm = 200.0;
    double m_tempo = 120.0; QVector<double> m_beats;
    QVector<double> onsetEnvelope(const QVector<double>& sig);
    double autoCorrelateTempo(const QVector<double>& env);
    Stats m_stats; double m_timeSum = 0.0;
};
