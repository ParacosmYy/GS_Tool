/**
 * @file ZeroCrossingRate.h
 * @brief 过零率分析器 — 信号频率/周期性特征提取
 *
 * 功能: 计算信号过零率，支持单次/滑动窗口/分段分析，
 *       统计分析次数/过零数/平均过零率。
 */
#ifndef ZEROCROSSINGRATE_H
#define ZEROCROSSINGRATE_H

#include <QObject>
#include <QVector>

class ZeroCrossingRate : public QObject {
    Q_OBJECT
public:
    /** 分析结果 */
    struct Result {
        double rate = 0.0;              ///< 过零率(次/采样)
        int crossingCount = 0;          ///< 过零数
        QVector<int> crossingIndices;   ///< 过零位置
        double estimatedFrequency = 0.0;///< 估算频率(Hz)
    };

    /** 分析统计 */
    struct Stats {
        quint64 totalAnalyses = 0;
        quint64 totalCrossingsDetected = 0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit ZeroCrossingRate(QObject* parent = nullptr);

    /** @brief 计算过零率 @param data 信号 @param sampleRate 采样率 @return 结果 */
    Result compute(const QVector<double>& data, double sampleRate = 1.0);

    /** @brief 滑动窗口过零率 @param data 信号 @param windowSize 窗口大小 @param hopSize 步进 @return 每窗口过零率 */
    QVector<double> slidingWindowRate(const QVector<double>& data,
                                      int windowSize, int hopSize) const;

    /** @brief 正向过零率(仅上升沿) @param data 信号 @return 正向过零率 */
    double positiveRate(const QVector<double>& data) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void analysisCompleted(int crossingCount, double rate);

private:
    mutable Stats m_stats;
    mutable double m_timeSum;
};

#endif // ZEROCROSSINGRATE_H
