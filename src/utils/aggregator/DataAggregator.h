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

    /** @brief 构造数据聚合器 @param parent 父对象 */
    explicit DataAggregator(QObject *parent = nullptr);
    /** @brief 析构函数 */
    ~DataAggregator() override;

    // ---- 滑动窗口聚合(原有接口) ----

    /** @brief 添加数据源并指定聚合函数和窗口大小 @param name 数据源名称(唯一标识) @param func 聚合函数类型 @param windowSize 滑动窗口大小(样本数)，默认100 */
    void addSource(const QString &name, AggregateFunc func, int windowSize = 100);
    /** @brief 移除指定名称的数据源(滑动窗口+时间窗口一并移除) @param name 数据源名称 */
    void removeSource(const QString &name);
    /** @brief 向指定数据源输入一个新值，触发聚合计算并发射结果 @param source 数据源名称 @param value 输入值 */
    void feedValue(const QString &source, double value);
    /** @brief 获取指定数据源的聚合结果 @param source 数据源名称 @return 聚合结果，不存在返回0.0 */
    double aggregateResult(const QString &source) const;
    /** @brief 获取所有数据源的聚合结果 @return 数据源名称→聚合结果映射 */
    QMap<QString, double> allResults() const;
    /** @brief 获取所有数据源名称列表 @return 名称列表 */
    QStringList sources() const;
    /** @brief 设置指定数据源的滑动窗口大小 @param source 数据源名称 @param size 新的窗口大小 */
    void setWindowSize(const QString &source, int size);
    /** @brief 设置指定数据源的聚合函数 @param source 数据源名称 @param func 新的聚合函数类型 */
    void setAggregateFunc(const QString &source, AggregateFunc func);
    /** @brief 重置指定数据源的缓冲区和结果(滑动窗口+时间窗口一并重置) @param source 数据源名称 */
    void resetSource(const QString &source);
    /** @brief 重置所有数据源的缓冲区和结果 */
    void resetAll();

    // ---- 时间窗口聚合 ----

    /** @brief 为数据源启用时间窗口聚合，窗口按自然时间对齐 @param name 数据源名称 @param intervalSec 窗口间隔(秒) */
    void enableTimeWindow(const QString &name, int intervalSec);
    /** @brief 禁用指定数据源的时间窗口聚合 @param name 数据源名称 */
    void disableTimeWindow(const QString &name);
    /** @brief 获取最近一个已完成时间窗口的统计结果 @param name 数据源名称 @return 窗口统计结构体 */
    WindowStats lastWindowStats(const QString &name) const;
    /** @brief 获取所有已完成时间窗口统计结果(时间升序) @param name 数据源名称 @return 窗口统计列表 */
    QList<WindowStats> windowStatsHistory(const QString &name) const;
    /** @brief 获取当前进行中的时间窗口实时统计 @param name 数据源名称 @return 当前窗口统计结构体 */
    WindowStats currentWindowStats(const QString &name) const;
    /** @brief 设置时间窗口历史记录最大保留数量 @param max 最大保留数量，默认100 */
    void setMaxHistoryWindows(int max);
    /** @brief 获取时间窗口历史记录最大保留数量 @return 最大保留数量 */
    int maxHistoryWindows() const;

    // ---- 实时滚动聚合 ----

    /** @brief 启用实时滚动聚合定时器，定期发射rollingAggregation信号 @param rollIntervalMs 滚动间隔(毫秒)，默认1000 */
    void enableRollingAggregation(int rollIntervalMs = 1000);
    /** @brief 禁用实时滚动聚合定时器 */
    void disableRollingAggregation();
    /** @brief 查询实时滚动聚合是否已启用 @return true=已启用 */
    bool isRollingEnabled() const;

