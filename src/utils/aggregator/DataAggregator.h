/**
 * @file DataAggregator.h
 * @brief 数据聚合器 — 多源滑动窗口聚合 + 时间窗口聚合
 *
 * 滑动窗口: 基于固定样本数实时计算聚合结果(Sum/Average/Min/Max/Count/First/Last)
 * 时间窗口: 按固定间隔(1s/5s/30s/1min/5min)独立统计min/max/avg/count/sum
 * 两种模式可独立或组合使用。
 */
#pragma once

#include <QObject>
#include <QMap>
#include <QString>
#include <QVariant>
#include <QList>
#include <QTimer>
#include <QElapsedTimer>
#include <QDateTime>

/** @brief 数据聚合器 — 多源滑动窗口聚合 + 时间窗口聚合 */
class DataAggregator : public QObject {
    Q_OBJECT
public:
    enum AggregateFunc { Sum, Average, Min, Max, Count, First, Last }; ///< 滑动窗口聚合函数类型枚举
    Q_ENUM(AggregateFunc)

    enum TimeInterval {
        Interval_1s   = 1,   ///< 1秒
        Interval_5s   = 5,   ///< 5秒
        Interval_30s  = 30,  ///< 30秒
        Interval_1min = 60,  ///< 1分钟
        Interval_5min = 300  ///< 5分钟
    };
    Q_ENUM(TimeInterval)

    /** @brief 时间窗口统计结果结构体 */
    struct WindowStats {
        double min   = 0.0;      ///< 窗口内最小值
        double max   = 0.0;      ///< 窗口内最大值
        double avg   = 0.0;      ///< 窗口内平均值
        double sum   = 0.0;      ///< 窗口内累计和
        int    count = 0;        ///< 窗口内样本数量
        qint64 windowStartMs = 0;///< 窗口起始时间(Epoch毫秒)
        qint64 windowEndMs   = 0;///< 窗口结束时间(Epoch毫秒)
    };

    explicit DataAggregator(QObject *parent = nullptr); ///< 构造数据聚合器
    ~DataAggregator() override; ///< 析构函数

    // ---- 滑动窗口聚合(原有接口) ----

    void addSource(const QString &name, AggregateFunc func, int windowSize = 100); ///< 添加数据源并指定聚合函数和窗口大小
    void removeSource(const QString &name); ///< 移除指定名称的数据源(滑动窗口+时间窗口一并移除)
    void feedValue(const QString &source, double value); ///< 向指定数据源输入一个新值，触发聚合计算并发射结果
    double aggregateResult(const QString &source) const; ///< 获取指定数据源的聚合结果，不存在返回0.0
    QMap<QString, double> allResults() const; ///< 获取所有数据源的聚合结果
    QStringList sources() const;              ///< 获取所有数据源名称列表
    void setWindowSize(const QString &source, int size); ///< 设置指定数据源的滑动窗口大小
    void setAggregateFunc(const QString &source, AggregateFunc func); ///< 设置指定数据源的聚合函数
    void resetSource(const QString &source);  ///< 重置指定数据源的缓冲区和结果(滑动窗口+时间窗口一并重置)
    void resetAll();                          ///< 重置所有数据源的缓冲区和结果

    // ---- 时间窗口聚合 ----

    void enableTimeWindow(const QString &name, int intervalSec); ///< 为数据源启用时间窗口聚合，窗口按自然时间对齐
    void disableTimeWindow(const QString &name); ///< 禁用指定数据源的时间窗口聚合
    WindowStats lastWindowStats(const QString &name) const; ///< 获取最近一个已完成时间窗口的统计结果
    QList<WindowStats> windowStatsHistory(const QString &name) const; ///< 获取所有已完成时间窗口统计结果(时间升序)
    WindowStats currentWindowStats(const QString &name) const; ///< 获取当前进行中的时间窗口实时统计
    void setMaxHistoryWindows(int max); ///< 设置时间窗口历史记录最大保留数量(默认100)
    int maxHistoryWindows() const;      ///< 获取时间窗口历史记录最大保留数量

