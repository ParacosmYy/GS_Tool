#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class DeEsser3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalProcessings = 0; int totalSamples = 0; double avgProcessingTimeMs = 0.0; };
    explicit DeEsser3(QObject* parent = nullptr);
    void setThreshold(double thresh);
    void setFrequency(double freq);
    void setBandwidth(double bw);
    QVector<double> process(const QVector<double>& input);
    double sibilanceLevel() const { return m_sibLevel; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void processingCompleted(int samples, double reduction);
private:
    double m_threshold = -20.0; double m_freq = 6000.0; double m_bw = 2000.0;
    double m_sibLevel = 0.0;
    QVector<double> bandpassFilter(const QVector<double>& sig, double freq, double bw);
    Stats m_stats; double m_timeSum = 0.0;
};
