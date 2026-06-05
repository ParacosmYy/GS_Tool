#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class StereoProcessor3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalProcessings = 0; int totalSamples = 0; double avgProcessingTimeMs = 0.0; };
    explicit StereoProcessor3(QObject* parent = nullptr);
    void setMidGain(double gain);
    void setSideGain(double gain);
    void setPanLaw(const QString& law);
    QVector<QVector<double>> process(const QVector<QVector<double>>& input);
    double correlation() const { return m_corr; }
    double balance() const { return m_balance; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void processingCompleted(int samples, double balance);
private:
    double m_midGain = 1.0; double m_sideGain = 1.0; QString m_panLaw = "constant_power";
    double m_corr = 0.0; double m_balance = 0.0;
    Stats m_stats; double m_timeSum = 0.0;
};
