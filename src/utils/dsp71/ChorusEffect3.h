#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class ChorusEffect3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalProcessings = 0; int totalSamples = 0; double avgProcessingTimeMs = 0.0; };
    explicit ChorusEffect3(QObject* parent = nullptr);
    void setRate(double hz);
    void setDepth(double ms);
    void setVoices(int v);
    void setFeedback(double fb);
    void setMix(double mix);
    QVector<double> process(const QVector<double>& input);
    int voices() const { return m_voices; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void processingCompleted(int samples);
private:
    double m_rate = 1.5; double m_depth = 7.0; int m_voices = 3;
    double m_feedback = 0.2; double m_mix = 0.5;
    QVector<QVector<double>> m_buffers; QVector<int> m_bufPos;
    double lfo(double phase, int voice) const;
    Stats m_stats; double m_timeSum = 0.0;
};
