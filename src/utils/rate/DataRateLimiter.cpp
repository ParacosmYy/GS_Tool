/**
 * @file DataRateLimiter.cpp
 * @brief 数据速率限制器实现 — 令牌桶/漏桶/滑动窗口/固定窗口四种限速算法
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/rate/DataRateLimiter.h"

#include <QDateTime>

/** @brief 构造函数，初始化定时器与默认状态 @param parent 父对象 */
DataRateLimiter::DataRateLimiter(QObject *parent)
    : QObject(parent)
    , m_tokens(static_cast<double>(m_burstSize))
    , m_leakyQuota(0.0)
    , m_fixedWindowStart(0)
    , m_fixedWindowUsed(0)
{
    setObjectName(QStringLiteral("DataRateLimiter"));

    m_tickTimer.setInterval(100);
    m_tickTimer.setSingleShot(false);
    connect(&m_tickTimer, &QTimer::timeout,
            this, &DataRateLimiter::onTick);
    m_tickTimer.start();

    m_elapsed.start();
    m_fixedWindowStart = QDateTime::currentMSecsSinceEpoch();
}

/** @brief 析构函数，停止定时器 */
DataRateLimiter::~DataRateLimiter()
{
    m_tickTimer.stop();
}

// ── 算法选择 ──

/** @brief 设置限速算法，同时重置内部状态 @param algo 目标算法 */
void DataRateLimiter::setAlgorithm(Algorithm algo)
{
    m_algorithm = algo;
    /* 切换算法后重置所有内部状态，避免残留数据导致行为异常 */
    m_tokens = static_cast<double>(m_burstSize);
    m_leakyQuota = 0.0;
    m_slidingWindow.clear();
    m_fixedWindowStart = QDateTime::currentMSecsSinceEpoch();
    m_fixedWindowUsed = 0;
}

// ── 速率配置 ──

/** @brief 设置目标速率(Bytes/s) @param bytesPerSecond 每秒允许的字节数 */
void DataRateLimiter::setRate(quint64 bytesPerSecond)
{
    m_rate = qMax<quint64>(1, bytesPerSecond);
}

/** @brief 设置突发大小(桶容量)，自动截断当前令牌 @param maxBurst 最大突发字节数 */
void DataRateLimiter::setBurstSize(quint64 maxBurst)
{
    m_burstSize = qMax<quint64>(1, maxBurst);
    /* 令牌数不能超过新的桶容量 */
    if (m_tokens > static_cast<double>(m_burstSize)) {
        m_tokens = static_cast<double>(m_burstSize);
    }
}

// ── canSend: 检查是否允许发送(不消耗配额) ──

/** @brief 检查限速是否允许发送指定字节数 @param byteCount 目标字节数 @return true 表示允许 */
bool DataRateLimiter::canSend(int byteCount) const
{
    if (byteCount <= 0) return true;

    switch (m_algorithm) {
    case Algorithm::TokenBucket:
        return m_tokens >= byteCount;

    case Algorithm::LeakyBucket: {
        /* 漏桶: 每个100ms tick允许 m_rate/10 字节 */
        double perTick = static_cast<double>(m_rate) / 10.0;
        return byteCount <= static_cast<int>(perTick + m_leakyQuota);
    }

    case Algorithm::SlidingWindow: {
        quint64 used = slidingWindowUsed();
        return (used + static_cast<quint64>(byteCount)) <= m_rate;
    }

    case Algorithm::FixedWindow: {
        quint64 used = fixedWindowUsed();
        return (used + static_cast<quint64>(byteCount)) <= m_rate;
    }
    }
    return false;
}

// ── acquire: 消耗配额并记录统计 ──

