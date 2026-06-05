#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class DelayLine3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalProcessCalls = 0; int totalSamples = 0; double avgProcessingTimeMs = 0.0; };
    explicit DelayLine3(QObject* parent = nullptr);
    void setDelaySamples(int samples);
    void setFeedback(double fb);
    void setMix(double wet);
    QVector<double> process(const QVector<double>& input);
    void clear();
    int delaySamples() const { return m_delay; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void processingCompleted(int samples);
private:
    int m_delay = 4410; double m_feedback = 0.3; double m_wet = 0.5;
    QVector<double> m_buffer; int m_writeIdx = 0;
    Stats m_stats; double m_timeSum = 0.0;
};
