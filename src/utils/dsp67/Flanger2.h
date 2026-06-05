#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class Flanger2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalProcessings = 0; int totalSamples = 0; double avgProcessingTimeMs = 0.0; };
    explicit Flanger2(QObject* parent = nullptr);
    void setRate(double hz);
    void setDepth(double ms);
    void setFeedback(double fb);
    void setWaveform(const QString& wave);
    void setMix(double mix);
    QVector<double> process(const QVector<double>& input);
    double lfoValue() const { return m_lfoVal; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void processingCompleted(int samples);
private:
    double m_rate = 0.2; double m_depth = 5.0; double m_feedback = 0.5;
    QString m_wave = "sine"; double m_mix = 0.5; double m_lfoVal = 0.0;
    QVector<double> m_buffer; int m_bufPos = 0;
    double lfo(double phase) const;
    Stats m_stats; double m_timeSum = 0.0;
};
