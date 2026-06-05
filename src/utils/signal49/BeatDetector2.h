/**
 * @file BeatDetector2.h
 * @brief 节拍检测2 — 模式匹配+节奏跟踪
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class BeatDetector2 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalDetections = 0;
        int totalFrames = 0;
        double tempo = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BeatDetector2(QObject* parent = nullptr);

    void setSampleRate(double sampleRate);
    void setHopSize(int hopSize);
    void setTempoRange(double minBPM, double maxBPM);
    QVector<double> detect(const QVector<double>& signal);
    double estimateTempo(const QVector<double>& onsetEnvelope);
    QVector<double> onsetFunction(const QVector<double>& signal);

    double tempo() const { return m_tempo; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void beatDetected(double time, double strength);
    void tempoUpdated(double bpm);

private:
    double m_sampleRate = 44100.0;
    int m_hopSize = 512;
    double m_minBPM = 60.0;
    double m_maxBPM = 200.0;
    double m_tempo = 120.0;
    QVector<double> m_beatTimes;
    QVector<double> m_onsetEnv;

    QVector<double> computeOnsetEnvelope(const QVector<double>& signal);
    double autoCorrelationTempo(const QVector<double>& env) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