/** @brief 尝试获取发送配额(消耗令牌/信用) @param byteCount 目标字节数 @return true 表示成功获取 */
bool DataRateLimiter::acquire(int byteCount)
{
    if (byteCount <= 0) return true;

    int algoIndex = static_cast<int>(m_algorithm);

    switch (m_algorithm) {
    case Algorithm::TokenBucket: {
        if (m_tokens >= byteCount) {
            m_tokens -= byteCount;
            m_stats.totalBytesAllowed += static_cast<quint64>(byteCount);
            return true;
        }
        /* 令牌不足，拒绝 */
        m_stats.totalBytesRejected += static_cast<quint64>(byteCount);
        m_stats.rejectionsByAlgorithm[algoIndex]++;
        emit rateLimited(byteCount);
        return false;
    }

    case Algorithm::LeakyBucket: {
        /* 漏桶: 恒定速率，每tick只允许 m_rate/10 字节 */
        double perTick = static_cast<double>(m_rate) / 10.0;
        double available = perTick + m_leakyQuota;
        if (available >= byteCount) {
            /* 消耗配额: 先用 m_leakyQuota，再用本tick配额 */
            double consumed = byteCount;
            if (m_leakyQuota >= consumed) {
                m_leakyQuota -= consumed;
            } else {
                consumed -= m_leakyQuota;
                m_leakyQuota = 0.0;
                /* perTick 的剩余不保留到下次(漏桶特性: 多余的直接丢弃) */
            }
            m_stats.totalBytesAllowed += static_cast<quint64>(byteCount);
            return true;
        }
        m_stats.totalBytesRejected += static_cast<quint64>(byteCount);
        m_stats.rejectionsByAlgorithm[algoIndex]++;
        emit rateLimited(byteCount);
        return false;
    }

    case Algorithm::SlidingWindow: {
        quint64 used = slidingWindowUsed();
        if ((used + static_cast<quint64>(byteCount)) <= m_rate) {
            qint64 now = QDateTime::currentMSecsSinceEpoch();
            m_slidingWindow.enqueue({now, static_cast<quint64>(byteCount)});
            m_stats.totalBytesAllowed += static_cast<quint64>(byteCount);
            return true;
        }
        m_stats.totalBytesRejected += static_cast<quint64>(byteCount);
        m_stats.rejectionsByAlgorithm[algoIndex]++;
        emit rateLimited(byteCount);
        return false;
    }

    case Algorithm::FixedWindow: {
        /* 检查是否需要切换到下一个窗口 */
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        if ((now - m_fixedWindowStart) >= 1000) {
            m_fixedWindowStart = now;
            m_fixedWindowUsed = 0;
        }
        if ((m_fixedWindowUsed + static_cast<quint64>(byteCount)) <= m_rate) {
            m_fixedWindowUsed += static_cast<quint64>(byteCount);
            m_stats.totalBytesAllowed += static_cast<quint64>(byteCount);
            return true;
        }
        m_stats.totalBytesRejected += static_cast<quint64>(byteCount);
        m_stats.rejectionsByAlgorithm[algoIndex]++;
        emit rateLimited(byteCount);
        return false;
    }
    }
    return false;
}

// ── waitTime: 预估等待时间 ──

/** @brief 计算发送指定字节数前需要等待的时间 @param byteCount 目标字节数 @return 预估等待时间(ms)，0 表示无需等待 */
int DataRateLimiter::waitTime(int byteCount) const
{
    if (byteCount <= 0) return 0;

    switch (m_algorithm) {
    case Algorithm::TokenBucket: {
        if (m_tokens >= byteCount) return 0;
        double deficit = byteCount - m_tokens;
        /* 每100ms补充 m_rate/10 个令牌，等待时间 = deficit / (m_rate/10) * 100 */
        double ratePerTick = static_cast<double>(m_rate) / 10.0;
        if (ratePerTick <= 0) return 0;
        int ms = static_cast<int>(deficit / ratePerTick * 100.0);
        return qMax(ms, 0);
    }

    case Algorithm::LeakyBucket: {
        double perTick = static_cast<double>(m_rate) / 10.0;
        if (perTick <= 0) return 0;
        double available = perTick + m_leakyQuota;
        if (available >= byteCount) return 0;
        double deficit = byteCount - available;
        int ms = static_cast<int>(deficit / perTick * 100.0);
        return qMax(ms, 0);
    }

    case Algorithm::SlidingWindow: {
        quint64 used = slidingWindowUsed();
        quint64 remaining = 0;
        if (used < m_rate) remaining = m_rate - used;
        if (remaining >= static_cast<quint64>(byteCount)) return 0;
        /* 需要等最早的记录过期(1秒窗口滑过) */
        quint64 deficit = static_cast<quint64>(byteCount) - remaining;
        double ratePerMs = static_cast<double>(m_rate) / 1000.0;
        if (ratePerMs <= 0) return 0;
        int ms = static_cast<int>(deficit / ratePerMs);
        return qMax(ms, 0);
    }

    case Algorithm::FixedWindow: {
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        qint64 elapsedInWindow = now - m_fixedWindowStart;
        qint64 remainingMs = 1000 - elapsedInWindow;
        if (remainingMs < 0) remainingMs = 0;

        quint64 used = fixedWindowUsed();
        quint64 remaining = 0;
        if (used < m_rate) remaining = m_rate - used;
        if (remaining >= static_cast<quint64>(byteCount)) return 0;

        /* 需要等当前窗口结束 */
        return static_cast<int>(remainingMs);
    }
    }
    return 0;
}

