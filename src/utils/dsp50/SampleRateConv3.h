#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class SampleRateConv3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalConversions = 0; int totalSamples = 0; double avgProcessingTimeMs = 0.0; };
    explicit SampleRateConv3(QObject* parent = nullptr);
    void setRates(double inputRate, double outputRate);
    void setQuality(int taps);
    QVector<double> convert(const QVector<double>& input);
    double inputRate() const { return m_inRate; }
    double outputRate() const { return m_outRate; }
    double ratio() const { return m_ratio; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void conversionCompleted(int inSamples, int outSamples);
private:
    double m_inRate = 44100.0; double m_outRate = 48000.0;
    double m_ratio = 48000.0/44100.0; int m_taps = 64;
    QVector<double> m_filter; bool m_initialized = false;
    void designFilter();
    Stats m_stats; double m_timeSum = 0.0;
};
