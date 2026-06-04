/**
 * @file EventBus.h
 * @brief 全局事件总线 - 模块间松耦合通信的中心调度器
 *
 * 提供发布/订阅模式的全局事件总线，使模块间通过事件通信而非直接方法调用。
 * 支持类型安全的事件订阅、取消订阅和发布。
 *
 * 设计原则:
 *   - 单例模式: 全局唯一事件总线，通过 EventBus::instance() 访问
 *   - 类型安全: 使用模板确保事件类型匹配
 *   - 线程安全: 所有操作通过 Qt 信号槽机制保证线程安全
 *   - 零耦合: 发布者和订阅者互不知道对方存在
 *
 * 使用示例:
 *   // 订阅事件
 *   EventBus::instance().subscribe("connection.changed", this, [this](const QVariant& data) {
 *       auto state = data.value<ConnectionState>();
 *       updateUI(state);
 *   });
 *
 *   // 发布事件
 *   EventBus::instance().publish("connection.changed", QVariant::fromValue(newState));
 *
 * 协作关系:
 *   - ConnectionController: 发布连接状态变化事件
 *   - PanelManager: 订阅连接状态变化，切换面板可用性
 *   - DataStatistics: 订阅数据收发事件，更新统计信息
 *   - ThemeManager: 发布主题切换事件，所有面板订阅刷新
 *
 * 约束:
 *   - 事件名称使用点分层次: "模块.动作" (如 "connection.opened", "data.received")
 *   - 事件数据通过 QVariant 传递，支持任意 Qt 可注册类型
 *   - 订阅回调中禁止发布同类事件（防止递归）
 */
#ifndef EVENTBUS_H
#define EVENTBUS_H

#include <QObject>
#include <QMap>
#include <QMultiMap>
#include <QVariant>
#include <functional>
#include <mutex>

/**
 * @brief 全局事件总线 - 模块间松耦合通信的中心调度器
 *
 * 单例模式，通过 EventBus::instance() 获取全局实例。
 * 所有模块通过此总线发布和订阅事件，实现零耦合通信。
 */
class EventBus : public QObject {
    Q_OBJECT

public:
    /** @brief 事件回调函数类型 */
    using Callback = std::function<void(const QVariant&)>;

    /**
     * @brief 获取全局单例实例
     * @return EventBus 单例引用
     */
    static EventBus& instance();

    /** @brief 析构函数 */
    ~EventBus() override;

    // 禁止拷贝和移动
    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;

    /**
     * @brief 订阅事件
     * @param eventName 事件名称 (点分层次，如 "connection.opened")
     * @param subscriber 订阅者对象指针 (用于自动取消订阅)
     * @param callback 事件回调函数
     * @return 订阅 ID (用于手动取消订阅)
     */
    int subscribe(const QString& eventName, QObject* subscriber, Callback callback);

    /**
     * @brief 取消订阅
     * @param subscriptionId subscribe() 返回的订阅 ID
     */
    void unsubscribe(int subscriptionId);

    /**
     * @brief 取消指定对象的所有订阅
     * @param subscriber 订阅者对象指针
     */
    void unsubscribeAll(QObject* subscriber);

    /**
     * @brief 发布事件 (同步，在当前线程执行所有回调)
     * @param eventName 事件名称
     * @param data 事件数据 (通过 QVariant 传递)
     */
    void publish(const QString& eventName, const QVariant& data = QVariant());

    /**
     * @brief 发布事件 (异步，通过 Qt 事件循环投递)
     * @param eventName 事件名称
     * @param data 事件数据
     */
    void publishAsync(const QString& eventName, const QVariant& data = QVariant());

    /** @brief 获取指定事件的订阅者数量 */
    int subscriberCount(const QString& eventName) const;

    /** @brief 获取所有已注册的事件名称列表 */
    QStringList eventNames() const;

    /** @brief 获取事件总线的统计信息 (用于调试) */
    QMap<QString, int> statistics() const;

    // ---- 统计计数器接口 ----

    /** @brief 获取累计发布的事件总数(同步+异步) @return 事件数 */
    quint64 totalPublished() const;

    /** @brief 获取累计发布的事件总数(同步+异步)别名 @return 事件数 */
    quint64 totalPublishes() const { return m_totalPublished; }

    /** @brief 获取累计异步发布(publishAsync)的事件总数 @return 异步事件数 */
    quint64 totalAsyncPublished() const { return m_totalAsyncPublished; }

    /** @brief 获取累计同步发布(publish)的事件总数(=totalPublished-totalAsyncPublished) @return 同步事件数 */
    quint64 totalSyncPublished() const;

    /** @brief 获取累计订阅操作总数 @return 订阅数 */
    quint64 totalSubscriptions() const;

    /** @brief 获取累计取消订阅操作总数 @return 取消订阅数 */
    quint64 totalUnsubscriptions() const;

    /** @brief 获取累计调用的回调处理器总数 @return 调用次数 */
    quint64 totalHandlersCalled() const;

    /** @brief 获取单个事件的历史峰值订阅者数 @return 峰值订阅者数 */
    quint64 peakSubscribersPerEvent() const;

    /** @brief 重置所有统计计数器(发布/异步发布/订阅/取消订阅/回调/峰值) */
    void resetEventStatistics();

    /** @brief 重置所有统计计数器(resetEventStatistics的简短别名) */
    void resetStats() { resetEventStatistics(); }

signals:
    /**
     * @brief 事件发布信号 (内部使用，支持跨线程异步发布)
     * @param eventName 事件名称
     * @param data 事件数据
     */
    void eventPublished(const QString& eventName, const QVariant& data);

private:
    /** @brief 私有构造函数 (单例模式) */
    explicit EventBus(QObject* parent = nullptr);

    /** @brief 订阅记录 */
    struct Subscription {
        int id;              ///< 唯一订阅 ID
        QString eventName;   ///< 事件名称
        QObject* subscriber; ///< 订阅者对象指针
        Callback callback;   ///< 回调函数
    };

    int m_nextId = 1;                              ///< 下一个订阅 ID
    QMap<int, Subscription> m_subscriptions;       ///< 订阅 ID → 订阅记录
    QMultiMap<QString, int> m_eventSubscriptions;  ///< 事件名称 → 订阅 ID 列表
    mutable std::mutex m_mutex;                     ///< 线程安全互斥锁

    // 统计计数器
    quint64 m_totalPublished = 0;               ///< 累计发布的事件总数(同步+异步)
    quint64 m_totalAsyncPublished = 0;          ///< 累计异步发布(publishAsync)的事件总数
    quint64 m_totalSubscriptions = 0;           ///< 累计订阅操作总数
    quint64 m_totalUnsubscriptions = 0;         ///< 累计取消订阅操作总数
    quint64 m_totalHandlersCalled = 0;          ///< 累计调用的回调处理器总数
    quint64 m_peakSubscribersPerEvent = 0;      ///< 单个事件的历史峰值订阅者数

    /** @brief 处理异步事件发布 */
    void handleAsyncEvent(const QString& eventName, const QVariant& data);
};

#endif // EVENTBUS_H