signals:
    /** @brief 数据源聚合完成信号(滑动窗口) @param source 数据源名称 @param result 聚合结果 */
    void valueAggregated(const QString &source, double result);
    /** @brief 数据源添加信号 @param name 数据源名称 */
    void sourceAdded(const QString &name);
    /** @brief 数据源移除信号 @param name 数据源名称 */
    void sourceRemoved(const QString &name);
    /** @brief 时间窗口关闭信号 @param source 数据源名称 @param stats 窗口统计结果 */
    void windowClosed(const QString &source, const WindowStats &stats);
    /** @brief 实时滚动聚合信号 @param results 所有数据源的当前聚合结果 */
    void rollingAggregation(const QMap<QString, double> &results);

private:
    /** @brief 对指定数据源执行滑动窗口聚合计算 @param source 数据源名称 */
    void computeAggregate(const QString &source);
    /** @brief 将新值纳入时间窗口统计 @param source 数据源名称 @param value 新值 @param timestampMs 时间戳(毫秒) */
    void processTimeWindow(const QString &source, double value, qint64 timestampMs);
    /** @brief 计算指定列表值的完整统计 @param values 值列表 @return 窗口统计结构体 */
    static WindowStats computeStats(const QList<double> &values);
    /** @brief 计算指定时间戳所属的时间窗口起始时间 @param timestampMs 时间戳(毫秒) @param intervalSec 窗口间隔(秒) @return 窗口起始时间(毫秒) */
    static qint64 alignToWindow(qint64 timestampMs, int intervalSec);
    /** @brief 定时触发滚动聚合 */
    void onRollingTimeout();

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
    quint64 m_totalResets = 0;             ///< 累计重置次数(resetSource+resetAll)
    quint64 m_totalTimeWindowEnables = 0;  ///< 累计时间窗口启用次数
    quint64 m_totalTimeWindowDisables = 0; ///< 累计时间窗口禁用次数
    quint64 m_totalWindowSlides = 0;       ///< 累计滑动窗口淘汰次数(旧值被移除)
    quint64 m_totalOutputEmissions = 0;    ///< 累计聚合结果发射次数(valueAggregated信号)
    quint64 m_totalComputeCalls = 0;       ///< 累计聚合计算调用次数(computeAggregate)

public:
    /** @brief 获取累计输入值总数 @return 输入值计数 */
    quint64 totalValuesFed() const { return m_totalValuesFed; }
    /** @brief 获取累计完成的时间窗口数 @return 完成窗口计数 */
    quint64 totalWindowsCompleted() const { return m_totalWindowsCompleted; }
    /** @brief 获取累计滚动聚合发射次数 @return 发射计数 */
    quint64 totalRollingEmits() const { return m_totalRollingEmits; }
    /** @brief 获取累计数据源添加次数 @return 添加计数 */
    quint64 totalSourceAdds() const { return m_totalSourceAdds; }
    /** @brief 获取累计数据源移除次数 @return 移除计数 */
    quint64 totalSourceRemoves() const { return m_totalSourceRemoves; }
    /** @brief 获取累计重置次数(resetSource+resetAll) @return 重置计数 */
    quint64 totalResets() const { return m_totalResets; }
    /** @brief 获取累计时间窗口启用次数 @return 启用计数 */
    quint64 totalTimeWindowEnables() const { return m_totalTimeWindowEnables; }
    /** @brief 获取累计时间窗口禁用次数 @return 禁用计数 */
    quint64 totalTimeWindowDisables() const { return m_totalTimeWindowDisables; }
    /** @brief 获取累计滑动窗口淘汰次数(旧值被移除) @return 淘汰计数 */
    quint64 totalWindowSlides() const { return m_totalWindowSlides; }
    /** @brief 获取累计聚合结果发射次数(valueAggregated信号) @return 发射计数 */
    quint64 totalOutputEmissions() const { return m_totalOutputEmissions; }
    /** @brief 获取累计聚合计算调用次数(computeAggregate) @return 计算次数 */
    quint64 totalComputeCalls() const { return m_totalComputeCalls; }
    /** @brief 重置聚合器统计计数器 */
    void resetAggregatorStatistics();
};
