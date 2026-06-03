/**
 * @file DataAggregator.h
 * @brief 数据聚合器 — 多源滑动窗口聚合 + 时间窗口聚合
 *
 * 支持两种聚合模式:
 * 1. 滑动窗口聚合(SlidingWindow): 基于固定数量的样本窗口，实时计算聚合结果
 * 2. 时间窗口聚合(TimeWindow): 基于固定时间间隔(1s/5s/30s/1min/5min)，
 *    每个窗口内独立计算min/max/avg/count/sum五项统计指标
 *
 * 两种模式可独立使用，也可组合使用(滑动窗口做实时监控，时间窗口做历史统计)。
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

/**
 * @brief 数据聚合器 — 多源滑动窗口聚合 + 时间窗口聚合
 *
 * 支持多个独立数据源，每个数据源可单独配置聚合函数和窗口大小。
 * 聚合类型包括: 求和/均值/最小值/最大值/计数/首值/末值。
 * 时间窗口聚合支持固定时间间隔内的统计计算，并提供实时滚动聚合功能。
 */
class DataAggregator : public QObject {
    Q_OBJECT
public:
    /** @brief 滑动窗口聚合函数类型枚举 */
    enum AggregateFunc { Sum, Average, Min, Max, Count, First, Last };
    Q_ENUM(AggregateFunc)

    /** @brief 时间窗口聚合间隔枚举 — 预定义常用时间间隔 */
    enum TimeInterval {
        Interval_1s   = 1,      ///< 1秒
        Interval_5s   = 5,      ///< 5秒
        Interval_30s  = 30,     ///< 30秒
        Interval_1min = 60,     ///< 1分钟
        Interval_5min = 300     ///< 5分钟
    };
    Q_ENUM(TimeInterval)

    /**
     * @brief 时间窗口统计结果结构体
     * 包含单个时间窗口内的完整统计信息
     */
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

    /** @brief 添加数据源并指定聚合函数和窗口大小 @param name 数据源名称 @param func 聚合函数类型 @param windowSize 滑动窗口大小 */
    void addSource(const QString &name, AggregateFunc func, int windowSize = 100);
    /** @brief 移除指定名称的数据源(滑动窗口+时间窗口一并移除) @param name 数据源名称 */
    void removeSource(const QString &name);
    /** @brief 向指定数据源输入一个新值，触发聚合计算并发射结果 @param source 数据源名称 @param value 新数据值 */
    void feedValue(const QString &source, double value);
    /** @brief 获取指定数据源的聚合结果 @param source 数据源名称 @return 聚合结果值，不存在返回0.0 */
    double aggregateResult(const QString &source) const;
    /** @brief 获取所有数据源的聚合结果 @return 数据源名称到聚合结果的映射 */
    QMap<QString, double> allResults() const;
    /** @brief 获取所有数据源名称列表 @return 数据源名称列表 */
    QStringList sources() const;
    /** @brief 设置指定数据源的滑动窗口大小 @param source 数据源名称 @param size 新窗口大小 */
    void setWindowSize(const QString &source, int size);
    /** @brief 设置指定数据源的聚合函数 @param source 数据源名称 @param func 聚合函数类型 */
    void setAggregateFunc(const QString &source, AggregateFunc func);
    /** @brief 重置指定数据源的缓冲区和结果(滑动窗口+时间窗口一并重置) @param source 数据源名称 */
    void resetSource(const QString &source);
    /** @brief 重置所有数据源的缓冲区和结果 */
    void resetAll();

    // ---- 时间窗口聚合(新增接口) ----

    /**
     * @brief 为数据源启用时间窗口聚合
     *
     * 启用后，通过feedValue()输入的值会同时参与时间窗口统计。
     * 窗口按自然时间对齐: 以Unix Epoch为起点，按intervalSec秒对齐。
     * 例如intervalSec=60时，窗口边界为每分钟的00秒。
     *
     * @param name 数据源名称(必须已通过addSource添加)
     * @param intervalSec 时间窗口间隔(秒)，推荐使用TimeInterval枚举值
     */
    void enableTimeWindow(const QString &name, int intervalSec);

    /**
     * @brief 禁用指定数据源的时间窗口聚合
     * @param name 数据源名称
     */
    void disableTimeWindow(const QString &name);

    /**
     * @brief 获取指定数据源最近一个已完成时间窗口的统计结果
     * @param name 数据源名称
     * @return 统计结果；若不存在或无已完成窗口，返回count=0的结构体
     */
    WindowStats lastWindowStats(const QString &name) const;

    /**
     * @brief 获取指定数据源所有已完成时间窗口的统计结果列表
     *
     * 返回结果按时间升序排列，每个元素对应一个完整时间窗口。
     * 最多保留最近 maxHistoryWindows() 个窗口的历史记录。
     *
     * @param name 数据源名称
     * @return 统计结果列表(时间升序)
     */
    QList<WindowStats> windowStatsHistory(const QString &name) const;

