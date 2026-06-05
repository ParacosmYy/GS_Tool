#pragma once
#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>
class PitchTracker2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalFrames = 0; int totalPitchesDetected = 0; double avgProcessingTimeMs = 0.0; };
    struct Pitch { double frequency = 0.0; double confidence = 0.0; double time = 0.0; };
    explicit PitchTracker2(QObject* parent = nullptr);
    void setSampleRate(double rate); void setFrameSize(int size);
    void setHopSize(int hop); void setMethod(int method);
    QList<Pitch> track(const QVector<double>& audio);
    double detectFrame(const QVector<double>& frame);
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void frameProcessed(int index, double freq, double conf);
private:
    double m_sampleRate = 44100.0; int m_frameSize = 2048;
    int m_hopSize = 512; int m_method = 0;
    Stats m_stats; double m_timeSum = 0.0;
};
