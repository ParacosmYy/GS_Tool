/**
 * @file Phaser.h
 * @brief 移相器 — 全通滤波器链/LFO调制/反馈/级联级
 */
#pragma once
#include <QObject>
#include <QVector>
class Phaser : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSamplesProcessed = 0; double avgProcessingTimeMs = 0.0; };
    explicit Phaser(QObject* parent = nullptr);
    void setRate(double hz);
    void setDepth(double depth);
    void setFeedback(double fb);
    void setStages(int stages);
    void setSampleRate(double rate);
    double processOne(double sample);
    QVector<double> process(const QVector<double>& input);
    void reset();
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void processingComplete(int samples);
private:
    double allpassFilter(double sample, int stage);
    double currentLFOValue() const;
    void setLFOWaveform(int type);
    QVector<double> processWithMix(const QVector<double>& input, double dryWet);
    double phaseResponse(double freq) const;
    double magnitudeResponse(double freq) const;
    double m_rate = 0.5; double m_depth = 0.7;
    double m_feedback = 0.5; int m_stages = 4;
    double m_sampleRate = 44100.0;
    double m_phase = 0.0; double m_feedbackBuf = 0.0;
    QVector<double> m_x1, m_y1;
    Stats m_stats; double m_timeSum = 0.0;
};
