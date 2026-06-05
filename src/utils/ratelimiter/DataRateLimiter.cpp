/**
 * @file DataRateLimiter.cpp
 * @brief 数据速率限制器实现 — 令牌桶/漏桶/滑动窗口
 */

#include "utils/ratelimiter/DataRateLimiter.h"

#include <QDateTime>

/** @brief 构造函数 @param parent 父对象 */
DataRateLimiter::DataRateLimiter(QObject* parent)
    : QObject(parent)
    , m_policy(LimitPolicy::TokenBucket)
    , m_targetRate(1024.0)
    , m_bucketCapacity(2048)
    , m_currentTokens(2048.0)
    , m_lastOutputRate(0.0)
{
    m_refillTimer.setInterval(100);
    m_refillTimer.setSingleShot(false);
    connect(&m_refillTimer, &QTimer::timeout,
            this, &DataRateLimiter::onRefillTimer);
    m_refillTimer.start();
    m_elapsedTimer.start();
}

/** @brief 设置限速策略 @param policy 策略 */
void DataRateLimiter::setPolicy(LimitPolicy policy)
{
    m_policy = policy;
    m_currentTokens = static_cast<double>(m_bucketCapacity);
}

/** @brief 设置目标速率 @param bytesPerSec 目标速率(B/s) */
void DataRateLimiter::setTargetRate(double bytesPerSec)
{
    m_targetRate = qMax(1.0, bytesPerSec);
}

/** @brief 设置桶容量 @param capacity 容量(字节) */
void DataRateLimiter::setBucketCapacity(int capacity)
{
    m_bucketCapacity = qMax(1, capacity);
    m_currentTokens = qMin(m_currentTokens, static_cast<double>(m_bucketCapacity));
}

/** @brief 尝试发送数据 @param data 待发送数据 @return 实际发送的数据 */
QByteArray DataRateLimiter::trySend(const QByteArray& data)
{
    int size = data.size();
    m_stats.totalBytesInput += static_cast<quint64>(size);
    ++m_stats.totalPacketsInput;

    /* 计算当前输入速率 */
    double elapsed = m_elapsedTimer.elapsed() / 1000.0;
    if (elapsed > 0) {
        double inputRate = m_stats.totalBytesInput / elapsed;
        if (inputRate > m_stats.peakInputRate) {
            m_stats.peakInputRate = inputRate;
        }
    }

    switch (m_policy) {
    case LimitPolicy::TokenBucket: {
        /* 令牌桶: 允许突发到桶容量 */
        if (m_currentTokens >= size) {
            m_currentTokens -= size;
            m_stats.totalBytesOutput += static_cast<quint64>(size);
            /* 检测突发 */
            if (size > static_cast<int>(m_targetRate * 0.5)) {
                ++m_stats.burstCount;
            }
            return data;
        }
        int allowed = static_cast<int>(m_currentTokens);
        if (allowed > 0) {
            m_currentTokens = 0;
            m_stats.totalBytesOutput += static_cast<quint64>(allowed);
            m_stats.totalBytesDropped += static_cast<quint64>(size - allowed);
            ++m_stats.totalPacketsDropped;
            return data.left(allowed);
        }
        m_stats.totalBytesDropped += static_cast<quint64>(size);
        ++m_stats.totalPacketsDropped;
        emit dataDropped(size);
        return QByteArray();
    }

    case LimitPolicy::LeakyBucket: {
        /* 漏桶: 匀速输出 */
        double allowedPerTick = m_targetRate * 0.1; // 100ms一个tick
        int allowed = static_cast<int>(allowedPerTick);
        if (allowed > size) allowed = size;
        m_stats.totalBytesOutput += static_cast<quint64>(allowed);
        m_stats.totalBytesDropped += static_cast<quint64>(size - allowed);
        if (allowed < size) {
            ++m_stats.totalPacketsDropped;
            emit dataDropped(size - allowed);
        }
        return data.left(allowed);
    }

    case LimitPolicy::SlidingWindow: {
        /* 滑动窗口: 1秒内不超过targetRate字节 */
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        qint64 windowStart = now - 1000;
        while (!m_windowHistory.isEmpty()
               && m_windowHistory.front().first < windowStart) {
            m_windowHistory.dequeue();
        }
        qint64 windowTotal = 0;
        for (const auto& entry : m_windowHistory) {
            windowTotal += entry.second;
        }
        qint64 remaining = static_cast<qint64>(m_targetRate) - windowTotal;
        if (remaining >= size) {
            m_windowHistory.enqueue({now, size});
            m_stats.totalBytesOutput += static_cast<quint64>(size);
            return data;
        }
        int allowed = static_cast<int>(qMax(static_cast<qint64>(0), remaining));
        if (allowed > 0) {
            m_windowHistory.enqueue({now, allowed});
            m_stats.totalBytesOutput += static_cast<quint64>(allowed);
        }
        m_stats.totalBytesDropped += static_cast<quint64>(size - allowed);
        if (allowed < size) {
            ++m_stats.totalPacketsDropped;
            emit dataDropped(size - allowed);
        }
        return data.left(allowed);
    }
    }
    return QByteArray();
}

