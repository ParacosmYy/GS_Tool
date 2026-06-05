#pragma once
#include <QObject>
#include <QVector>
#include <QPair>
class StereoProcessor2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSamplesProcessed = 0; double avgProcessingTimeMs = 0.0; };
    explicit StereoProcessor2(QObject* parent = nullptr);
    void setWidth(double w); void setBalance(double b);
    void setCrossFeed(double cf); void setSampleRate(double rate);
    QPair<double,double> processOne(double left, double right);
    QVector<QPair<double,double>> process(const QVector<QPair<double,double>>& input);
    void reset();
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void processingComplete(int samples);
private:
    double m_width = 1.0; double m_balance = 0.0;
    double m_crossFeed = 0.0; double m_sampleRate = 44100.0;
    Stats m_stats; double m_timeSum = 0.0;
};
