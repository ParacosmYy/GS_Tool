/**
 * @file EventBus.h
 * @brief 全局事件总线 - 模块间松耦合通信的中心调度器
 *
 * 发布/订阅模式，单例，类型安全，线程安全(Qt信号槽)，零耦合。
 * 事件名称使用点分层次: "模块.动作" (如 "connection.opened", "data.received")
 * 协作: ConnectionController(发布连接事件) / PanelManager(订阅连接事件) / ThemeManager(发布主题事件)
 * 约束: 订阅回调中禁止发布同类事件（防止递归）
 */
#ifndef EVENTBUS_H
#define EVENTBUS_H

#include <QObject>
#include <QMap>
#include <QMultiMap>
#include <QVariant>
#include <functional>
#include <mutex>
#include <atomic>

/** @brief 全局事件总线 - 模块间松耦合通信的中心调度器(单例) */
class EventBus : public QObject {
    Q_OBJECT

public:
    using Callback = std::function<void(const QVariant&)>; ///< 事件回调函数类型
    static EventBus& instance(); ///< 获取全局单例实例
    ~EventBus() override;
    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;
    /** @brief 订阅事件 @param eventName 事件名称(点分层次) @param subscriber 订阅者对象指针 @param callback 事件回调 @return 订阅ID */
    int subscribe(const QString& eventName, QObject* subscriber, Callback callback);
    /** @brief 取消订阅 @param subscriptionId subscribe()返回的订阅ID */
    void unsubscribe(int subscriptionId);
    /** @brief 取消指定对象的所有订阅 @param subscriber 订阅者对象指针 */
    void unsubscribeAll(QObject* subscriber);
    /** @brief 发布事件(同步) @param eventName 事件名称 @param data 事件数据 */
    void publish(const QString& eventName, const QVariant& data = QVariant());
    /** @brief 发布事件(异步，通过Qt事件循环投递) @param eventName 事件名称 @param data 事件数据 */
    void publishAsync(const QString& eventName, const QVariant& data = QVariant());
    int subscriberCount(const QString& eventName) const;  ///< 获取指定事件的订阅者数量
    QStringList eventNames() const;                       ///< 获取所有已注册的事件名称列表
    QMap<QString, int> statistics() const;                ///< 获取事件总线统计信息(调试用)
    // ---- 统计接口 ----
    quint64 totalPublished() const;                       ///< 获取累计发布事件总数(同步+异步)
    quint64 totalPublishes() const { return m_totalPublished; } ///< totalPublished别名
    quint64 totalAsyncPublished() const { return m_totalAsyncPublished.load(std::memory_order_relaxed); } ///< 获取累计异步发布事件总数
    quint64 totalSyncPublished() const;                   ///< 获取累计同步发布事件总数
    quint64 totalSubscriptions() const;                   ///< 获取累计订阅操作总数
    quint64 totalUnsubscriptions() const;                 ///< 获取累计取消订阅操作总数
    quint64 totalHandlersCalled() const;                  ///< 获取累计回调处理器调用总数
    quint64 peakSubscribersPerEvent() const;              ///< 获取单个事件历史峰值订阅者数
    quint64 totalHandlerErrors() const { return m_totalHandlerErrors.load(std::memory_order_relaxed); } ///< 获取累计回调处理器异常次数
    quint64 totalPublishsWithNoSubscribers() const { return m_totalPublishsWithNoSubscribers; } ///< 获取无订阅者发布次数
    quint64 totalUniqueEventNames() const { return m_totalUniqueEventNames; } ///< 获取独立事件名称总数
    void resetEventStatistics();                          ///< 重置所有统计计数器
    void resetStats() { resetEventStatistics(); }         ///< resetEventStatistics的简短别名

signals:
    /** @brief 事件发布信号(内部使用，支持跨线程异步发布) @param eventName 事件名称 @param data 事件数据 */
    void eventPublished(const QString& eventName, const QVariant& data);

private:
    explicit EventBus(QObject* parent = nullptr); ///< 私有构造函数(单例模式)
    /** @brief 订阅记录 */
    struct Subscription {
        int id;              ///< 唯一订阅ID
        QString eventName;   ///< 事件名称
        QObject* subscriber; ///< 订阅者对象指针
        Callback callback;   ///< 回调函数
    };
    int m_nextId = 1;                              ///< 下一个订阅ID
    QMap<int, Subscription> m_subscriptions;       ///< 订阅ID→订阅记录
    QMultiMap<QString, int> m_eventSubscriptions;  ///< 事件名称→订阅ID列表
    mutable std::mutex m_mutex;                     ///< 线程安全互斥锁
    quint64 m_totalPublished = 0;               ///< 累计发布事件总数(同步+异步)
    std::atomic<quint64> m_totalAsyncPublished{0};  ///< 累计异步发布事件总数(原子操作)
    quint64 m_totalSubscriptions = 0;           ///< 累计订阅操作总数
    quint64 m_totalUnsubscriptions = 0;         ///< 累计取消订阅操作总数
    std::atomic<quint64> m_totalHandlersCalled{0};  ///< 累计回调处理器调用总数(原子操作)
    quint64 m_peakSubscribersPerEvent = 0;      ///< 单个事件历史峰值订阅者数
    std::atomic<quint64> m_totalHandlerErrors{0};   ///< 累计回调处理器执行异常次数(原子操作)
    quint64 m_totalPublishsWithNoSubscribers = 0; ///< 累计发布时无订阅者的事件数
    quint64 m_totalUniqueEventNames = 0;        ///< 历史上去重后的独立事件名称总数
    void handleAsyncEvent(const QString& eventName, const QVariant& data); ///< 处理异步事件发布
};

#endif // EVENTBUS_H
