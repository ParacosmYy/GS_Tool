#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class GateExpand3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalProcessings = 0; int totalSamples = 0; double avgProcessingTimeMs = 0.0; };
    explicit GateExpand3(QObject* parent = nullptr);
    void setThreshold(double thresh);
    void setAttack(double ms);
    void setRelease(double ms);
    void setHold(double ms);
    void setRange(double db);
    QVector<double> process(const QVector<double>& input);
    double gateGain() const { return m_currentGain; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void processingCompleted(int samples, double avgGain);
private:
    double m_threshold = -40.0; double m_attack = 1.0; double m_release = 100.0;
    double m_hold = 50.0; double m_range = -80.0; double m_currentGain = 1.0;
    Stats m_stats; double m_timeSum = 0.0;
};
