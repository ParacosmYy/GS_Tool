#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class ChorusEffect2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalProcessCalls = 0; int totalSamples = 0; double avgProcessingTimeMs = 0.0; };
    explicit ChorusEffect2(int voices = 3, QObject* parent = nullptr);
    void setSampleRate(double sr);
    void setRate(double hz);
    void setDepth(double ms);
    void setMix(double wet);
    QVector<double> process(const QVector<double>& input);
    int numVoices() const { return m_voices; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void processingCompleted(int samples);
private:
    int m_voices = 3; double m_sampleRate = 44100.0;
    double m_rate = 1.5; double m_depth = 5.0; double m_mix = 0.5;
    QVector<QVector<double>> m_buffers;
    QVector<int> m_writeIdx; QVector<double> m_phase;
    Stats m_stats; double m_timeSum = 0.0;
};
