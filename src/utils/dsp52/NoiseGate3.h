#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class NoiseGate3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalProcessCalls = 0; int totalSamples = 0; double avgProcessingTimeMs = 0.0; };
    explicit NoiseGate3(QObject* parent = nullptr);
    void setSampleRate(double sr);
    void setThreshold(double db);
    void setAttack(double ms);
    void setRelease(double ms);
    void setHold(double ms);
    void setRange(double db);
    QVector<double> process(const QVector<double>& input);
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void gateStateChanged(bool open);
private:
    double m_sampleRate = 44100.0; double m_threshold = -40.0;
    double m_attack = 1.0; double m_release = 100.0;
    double m_hold = 50.0; double m_range = -80.0;
    double m_envelope = 0.0; double m_gain = 0.0;
    int m_holdCount = 0; bool m_gateOpen = false;
    Stats m_stats; double m_timeSum = 0.0;
};
