/**
 * @file EventTimeline.h
 * @brief 事件时间线 — 记录调试会话中的时间戳事件
 *
 * 功能: 支持分类/严重级别的事件记录，时间范围查询，
 *       JSON导出，邻近事件搜索。
 *
 * 协作: DataQualityScorer(质量事件) / ConnectionHealthMonitor(连接事件)
 */
#ifndef EVENTTIMELINE_H
#define EVENTTIMELINE_H

#include <QObject>
#include <QList>
#include <QString>
#include <QMap>
#include <QDateTime>

/**
 * @brief 事件时间线 — 时间戳事件记录与查询
 */
class EventTimeline : public QObject {
    Q_OBJECT

public:
    /** @brief 事件类别 */
    enum class Category {
        System,     ///< 系统事件
        Connection, ///< 连接事件
        Data,       ///< 数据事件
        Protocol,   ///< 协议事件
        Error,      ///< 错误事件
        Custom      ///< 自定义事件
    };
    Q_ENUM(Category)

    /** @brief 严重级别 */
    enum class Severity {
        Debug,      ///< 调试
        Info,       ///< 信息
        Warning,    ///< 警告
        Error,      ///< 错误
        Critical    ///< 严重
    };
    Q_ENUM(Severity)

    /** @brief 事件记录 */
    struct Event {
        qint64 timestamp = 0;       ///< 时间戳(ms)
        Category category;          ///< 类别
        Severity severity;          ///< 严重级别
        QString message;            ///< 事件消息
        QMap<QString, QString> metadata; ///< 附加元数据
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalEvents = 0;            ///< 累计事件数
        QMap<int, quint64> eventsByCategory;///< 各类别计数
        QMap<int, quint64> eventsBySeverity;///< 各级别计数
        double  peakEventsPerSecond = 0.0;  ///< 峰值每秒事件数
    };

    explicit EventTimeline(QObject* parent = nullptr);

    /** @brief 记录事件 @param category 类别 @param severity 级别 @param message 消息 */
    void record(Category category, Severity severity, const QString& message);

    /** @brief 记录带元数据的事件 @param category 类别 @param severity 级别 @param message 消息 @param metadata 元数据 */
    void recordWithMetadata(Category category, Severity severity,
                            const QString& message,
                            const QMap<QString, QString>& metadata);

    /** @brief 按时间范围查询 @param fromMs 起始 @param toMs 结束 @return 事件列表 */
    QList<Event> queryByTime(qint64 fromMs, qint64 toMs) const;

    /** @brief 按类别查询 @param category 类别 @return 事件列表 */
    QList<Event> queryByCategory(Category category) const;

    /** @brief 按严重级别查询 @param severity 级别 @return 事件列表 */
    QList<Event> queryBySeverity(Severity severity) const;

    /** @brief 查找时间戳附近的事件 @param ts 时间戳 @param maxDelta 最大偏差 @return 事件列表 */
    QList<Event> findNear(qint64 ts, qint64 maxDelta) const;

    /** @brief 导出为JSON @param path 文件路径 @return 是否成功 */
    bool exportToJson(const QString& path) const;

    /** @brief 清除所有事件 */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 事件已记录 @param event 事件 */
    void eventRecorded(const Event& event);

    /** @brief Critical事件告警 @param event 事件 */
    void criticalEvent(const Event& event);

private:
    QList<Event> m_events;          ///< 事件列表
    int m_maxEvents;                ///< 最大保留事件数

    Stats m_stats;
    QMap<qint64, int> m_eventsPerSecond; ///< 每秒事件计数
};

#endif // EVENTTIMELINE_H
