/**
 * @file RmsMeter.h
 * @brief RMS功率计 — 信号有效值/峰值/波峰因子测量
 *
 * 功能: 实时计算信号RMS/峰值/dBFS/波峰因子，
 *       统计测量次数/样本数/耗时。
 */
#ifndef RMSMETER_H
#define RMSMETER_H

#include <QObject>
#include <QVector>

class RmsMeter : public QObject {
    Q_OBJECT
public:
    struct Measurement {
        double rms = 0.0;
        double peak = 0.0;
        double rmsDb = 0.0;
        double peakDb = 0.0;
        double crestFactor = 0.0;
    };

    struct Stats {
        quint64 totalMeasurements = 0;
        quint64 totalSamplesProcessed = 0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit RmsMeter(QObject* parent = nullptr);

    void setReference(double ref);

    Measurement measure(const QVector<double>& data);

    /** @brief 滑动RMS @param data 信号 @param windowSize 窗口大小 @return 逐点RMS */
    QVector<double> slidingRms(const QVector<double>& data, int windowSize);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void measurementCompleted(double rmsDb, double peakDb);

private:
    double m_reference;
    Stats m_stats;
    double m_timeSum;
};

#endif // RMSMETER_H
