#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class SampleRateConv4 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalConversions = 0; int totalSamples = 0; double avgProcessingTimeMs = 0.0; };
    explicit SampleRateConv4(QObject* parent = nullptr);
    void setSourceRate(double sr);
    void setTargetRate(double tr);
    void setFilterLength(int len);
    void setWindowType(const QString& type);
    QVector<double> process(const QVector<double>& input);
    double ratio() const { return m_target / m_source; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void conversionCompleted(int inLen, int outLen);
private:
    double m_source = 44100.0; double m_target = 48000.0;
    int m_filterLen = 64; QString m_winType = "kaiser";
    QVector<double> m_filter;
    void designFilter();
    QVector<double> polyphaseUp(const QVector<double>& in, int factor);
    QVector<double> polyphaseDown(const QVector<double>& in, int factor);
    Stats m_stats; double m_timeSum = 0.0;
};
