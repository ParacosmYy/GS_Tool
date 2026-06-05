#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class Reverb3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalProcessings = 0; int totalSamples = 0; double avgProcessingTimeMs = 0.0; };
    explicit Reverb3(QObject* parent = nullptr);
    void setRoomSize(double size);
    void setDecayTime(double ms);
    void setDamping(double damp);
    void setPreDelay(double ms);
    void setWetDry(double mix);
    QVector<double> process(const QVector<double>& input);
    double tailLevel() const { return m_tailLevel; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void processingCompleted(int samples, double tailLevel);
private:
    double m_roomSize = 0.8; double m_decay = 2000.0; double m_damping = 0.5;
    double m_preDelay = 20.0; double m_wetDry = 0.3; double m_tailLevel = 0.0;
    QVector<double> m_delayBuf; int m_delayPos = 0;
    Stats m_stats; double m_timeSum = 0.0;
};