// ── reset: 重置限速器状态 ──

/** @brief 重置限速器状态(配额/窗口/令牌恢复满)，不影响统计 */
void DataRateLimiter::reset()
{
    m_tokens = static_cast<double>(m_burstSize);
    m_leakyQuota = 0.0;
    m_slidingWindow.clear();
    m_fixedWindowStart = QDateTime::currentMSecsSinceEpoch();
    m_fixedWindowUsed = 0;
}

// ── 统计接口 ──

/** @brief 获取统计快照 @return Stats 结构体 */
DataRateLimiter::Stats DataRateLimiter::stats() const
{
    Stats s = m_stats;
    s.currentRate = m_lastOutputRate;
    return s;
}

/** @brief 重置所有统计计数器(不影响当前配额状态) */
void DataRateLimiter::resetStatistics()
{
    m_stats = Stats{};
    m_statsTotalWaitTimeMs = 0;
    m_lastOutputRate = 0.0;
}

// ── 私有方法 ──

/** @brief 补充令牌桶令牌(每100ms调用一次) */
void DataRateLimiter::refillTokens()
{
    if (m_algorithm == Algorithm::TokenBucket) {
        /* 每100ms补充 m_rate/10 个令牌，上限为 m_burstSize */
        double refill = static_cast<double>(m_rate) / 10.0;
        m_tokens = qMin(m_tokens + refill, static_cast<double>(m_burstSize));
    }
}

/** @brief 更新实时速率统计(基于总输出字节 / 总运行时间) */
void DataRateLimiter::updateRateStats()
{
    double elapsed = m_elapsed.elapsed() / 1000.0;
    if (elapsed > 0.5) {
        double rate = static_cast<double>(m_stats.totalBytesAllowed) / elapsed;
        m_lastOutputRate = rate;
        if (rate > m_stats.peakRate) {
            m_stats.peakRate = rate;
        }
    }
}

/** @brief 获取滑动窗口已用字节数 @return 窗口内已发送字节总数 */
quint64 DataRateLimiter::slidingWindowUsed() const
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    qint64 windowStart = now - 1000;
    quint64 total = 0;
    for (const auto &entry : m_slidingWindow) {
        if (entry.first >= windowStart) {
            total += entry.second;
        }
    }
    return total;
}

/** @brief 获取固定窗口已用字节数 @return 当前窗口内已发送字节总数 */
quint64 DataRateLimiter::fixedWindowUsed() const
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    if ((now - m_fixedWindowStart) >= 1000) {
        return 0; /* 窗口已过期，使用量为0 */
    }
    return m_fixedWindowUsed;
}

// ── 定时器回调 ──

/** @brief 100ms定时器回调: 补充令牌、更新速率、清理过期窗口 */
void DataRateLimiter::onTick()
{
    /* 1. 补充令牌 */
    refillTokens();

    /* 2. 更新速率统计 */
    updateRateStats();

    /* 3. 清理滑动窗口中过期的记录(1秒前) */
    if (m_algorithm == Algorithm::SlidingWindow) {
        qint64 cutoff = QDateTime::currentMSecsSinceEpoch() - 1000;
        while (!m_slidingWindow.isEmpty()
               && m_slidingWindow.front().first < cutoff) {
            m_slidingWindow.dequeue();
        }
    }

    /* 4. 固定窗口自动重置(在onTick中不重置，在acquire中按需重置) */

    /* 5. 计算平均等待时间 */
    if (m_stats.totalWaits > 0) {
        m_stats.avgWaitTimeMs =
            static_cast<double>(m_statsTotalWaitTimeMs)
            / static_cast<double>(m_stats.totalWaits);
    }
}
