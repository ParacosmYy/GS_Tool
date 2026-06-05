#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class VoiceActivityDetect3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDetections = 0; int totalFrames = 0; double avgProcessingTimeMs = 0.0; };
    explicit VoiceActivityDetect3(QObject* parent = nullptr);
    void setSampleRate(double sr);
    void setFrameSize(int size);
    void setHangover(int frames);
    QVector<int> detect(const QVector<double>& signal);
    double speechRatio() const { return m_speechRatio; }
    int speechFrames() const { return m_speechFrames; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void voiceDetected(int frame, double energy);
private:
    double m_sampleRate = 16000.0; int m_frameSize = 480; int m_hangover = 10;
    double m_speechRatio = 0.0; int m_speechFrames = 0;
    double computeEnergy(const QVector<double>& frame) const;
    double computeZCR(const QVector<double>& frame) const;
    Stats m_stats; double m_timeSum = 0.0;
};
