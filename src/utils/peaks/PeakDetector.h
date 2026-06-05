/**
 * @file PeakDetector.h
 * @brief 峰值检测器 — 实时检测数据流中的峰值和谷值
 *
 * 功能: 支持阈值/ prominence/宽度/距离4种峰值选择策略。
 */
#ifndef PEAKDETECTOR_H
#define PEAKDETECTOR_H

#include <QObject>
#include <QVector>
#include <QList>

class PeakDetector : public QObject {
    Q_OBJECT
public:
    struct Peak {
        int index = 0;
        double value = 0.0;
        double prominence = 0.0;
        double width = 0.0;
        bool isMaxima = true;
        bool operator==(const Peak& o) const { return index == o.index; }
    };

    struct Stats {
        quint64 totalPeaksFound = 0;
        quint64 totalValleysFound = 0;
        double  peakPeakAmplitude = 0.0;
        double  averagePeakHeight = 0.0;
    };

    explicit PeakDetector(QObject* parent = nullptr);

    void setMinProminence(double p);
    void setMinDistance(int d);
    QList<Peak> detectPeaks(const QVector<double>& data);
    QList<Peak> detectValleys(const QVector<double>& data);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void peaksFound(int count);

private:
    double m_minProminence;
    int m_minDistance;
    Stats m_stats;
    double m_heightSum;
};

#endif // PEAKDETECTOR_H
