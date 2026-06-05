#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class VoiceActivityDetect2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDetections = 0; int totalFrames = 0; double avgProcessingTimeMs = 0.0; };
    explicit VoiceActivityDetect2(QObject* parent = nullptr);
    void setSampleRate(double sr);
    void setFrameSize(int samples);
    void setThreshold(double db);
    void setHangover(int frames);
    QVector<int> detect(const QVector<double>& signal);
    QVector<double> energies() const { return m_energies; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void voiceActivityChanged(bool active);
private:
    double m_sampleRate = 44100.0; int m_frameSize = 512;
    double m_threshold = -30.0; int m_hangover = 10;
    QVector<double> m_energies;
    Stats m_stats; double m_timeSum = 0.0;
};
