#pragma once
#include <QObject>
#include <QVector>
class Beamformer2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalFrames = 0; int totalChannelsProcessed = 0; double avgProcessingTimeMs = 0.0; };
    explicit Beamformer2(QObject* parent = nullptr);
    void setNumChannels(int n); void setSampleRate(double rate);
    void setSteeringAngle(double degrees); void setSpeedOfSound(double speed);
    void setMicSpacing(double meters);
    QVector<double> processDelaySum(const QVector<QVector<double>>& frames);
    QVector<double> processMVDR(const QVector<QVector<double>>& frames);
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void beamformComplete(int channels);
private:
    int m_channels = 4; double m_sampleRate = 44100.0;
    double m_angle = 0.0; double m_speed = 343.0; double m_spacing = 0.05;
    Stats m_stats; double m_timeSum = 0.0;
};
