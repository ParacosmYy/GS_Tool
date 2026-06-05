/**
 * @file DataProfiler.h
 * @brief 数据画像引擎 — 自动分析数据集特征生成统计摘要
 */
#ifndef DATAPROFILER_H
#define DATAPROFILER_H

#include <QObject>
#include <QVector>
#include <QMap>

class DataProfiler : public QObject {
    Q_OBJECT
public:
    struct Profile {
        int count = 0;
        double mean = 0.0;
        double stddev = 0.0;
        double min = 0.0;
        double max = 0.0;
        double median = 0.0;
        double q1 = 0.0;
        double q3 = 0.0;
        double skewness = 0.0;
        double kurtosis = 0.0;
        int zeroCount = 0;
        int nanCount = 0;
        int uniqueValues = 0;
    };

    struct Stats {
        quint64 totalProfiles = 0;
        quint64 totalPointsProfiled = 0;
        double averageProcessingTimeMs = 0.0;
    };

    explicit DataProfiler(QObject* parent = nullptr);
    Profile profile(const QVector<double>& data);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void profileComplete(const Profile& p);

private:
    Stats m_stats;
    double m_timeSum;
};

#endif // DATAPROFILER_H