/** @brief 检查是否可以发送 @param bytes 字节数 @return 是否允许 */
bool DataRateLimiter::canSend(int bytes) const
{
    switch (m_policy) {
    case LimitPolicy::TokenBucket:
        return m_currentTokens >= bytes;
    case LimitPolicy::LeakyBucket:
        return bytes <= static_cast<int>(m_targetRate * 0.1);
    case LimitPolicy::SlidingWindow: {
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        qint64 windowTotal = 0;
        for (const auto& entry : m_windowHistory) {
            if (now - entry.first < 1000) {
                windowTotal += entry.second;
            }
        }
        return (windowTotal + bytes) <= static_cast<qint64>(m_targetRate);
    }
    }
    return false;
}

/** @brief 获取当前可用配额 @return 可用字节数 */
double DataRateLimiter::availableQuota() const
{
    switch (m_policy) {
    case LimitPolicy::TokenBucket:
        return m_currentTokens;
    case LimitPolicy::LeakyBucket:
        return m_targetRate * 0.1;
    case LimitPolicy::SlidingWindow: {
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        qint64 windowTotal = 0;
        for (const auto& entry : m_windowHistory) {
            if (now - entry.first < 1000) {
                windowTotal += entry.second;
            }
        }
        return qMax(0.0, m_targetRate - static_cast<double>(windowTotal));
    }
    }
    return 0.0;
}

/** @brief 获取当前输出速率 @return B/s */
double DataRateLimiter::currentOutputRate() const
{
    return m_lastOutputRate;
}

/** @brief 令牌补充定时器回调(100ms) */
void DataRateLimiter::onRefillTimer()
{
    refillTokens();

    /* 计算输出速率 */
    double elapsed = m_elapsedTimer.elapsed() / 1000.0;
    if (elapsed > 0.5) {
        m_lastOutputRate = m_stats.totalBytesOutput / elapsed;
        if (m_lastOutputRate > m_stats.peakOutputRate) {
            m_stats.peakOutputRate = m_lastOutputRate;
        }
    }

    /* 速率超限告警 */
    if (m_lastOutputRate > m_targetRate * 1.1) {
        emit rateExceeded(m_lastOutputRate, m_targetRate);
    }
}

/** @brief 补充令牌 */
void DataRateLimiter::refillTokens()
{
    if (m_policy == LimitPolicy::TokenBucket) {
        /* 每100ms补充 targetRate/10 个令牌 */
        double refill = m_targetRate / 10.0;
        m_currentTokens = qMin(m_currentTokens + refill,
                               static_cast<double>(m_bucketCapacity));
        ++m_stats.totalTokenRefills;
    }
}

/** @brief 重置所有统计计数器 */
void DataRateLimiter::resetStatistics()
{
    m_stats = Stats{};
    m_currentTokens = static_cast<double>(m_bucketCapacity);
    m_lastOutputRate = 0.0;
    m_windowHistory.clear();
}
