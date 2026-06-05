#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class Resampler3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalResamples = 0; int totalSamples = 0; double avgProcessingTimeMs = 0.0; };
    explicit Resampler3(QObject* parent = nullptr);
    void setSourceRate(double sr);
    void setTargetRate(double tr);
    void setQuality(int q);
    QVector<double> process(const QVector<double>& input);
    double ratio() const { return m_target / m_source; }
    double sourceRate() const { return m_source; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void resamplingCompleted(int inLen, int outLen);
private:
    double m_source = 44100.0; double m_target = 48000.0; int m_quality = 5;
    QVector<double> sincInterpolate(const QVector<double>& sig, double ratio);
    QVector<double> designFilter(int len, double cutoff) const;
    Stats m_stats; double m_timeSum = 0.0;
};
