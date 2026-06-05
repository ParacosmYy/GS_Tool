/**
 * @file ProcessMonitor.h
 * @brief 多进程指标监控器 — 系统级资源跟踪
 *
 * 功能: 跟踪多进程的CPU/内存/IO指标，支持阈值告警、
 *       历史记录、聚合统计，统计采样/告警次数。
 */
#ifndef PROCESSMONITOR_H
#define PROCESSMONITOR_H

#include <QObject>
#include <QMap>
#include <QVector>
#include <QElapsedTimer>

class ProcessMonitor : public QObject {
    Q_OBJECT
public:
    /** 进程指标 */
    struct Metrics {
        double cpuUsage = 0.0;      ///< CPU使用率(%)
        double memoryUsage = 0.0;   ///< 内存使用(MB)
        double diskReadKBps = 0.0;  ///< 磁盘读取(KB/s)
        double diskWriteKBps = 0.0; ///< 磁盘写入(KB/s)
        qint64 timestampMs = 0;     ///< 时间戳
    };

    /** 统计 */
    struct Stats {
        quint64 totalSamples = 0;
        quint64 totalAlerts = 0;
        quint64 totalProcesses = 0;
    };

    explicit ProcessMonitor(QObject* parent = nullptr);

    /** @brief 注册进程 @param pid 进程ID @param name 进程名 */
    void registerProcess(int pid, const QString& name);

    /** @brief 注销进程 @param pid 进程ID */
    void unregisterProcess(int pid);

    /** @brief 更新进程指标 @param pid 进程ID @param metrics 指标 */
    void updateMetrics(int pid, const Metrics& metrics);

    /** @brief 设置CPU告警阈值 @param threshold 百分比 */
    void setCpuAlertThreshold(double threshold);

    /** @brief 获取进程历史 @param pid 进程ID @param maxSamples 最大采样数 @return 指标历史 */
    QVector<Metrics> getHistory(int pid, int maxSamples = 100) const;

    /** @brief 获取所有进程当前指标 @return pid→Metrics */
    QMap<int, Metrics> currentSnapshot() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void metricsUpdated(int pid, const Metrics& metrics);
    void cpuAlert(int pid, double usage, double threshold);

private:
    struct ProcessInfo {
        QString name;
        Metrics current;
        QVector<Metrics> history;
    };

    QMap<int, ProcessInfo> m_processes;
    double m_cpuThreshold;
    int m_maxHistory;
    Stats m_stats;
};

#endif // PROCESSMONITOR_H
