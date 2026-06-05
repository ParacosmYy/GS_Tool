/**
 * @file WindowedAggregator.h
 * @brief 滑动窗口统计聚合器 -- 基于时间窗口计算均值/中位数/标准差等
 *
 * 维护一个按时间戳排序的滑动窗口，窗口外的过期数据自动淘汰。
 * 每次 pushValue 后重新计算 WindowStats 并发射 windowUpdated 信号。
 * 保留最近 100 次 WindowStats 历史快照，支持导出为 CSV。
 * 适用场景: 实时数据质量监控、滑动窗口异常检测。
 */
#ifndef WINDOWEDAGGREGATOR_H
#define WINDOWEDAGGREGATOR_H

#include <QElapsedTimer>
#include <QList>
#include <QObject>
#include <QPair>
#include <QQueue>
#include <QString>

/**
 * @brief 滑动窗口统计聚合器
 *
 * 窗口大小以毫秒为单位(默认 1000ms)，pushValue 时携带的
 * 值进入一个 QQueue<(timestamp, value)>，超过窗口的旧条目
 * 被自动移除。然后对窗口内所有值计算完整统计。
 */
class WindowedAggregator : public QObject {
    Q_OBJECT

public:
    /** @brief 窗口统计结果 */
    struct WindowStats {
        double mean   = 0.0;    ///< 均值
        double median = 0.0;    ///< 中位数
        double stddev = 0.0;    ///< 标准差
        double min    = 0.0;    ///< 最小值
        double max    = 0.0;    ///< 最大值
        double sum    = 0.0;    ///< 累计和
        int    count  = 0;      ///< 窗口内样本数
    };

    /** @brief 运行时统计信息 */
    struct Stats {
        quint64 totalWindowsComputed  = 0;   ///< 累计计算窗口统计次数
        quint64 totalValuesProcessed  = 0;   ///< 累计处理的值总数
        int     peakWindowSize        = 0;   ///< 历史窗口内最大样本数
        double  avgProcessingTimeUs   = 0.0; ///< 平均每次统计计算耗时(微秒)
    };

    /** @brief 构造滑动窗口聚合器 @param parent 父对象 */
    explicit WindowedAggregator(QObject *parent = nullptr);

    /** @brief 析构函数 */
    ~WindowedAggregator() override;

    // ---- 配置 ----

    /** @brief 设置滑动窗口时长(默认1000ms，最小100ms) @param ms 窗口大小(毫秒) */
    void setWindowSizeMs(qint64 ms);

    /** @brief 获取当前窗口时长 @return 窗口大小(毫秒) */
    qint64 windowSizeMs() const;

    // ---- 数据输入 ----

    /**
     * @brief 向窗口推入一个新值
     *
     * 自动淘汰窗口外的过期条目，然后重新计算统计并发射 windowUpdated。
     * @param value 新数据值
     */
    void pushValue(double value);

    // ---- 查询 ----

    /** @brief 获取当前窗口的统计快照 @return WindowStats 结构体 */
    WindowStats currentStats() const;

    /** @brief 获取当前窗口内的样本数 @return 样本数 */
    int currentCount() const;

    // ---- 重置与导出 ----

    /** @brief 清空窗口缓冲区、历史快照和统计计数器 */
    void reset();

    /**
     * @brief 将历史 WindowStats 导出为 CSV 文件
     *
     * CSV 列: 序号, mean, median, stddev, min, max, sum, count
     * @param path 目标文件路径
     * @return 成功返回 true
     */
    bool exportHistory(const QString &path) const;

    // ---- 统计 ----

    /** @brief 获取运行时统计快照 @return Stats 结构体 */
    Stats stats() const;

    /** @brief 重置运行时统计计数器(不影响窗口数据) */
    void resetStatistics();

signals:
    /** @brief 窗口统计已更新 @param ws 当前窗口统计结果 */
    void windowUpdated(const WindowedAggregator::WindowStats &ws);

    /** @brief 窗口已完全过期(窗口内无数据) */
    void windowExpired();

private:
    /**
     * @brief 淘汰窗口外的过期条目
     * @param nowMs 当前时间戳(ms)
     */
    void evictExpired(qint64 nowMs);

    /**
     * @brief 对给定值列表计算完整统计
     * @param values 值列表(会被排序用于中位数计算)
     * @return WindowStats 结构体
     */
    static WindowStats computeStats(QList<double> values);

    // ── 窗口数据 ──
    QQueue<QPair<qint64, double>> m_window;   ///< 滑动窗口: (时间戳, 值) 队列
    qint64 m_windowSizeMs = 1000;              ///< 窗口大小(ms)
    WindowStats m_currentStats;                ///< 当前窗口统计缓存
    QList<WindowStats> m_history;              ///< 历史窗口统计快照(最近100条)
    int m_maxHistory = 100;                    ///< 历史最大保留条数

    // ── 计时 ──
    QElapsedTimer m_elapsed;                   ///< 运行计时器

    // ── 统计 ──
    Stats m_stats;                             ///< 运行时统计
    double m_processingTimeSum = 0.0;          ///< 计算耗时累加器(us)
};

#endif // WINDOWEDAGGREGATOR_H
