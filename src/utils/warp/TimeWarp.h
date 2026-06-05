/**
 * @file TimeWarp.h
 * @brief 时间规整引擎 — DTW(动态时间规整)比较序列相似性
 *
 * 功能: 计算两个时间序列的DTW距离，支持Sakoe-Chiba带约束，
 *       用于模式匹配和手势识别。
 */
#ifndef TIMEWARP_H
#define TIMEWARP_H

#include <QObject>
#include <QVector>
#include <QPair>

class TimeWarp : public QObject {
    Q_OBJECT
public:
    struct DTWResult {
        double distance = 0.0;      ///< DTW距离
        double normalizedDistance = 0.0; ///< 归一化距离
        QVector<QPair<int,int>> warpPath; ///< 规整路径
    };

    struct Stats {
        quint64 totalComparisons = 0;
        double  averageDistance = 0.0;
        double  peakDistance = 0.0;
        double  averagePathLength = 0.0;
    };

    explicit TimeWarp(QObject* parent = nullptr);

    void setBandWidth(int width);
    DTWResult compute(const QVector<double>& seq1, const QVector<double>& seq2);
    static double euclideanDistance(double a, double b);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void comparisonComplete(double distance);

private:
    int m_bandWidth;
    Stats m_stats;
    double m_distSum, m_pathLenSum;
};

#endif // TIMEWARP_H
