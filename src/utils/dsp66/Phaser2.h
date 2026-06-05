#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class Phaser2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalProcessings = 0; int totalSamples = 0; double avgProcessingTimeMs = 0.0; };
    explicit Phaser2(QObject* parent = nullptr);
    void setRate(double hz);
    void setDepth(double depth);
    void setFeedback(double fb);
    void setStages(int stages);
    void setMix(double mix);
    QVector<double> process(const QVector<double>& input);
    double lfoPosition() const { return m_lfoPos; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void processingCompleted(int samples, double lfoPos);
private:
    double m_rate = 0.5; double m_depth = 0.7; double m_feedback = 0.5;
    int m_stages = 4; double m_mix = 0.5; double m_lfoPos = 0.0;
    QVector<double> m_allpassX; QVector<double> m_allpassY;
    double allpassFilter(double x, double freq);
    Stats m_stats; double m_timeSum = 0.0;
};