    /**
     * @brief 获取指定数据源当前正在进行的(尚未关闭的)时间窗口统计
     *
     * 该统计结果是实时更新的，每次feedValue()后都会重新计算。
     *
     * @param name 数据源名称
     * @return 当前窗口的实时统计；若不存在返回count=0的结构体
     */
    WindowStats currentWindowStats(const QString &name) const;

    /**
     * @brief 设置时间窗口历史记录最大保留数量
     *
     * 当历史窗口数量超过此值时，最早的窗口记录会被自动丢弃。
     *
     * @param max 最大保留数量(默认100)
     */
    void setMaxHistoryWindows(int max);

    /**
     * @brief 获取时间窗口历史记录最大保留数量
     * @return 最大保留数量
     */
    int maxHistoryWindows() const;

    // ---- 实时滚动聚合 ----

    /**
     * @brief 启用实时滚动聚合定时器
     *
     * 启用后，每隔rollIntervalMs毫秒自动发射rollingAggregation信号，
     * 携带所有数据源的最新聚合结果。适用于UI实时刷新场景。
     *
     * @param rollIntervalMs 滚动聚合间隔(毫秒)，默认1000ms
     */
    void enableRollingAggregation(int rollIntervalMs = 1000);

    /**
     * @brief 禁用实时滚动聚合定时器
     */
    void disableRollingAggregation();

    /**
     * @brief 查询实时滚动聚合是否已启用
     * @return true表示已启用
     */
    bool isRollingEnabled() const;

signals:
    /** @brief 数据源聚合完成信号(滑动窗口) @param source 数据源名称 @param result 聚合结果 */
    void valueAggregated(const QString &source, double result);
    /** @brief 数据源添加信号 @param name 数据源名称 */
    void sourceAdded(const QString &name);
    /** @brief 数据源移除信号 @param name 数据源名称 */
    void sourceRemoved(const QString &name);

    /** @brief 时间窗口关闭信号 — 当一个时间窗口结束并产生统计结果时发射 @param source 数据源名称 @param stats 窗口统计结果 */
    void windowClosed(const QString &source, const WindowStats &stats);
    /** @brief 实时滚动聚合信号 — 每隔rollIntervalMs毫秒发射一次，携带所有数据源的当前状态 @param results 数据源名称到当前滑动窗口聚合结果的映射 */
    void rollingAggregation(const QMap<QString, double> &results);

private:
    /** @brief 对指定数据源执行滑动窗口聚合计算 @param source 数据源名称 */
    void computeAggregate(const QString &source);
    /** @brief 将一个新值纳入指定数据源的时间窗口统计 @param source 数据源名称 @param value 新数据值 @param timestampMs 当前时间戳(Epoch毫秒) */
    void processTimeWindow(const QString &source, double value, qint64 timestampMs);
    /** @brief 计算指定列表值的完整统计 @param values 值列表 @return 统计结果 */
    static WindowStats computeStats(const QList<double> &values);
    /** @brief 计算指定时间戳所属的时间窗口起始时间 @param timestampMs 时间戳(Epoch毫秒) @param intervalSec 窗口间隔(秒) @return 窗口起始时间(Epoch毫秒) */
    static qint64 alignToWindow(qint64 timestampMs, int intervalSec);
    /** @brief 定时触发滚动聚合 */
    void onRollingTimeout();

    /** @brief 滑动窗口数据源配置结构 */
    struct SourceConfig {
        AggregateFunc func;      ///< 聚合函数类型
        int windowSize;           ///< 滑动窗口大小(样本数)
        QList<double> values;     ///< 值缓冲区
        double result = 0.0;      ///< 当前聚合结果
    };

    /** @brief 时间窗口数据结构 */
    struct TimeWindowData {
        int intervalSec = 0;             ///< 窗口间隔(秒)
        qint64 currentWindowStart = 0;   ///< 当前窗口起始时间(Epoch毫秒)
        QList<double> currentValues;     ///< 当前窗口内的值缓冲
        WindowStats lastCompleted;       ///< 最近一个已完成窗口的统计
        QList<WindowStats> history;      ///< 历史窗口统计列表(时间升序)
    };

    QMap<QString, SourceConfig> m_sources;         ///< 滑动窗口数据源映射
    QMap<QString, TimeWindowData> m_timeWindows;   ///< 时间窗口数据映射
    int m_maxHistoryWindows = 100;                  ///< 历史窗口最大保留数量
    QTimer m_rollingTimer;                          ///< 滚动聚合定时器
    bool m_rollingEnabled = false;                  ///< 滚动聚合是否启用
};
