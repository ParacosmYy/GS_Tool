/**
 * @file GranularProcessor.h
 * @brief 颗粒处理器 — 颗粒合成+散射+时间拉伸
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class GranularProcessor : public QObject
{
    Q_OBJECT

public:
    struct GrainConfig {
        double position = 0.0;
        double duration = 50.0;
        double pitchShift = 1.0;
        double pan = 0.0;
        double gain = 1.0;
        double shape = 0.5;
    };

    struct Stats {
        int totalProcessCalls = 0;
        int totalGrainsGenerated = 0;
        int activeGrains = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GranularProcessor(QObject* parent = nullptr);

    void setSampleRate(double sampleRate);
    void setGrainDensity(double grainsPerMs);
    void setGrainDuration(double minMs, double maxMs);
    QVector<double> process(const QVector<double>& input, int outputLength);
    QVector<GrainConfig> activeGrainList() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int grains, int samples);

private:
    double m_sampleRate = 44100.0;
    double m_density = 10.0;
    double m_minDuration = 20.0;
    double m_maxDuration = 80.0;
    int m_position = 0;
    int m_nextGrain = 0;
    QVector<GrainConfig> m_activeGrains;

    double grainEnvelope(int sample, int duration, double shape) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
