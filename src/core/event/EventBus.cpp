/**
 * @file EventBus.cpp
 * @brief 全局事件总线实现 - 模块间松耦合通信的中心调度器
 */

#include "core/event/EventBus.h"

#include <QCoreApplication>
#include <QMetaObject>
#include <QtGlobal>

// ---- 单例实现 ----

EventBus& EventBus::instance()
{
    static EventBus bus;
    return bus;
}

EventBus::EventBus(QObject* parent)
    : QObject(parent)
{
    // 连接内部信号用于异步发布
    connect(this, &EventBus::eventPublished,
            this, &EventBus::handleAsyncEvent,
            Qt::QueuedConnection);
}

EventBus::~EventBus() = default;

// ---- 订阅管理 ----

int EventBus::subscribe(const QString& eventName, QObject* subscriber, Callback callback)
{
    std::lock_guard<std::mutex> lock(m_mutex);

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

void EventBus::publish(const QString& eventName, const QVariant& data)
{
    // 收集回调（在锁内复制，锁外执行）
    QList<Callback> callbacks;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
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
    }
}

void EventBus::publishAsync(const QString& eventName, const QVariant& data)
{
    emit eventPublished(eventName, data);
}

void EventBus::handleAsyncEvent(const QString& eventName, const QVariant& data)
{
    publish(eventName, data);
}

// ---- 统计与调试 ----

int EventBus::subscriberCount(const QString& eventName) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_eventSubscriptions.count(eventName);
}

QStringList EventBus::eventNames() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    QStringList names;
    for (auto it = m_eventSubscriptions.constBegin(); it != m_eventSubscriptions.constEnd(); ++it) {
        if (!names.contains(it.key())) {
            names.append(it.key());
        }
    }
    return names;
}

QMap<QString, int> EventBus::statistics() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    QMap<QString, int> stats;
    for (auto it = m_eventSubscriptions.constBegin(); it != m_eventSubscriptions.constEnd(); ++it) {
        stats[it.key()]++;
    }
    return stats;
}