    // ---- 实时滚动聚合 ----

    void enableRollingAggregation(int rollIntervalMs = 1000); ///< 启用实时滚动聚合定时器，定期发射rollingAggregation信号
    void disableRollingAggregation();  ///< 禁用实时滚动聚合定时器
    bool isRollingEnabled() const;     ///< 查询实时滚动聚合是否已启用

signals:
    void valueAggregated(const QString &source, double result); ///< 数据源聚合完成信号(滑动窗口)
    void sourceAdded(const QString &name);    ///< 数据源添加信号
    void sourceRemoved(const QString &name);  ///< 数据源移除信号
    void windowClosed(const QString &source, const WindowStats &stats); ///< 时间窗口关闭信号
    void rollingAggregation(const QMap<QString, double> &results); ///< 实时滚动聚合信号

private:
    void computeAggregate(const QString &source); ///< 对指定数据源执行滑动窗口聚合计算
    void processTimeWindow(const QString &source, double value, qint64 timestampMs); ///< 将新值纳入时间窗口统计
    static WindowStats computeStats(const QList<double> &values); ///< 计算指定列表值的完整统计
    static qint64 alignToWindow(qint64 timestampMs, int intervalSec); ///< 计算指定时间戳所属的时间窗口起始时间
    void onRollingTimeout(); ///< 定时触发滚动聚合

    /** @brief 滑动窗口数据源配置结构 */
    struct SourceConfig {
        AggregateFunc func;    ///< 聚合函数类型
        int windowSize;        ///< 滑动窗口大小(样本数)
        QList<double> values;  ///< 值缓冲区
        double result = 0.0;   ///< 当前聚合结果
    };

    /** @brief 时间窗口数据结构 */
    struct TimeWindowData {
        int intervalSec = 0;           ///< 窗口间隔(秒)
        qint64 currentWindowStart = 0; ///< 当前窗口起始(Epoch毫秒)
        QList<double> currentValues;   ///< 当前窗口值缓冲
        WindowStats lastCompleted;     ///< 最近已完成窗口统计
        QList<WindowStats> history;    ///< 历史窗口统计(时间升序)
    };

    QMap<QString, SourceConfig> m_sources;       ///< 滑动窗口数据源映射
    QMap<QString, TimeWindowData> m_timeWindows; ///< 时间窗口数据映射
    int m_maxHistoryWindows = 100;                ///< 历史窗口最大保留数量
    QTimer m_rollingTimer;                        ///< 滚动聚合定时器
    bool m_rollingEnabled = false;                ///< 滚动聚合是否启用

    // ---- 统计计数器 ----
    quint64 m_totalValuesFed = 0;          ///< 累计输入值总数
    quint64 m_totalWindowsCompleted = 0;   ///< 累计完成的时间窗口数
    quint64 m_totalRollingEmits = 0;       ///< 累计滚动聚合发射次数
    quint64 m_totalSourceAdds = 0;         ///< 累计数据源添加次数
    quint64 m_totalSourceRemoves = 0;      ///< 累计数据源移除次数

public:
    quint64 totalValuesFed() const { return m_totalValuesFed; }         ///< 累计输入值总数
    quint64 totalWindowsCompleted() const { return m_totalWindowsCompleted; } ///< 累计完成的时间窗口数
    quint64 totalRollingEmits() const { return m_totalRollingEmits; }   ///< 累计滚动聚合发射次数
    quint64 totalSourceAdds() const { return m_totalSourceAdds; }       ///< 累计数据源添加次数
    quint64 totalSourceRemoves() const { return m_totalSourceRemoves; } ///< 累计数据源移除次数
    void resetAggregatorStatistics(); ///< 重置聚合器统计计数器
};
