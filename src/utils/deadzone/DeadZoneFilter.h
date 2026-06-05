/**
 * @file DeadZoneFilter.h
 * @brief 死区滤波器 — 消除信号抖动与微小波动
 *
 * 功能: 可配置死区范围，信号在死区内视为零/保持上一值，
 *       支持对称/非对称死区，统计过滤次数/通过率/耗时。
 */
#ifndef DEADZONEFILTER_H
#define DEADZONEFILTER_H

#include <QObject>

class DeadZoneFilter : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalSamples = 0;
        quint64 totalFiltered = 0;
        quint64 totalPassed = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit DeadZoneFilter(double zoneLow = -0.1, double zoneHigh = 0.1,
                             QObject* parent = nullptr);

    /** @brief 处理单值 @param value 输入 @return 滤波后值 */
    double process(double value);

    /** @brief 批量处理 @param values 输入序列 @return 滤波后序列 */
    QVector<double> processBatch(const QVector<double>& values);

    /** @brief 设置死区范围 @param low 下界 @param high 上界 */
    void setZone(double low, double high);

    double zoneLow() const { return m_zoneLow; }
    double zoneHigh() const { return m_zoneHigh; }
    double lastOutput() const { return m_lastOutput; }

    void reset();
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void sampleFiltered(double input, double output);

private:
    double m_zoneLow;
    double m_zoneHigh;
    double m_lastOutput;
    bool m_initialized;
    Stats m_stats;
    double m_timeSum;
};

#endif // DEADZONEFILTER_H
