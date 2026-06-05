/**
 * @file CycleDetector.h
 * @brief 周期检测器 — 自动检测数据流中的周期性模式
 *
 * 功能: 自相关法/零交叉法/峰值间距法检测周期，统计周期长度/稳定性/占空比。
 */
#ifndef CYCLEDETECTOR_H
#define CYCLEDETECTOR_H

#include <QObject>
#include <QVector>
#include <QList>

class CycleDetector : public QObject {
    Q_OBJECT
public:
    struct CycleInfo {
        double period = 0.0;        ///< 检测到的周期(采样点数)
        double frequency = 0.0;     ///< 频率(1/period)
        double confidence = 0.0;    ///< 置信度(0-1)
        double dutyCycle = 0.0;     ///< 占空比(0-1)
        int detectedCycles = 0;     ///< 检测到的周期数
    };

    struct Stats {
        quint64 totalDetections = 0;
        double  averagePeriod = 0.0;
        double  peakFrequency = 0.0;
        int     totalCyclesFound = 0;
    };

    explicit CycleDetector(QObject* parent = nullptr);

    void setMinPeriod(int minPeriod);
    void setMaxPeriod(int maxPeriod);
    CycleInfo detect(const QVector<double>& data);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void cycleDetected(double period, double confidence);

private:
    int findFirstPeakAutocorr(const QVector<double>& acf) const;
    QVector<double> computeAutocorrelation(const QVector<double>& data) const;

    int m_minPeriod, m_maxPeriod;
    Stats m_stats;
    double m_periodSum;
};

#endif // CYCLEDETECTOR_H
