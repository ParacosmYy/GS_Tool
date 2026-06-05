/**
 * @file DynamicEQ.h
 * @brief 动态均衡器 — 频段自适应增益控制
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class DynamicEQ : public QObject
{
    Q_OBJECT

public:
    struct BandConfig {
        double freq = 1000.0;
        double bandwidth = 1.0;
        double threshold = -20.0;
        double ratio = 4.0;
        double attack = 10.0;
        double release = 100.0;
        double gain = 0.0;
    };

    struct Stats {
        int totalProcessCalls = 0;
        int totalSamplesProcessed = 0;
        int activeBands = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DynamicEQ(int numBands = 4, QObject* parent = nullptr);

    void setBand(int index, const BandConfig& config);
    void setSampleRate(double sampleRate);
    QVector<double> process(const QVector<double>& input);
    QVector<double> bandGains() const;

    int numBands() const { return m_numBands; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int samples, double peakReduction);

private:
    int m_numBands;
    double m_sampleRate = 44100.0;
    QVector<BandConfig> m_bands;
    QVector<double> m_envelopeState;
    QVector<QVector<double>> m_filterCoeffs;
    bool m_initialized = false;

    void initFilters();
    QVector<double> bandpass(const QVector<double>& input, int band);
    double computeGain(double level, const BandConfig& cfg);

    Stats m_stats;
    double m_timeSum = 0.0;
};
