/**
 * @file NoiseGate2.h
 * @brief Noise gate enhanced - hysteresis/attack/release/sidechain
 */
#pragma once
#include <QObject>
#include <QVector>
class NoiseGate2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSamplesProcessed = 0; int totalGateEvents = 0; double avgProcessingTimeMs = 0.0; };
    explicit NoiseGate2(QObject* parent = nullptr);
    void setThreshold(double db);
    void setHysteresis(double db);
    void setAttack(double ms);
    void setRelease(double ms);
    void setHold(double ms);
    void setSampleRate(double rate);
    double processOne(double sample);
    QVector<double> process(const QVector<double>& input);
    bool isGateOpen() const;
    void reset();
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void gateChanged(bool open);
private:
    double m_threshold = -40.0; double m_hysteresis = 6.0;
    double m_attack = 1.0; double m_release = 50.0; double m_hold = 10.0;
    double m_sampleRate = 44100.0;
    double m_envelope = 0.0; bool m_gateOpen = false; double m_holdTimer = 0.0; double m_gain = 0.0;
    Stats m_stats; double m_timeSum = 0.0;
};
