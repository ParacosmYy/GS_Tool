/**
 * @file RateOfChange.h
 * @brief 变化率检测器 — 信号变化速度监控
 *
 * 功能: 计算信号的瞬时/平均变化率，支持阈值告警、
 *       方向检测(上升/下降/平稳)，统计检测/告警次数/耗时。
 */
#ifndef RATEOFCHANGE_H
#define RATEOFCHANGE_H

#include <QObject>
#include <QVector>

class RateOfChange : public QObject {
    Q_OBJECT
public:
    enum class Direction { Rising, Falling, Stable };

    /** 统计 */
    struct Stats {
        quint64 totalSamples = 0;
        quint64 totalThresholdBreaches = 0;
        double  avgRate = 0.0;
        double  peakRate = 0.0;
    };

    explicit RateOfChange(double threshold = 1.0, QObject* parent = nullptr);

    /** @brief 处理新值 @param value 新值 @param dt 时间增量 @return 变化率 */
    double update(double value, double dt = 1.0);

    /** @brief 批量处理 @param values 值序列 @param dt 步进 @return 变化率序列 */
    QVector<double> updateBatch(const QVector<double>& values, double dt = 1.0);

    /** @brief 设置变化率阈值 @param threshold 阈值 */
    void setThreshold(double threshold);

    double currentRate() const { return m_lastRate; }
    Direction direction() const;
    double threshold() const { return m_threshold; }

    void reset();
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void rateExceeded(double rate, double threshold);
    void directionChanged(Direction newDirection);

private:
    double m_threshold;
    double m_lastValue;
    double m_lastRate;
    bool m_initialized;
    Direction m_lastDirection;
    Stats m_stats;
};

#endif // RATEOFCHANGE_H
