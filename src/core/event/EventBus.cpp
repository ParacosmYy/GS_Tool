/**
 * @file EventBus.cpp
 * @brief 全局事件总线实现 - 模块间松耦合通信的中心调度器
 */

#include "core/event/EventBus.h"

#include <QCoreApplication>
#include <QMetaObject>
#include <QtGlobal>

// ---- 单例实现 ----

/** @brief 获取EventBus单例实例 @return 单例引用 */
EventBus& EventBus::instance()
{
    static EventBus bus;
    return bus;
}

/** @brief 构造事件总线，连接内部异步发布信号 @param parent 父对象指针 */
EventBus::EventBus(QObject* parent)
    : QObject(parent)
{
    // 连接内部信号用于异步发布
    connect(this, &EventBus::eventPublished,
            this, &EventBus::handleAsyncEvent,
            Qt::QueuedConnection);
}

/** @brief 析构函数，使用默认实现 */
EventBus::~EventBus() = default;

// ---- 订阅管理 ----

/**
 * @brief 订阅指定事件，注册回调处理器
 * @param eventName 事件名称
 * @param subscriber 订阅者对象指针，销毁时自动取消订阅
 * @param callback 事件触发时的回调函数
 * @return 订阅ID，用于后续取消订阅
 */
int EventBus::subscribe(const QString& eventName, QObject* subscriber, Callback callback)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    ++m_totalSubscriptions;  // 累计订阅计数
    const int id = m_nextId++;
    Subscription sub{id, eventName, subscriber, std::move(callback)};
    m_subscriptions.insert(id, sub);
    m_eventSubscriptions.insert(eventName, id);

    // 订阅者销毁时自动取消订阅
    if (subscriber) {
        QObject::connect(subscriber, &QObject::destroyed, this, [this, subscriber]() {
            unsubscribeAll(subscriber);
        }, Qt::UniqueConnection);
    }

    return id;
}

/**
 * @brief 取消指定ID的订阅
 * @param subscriptionId 要取消的订阅ID
 */
void EventBus::unsubscribe(int subscriptionId)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_subscriptions.find(subscriptionId);
    if (it == m_subscriptions.end()) return;

    const QString eventName = it->eventName;
    m_subscriptions.erase(it);

    // 从事件→ID 映射中移除
    auto range = m_eventSubscriptions.equal_range(eventName);
    for (auto eit = range.first; eit != range.second; ++eit) {
        if (*eit == subscriptionId) {
            m_eventSubscriptions.erase(eit);
            break;
        }
    }
}

/**
 * @brief 取消指定订阅者的所有订阅
 * @param subscriber 要清除订阅的对象指针
 */
void EventBus::unsubscribeAll(QObject* subscriber)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    // 收集该订阅者的所有订阅 ID
    QList<int> ids;
    for (auto it = m_subscriptions.begin(); it != m_subscriptions.end(); ++it) {
        if (it->subscriber == subscriber) {
            ids.append(it->id);
        }
    }

    // 批量移除
    for (int id : ids) {
        auto it = m_subscriptions.find(id);
        if (it != m_subscriptions.end()) {
            const QString eventName = it->eventName;
            m_subscriptions.erase(it);

            auto range = m_eventSubscriptions.equal_range(eventName);
            for (auto eit = range.first; eit != range.second; ++eit) {
                if (*eit == id) {
                    m_eventSubscriptions.erase(eit);
                    break;
                }
            }
        }
    }
}

// ---- 事件发布 ----

/**
 * @brief 同步发布事件，立即执行所有订阅者回调
 * @param eventName 事件名称
 * @param data 事件携带的数据
 */
void EventBus::publish(const QString& eventName, const QVariant& data)
{
    // 收集回调（在锁内复制，锁外执行）
    QList<Callback> callbacks;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        ++m_totalPublished;  // 累计发布计数
        auto range = m_eventSubscriptions.equal_range(eventName);
        for (auto it = range.first; it != range.second; ++it) {
            auto subIt = m_subscriptions.find(*it);
            if (subIt != m_subscriptions.end()) {
                callbacks.append(subIt->callback);
            }
        }
    }

    // 执行所有回调（锁外执行，避免死锁）
    for (const auto& cb : callbacks) {
        cb(data);
        ++m_totalHandlersCalled;  // 累计回调调用计数
    }
}

/**
 * @brief 异步发布事件，通过信号队列在事件循环中执行
 * @param eventName 事件名称
 * @param data 事件携带的数据
 */
void EventBus::publishAsync(const QString& eventName, const QVariant& data)
{
    emit eventPublished(eventName, data);
}

/**
 * @brief 异步事件内部处理槽，委托给同步publish执行
 * @param eventName 事件名称
 * @param data 事件携带的数据
 */
void EventBus::handleAsyncEvent(const QString& eventName, const QVariant& data)
{
    publish(eventName, data);
}

// subscriberCount/eventNames/statistics/totalPublished/totalSubscriptions/totalHandlersCalled/resetEventStatistics
// 已移至 EventBusStats.cpp
