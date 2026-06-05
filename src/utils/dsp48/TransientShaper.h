/**
 * @file TransientShaper.h
 * @brief 瞬态塑形器 — 攻击增强+平滑控制
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class TransientShaper : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalProcessCalls = 0;
        int totalSamplesProcessed = 0;
        int transientCount = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TransientShaper(QObject* parent = nullptr);

    void setSampleRate(double sampleRate);
    void setAttack(double attackDb);
    void setSustain(double sustainDb);
    void setSensitivity(double sensitivity);
    QVector<double> process(const QVector<double>& input);
    QVector<double> transientEnvelope() const { return m_transientEnv; }

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transientDetected(double position, double strength);

private:
    double m_sampleRate = 44100.0;
    double m_attack = 6.0;
    double m_sustain = 0.0;
    double m_sensitivity = 0.5;
    QVector<double> m_transientEnv;
    double m_fastEnv = 0.0;
    double m_slowEnv = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;
};
