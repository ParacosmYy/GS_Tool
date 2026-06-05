#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class StereoEnhance2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalProcessings = 0; int totalSamples = 0; double avgProcessingTimeMs = 0.0; };
    explicit StereoEnhance2(QObject* parent = nullptr);
    void setWidth(double w);
    void setPan(double pan);
    void setBassMonoFreq(double freq);
    QVector<QVector<double>> process(const QVector<QVector<double>>& input);
    double correlation() const { return m_corr; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void processingCompleted(int samples, double width);
private:
    double m_width = 1.5; double m_pan = 0.0; double m_bassFreq = 120.0;
    double m_corr = 0.0;
    Stats m_stats; double m_timeSum = 0.0;
};
