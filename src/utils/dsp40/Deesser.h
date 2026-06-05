/**
 * @file Deesser.h
 * @brief 去齿音器 — 唇齿音检测+频段压缩
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class Deesser : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalProcessCalls = 0;
        int totalSamplesProcessed = 0;
        int totalSibilanceDetected = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Deesser(QObject* parent = nullptr);

    void setFrequencyRange(double lowFreq, double highFreq);
    void setThreshold(double thresholdDb);
    void setRatio(double ratio);
    void setAttack(double ms);
    void setRelease(double ms);
    QVector<double> process(const QVector<double>& input);

    double sibilanceLevel() const { return m_sibilanceLevel; }
    bool isSibilant() const { return m_isSibilant; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void sibilanceDetected(double level, double reduction);

private:
    double m_lowFreq = 4000.0;
    double m_highFreq = 10000.0;
    double m_threshold = -20.0;
    double m_ratio = 4.0;
    double m_attack = 5.0;
    double m_release = 50.0;
    double m_sampleRate = 44100.0;
    double m_envelope = 0.0;
    double m_sibilanceLevel = 0.0;
    bool m_isSibilant = false;

    QVector<double> m_bpCoeffs;
    QVector<double> m_bpState;

    void designBandpass();
    QVector<double> bandpass(const QVector<double>& input);

    Stats m_stats;
    double m_timeSum = 0.0;
};
