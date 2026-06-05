/**
 * @file ArcCache2.cpp
 * @brief ArcCache2 实现 — 自适应替换缓存(ARC)
 *
 * ARC算法说明:
 * - T1: 存放最近使用一次的条目(类似LRU)
 * - T2: 存放频繁使用的条目(类似LFU)
 * - B1: T1淘汰后的幽灵条目(仅记录key, 不存值)
 * - B2: T2淘汰后的幽灵条目(仅记录key, 不存值)
 * - p: T1的目标大小, 由B1/B2的命中情况自适应调整
 *
 * 自适应机制:
 * - B1命中(新条目在B1历史中) → 增大p(T1分更多空间)
 * - B2命中(新条目在B2历史中) → 减小p(T2分更多空间)
 */

#include "utils/cache2/ArcCache2.h"

#include <algorithm>

// ── 构造 / 析构 ──

/**
 * @brief 构造函数, 初始化四个链表和自适应参数
 * @param capacity 缓存总容量
 * @param parent   父QObject
 */
ArcCache2::ArcCache2(int capacity, QObject* parent)
    : QObject(parent)
    , m_capacity(qMax(1, capacity))
    , m_p(0)
{
    setObjectName(QStringLiteral("ArcCache2"));
    m_timer.start();
}

ArcCache2::~ArcCache2() = default;

// ── 核心操作 ──

/**
 * @brief 读取缓存值
 *
 * 若key在T1中, 提升到T2尾部并返回值。
 * 若key在T2中, 移到T2尾部并返回值。
 * 否则返回无效QVariant。
 * @param key 缓存键
 * @return 缓存值或无效QVariant
 */
QVariant ArcCache2::get(const QString& key)
{
    m_timer.restart();

    QVariant result;

    /* 在T1中找到 → 提升到T2 */
    if (m_t1Data.contains(key)) {
        result = m_t1Data.take(key);
        m_t1.removeOne(key);
        m_t2.append(key);
        m_t2Data[key] = result;
        ++m_stats.totalHits;
    }
    /* 在T2中找到 → 移到T2尾部 */
    else if (m_t2Data.contains(key)) {
        m_t2.removeOne(key);
        m_t2.append(key);
        result = m_t2Data[key];
        ++m_stats.totalHits;
    }

    /* 更新统计 */
    ++m_stats.totalGets;
    double elapsed = static_cast<double>(m_timer.nsecsElapsed()) / 1e6;
    m_stats.avgProcessingTimeMs =
        (m_stats.avgProcessingTimeMs * static_cast<double>(m_stats.totalGets - 1)
         + elapsed) / static_cast<double>(m_stats.totalGets);

    return result;
}

/**
 * @brief 写入缓存条目
 *
 * 四种情况:
 * 1. key已在T1 → 更新值, 提升到T2
 * 2. key已在T2 → 更新值, 移到T2尾部
 * 3. key在B1(幽灵) → 增大p, replace(), 插入T2
 * 4. key在B2(幽灵) → 减小p, replace(), 插入T2
 * 5. 全新key → 若T1+T1已满则replace(), 插入T1
 * @param key   缓存键
 * @param value 缓存值
 */
void ArcCache2::put(const QString& key, const QVariant& value)
{
    m_timer.restart();

    /* 情况1: key在T1 → 更新并提升到T2 */
    if (m_t1Data.contains(key)) {
        m_t1Data.remove(key);
        m_t1.removeOne(key);
        m_t2Data[key] = value;
        m_t2.append(key);
        updateStats();
        return;
    }

    /* 情况2: key在T2 → 更新并移到T2尾部 */
    if (m_t2Data.contains(key)) {
        m_t2.removeOne(key);
        m_t2.append(key);
        m_t2Data[key] = value;
        updateStats();
        return;
    }

    /* 情况3: key在B1(幽灵) → 自适应增大p */
    if (m_b1.contains(key)) {
        /* delta1 = max(|B1|/|B2|, 1) */
        int delta1 = qMax(1, m_b2.size() / qMax(1, m_b1.size()));
        m_p = qMin(m_p + delta1, m_capacity);

        m_b1.removeOne(key);

        /* 执行替换策略 */
        replace(false);

        /* 插入T2(从B1提升的条目说明它被再次使用, 视为频繁) */
        m_t2Data[key] = value;
        m_t2.append(key);
        updateStats();
        return;
    }

    /* 情况4: key在B2(幽灵) → 自适应减小p */
    if (m_b2.contains(key)) {
        /* delta2 = max(|B2|/|B1|, 1) */
        int delta2 = qMax(1, m_b1.size() / qMax(1, m_b2.size()));
        m_p = qMax(m_p - delta2, 0);

        m_b2.removeOne(key);

        /* 执行替换策略 */
        replace(true);

        /* 插入T2 */
        m_t2Data[key] = value;
        m_t2.append(key);
        updateStats();
        return;
    }

    /* 情况5: 全新条目 */
    int totalSize = m_t1.size() + m_t2.size();
    if (totalSize >= m_capacity) {
        emit cacheFull();
        replace(false);
    }

    /* 限制B1+B2总大小为2*capacity */
    int ghostTotal = m_b1.size() + m_b2.size();
    int ghostLimit = 2 * m_capacity;
    while (ghostTotal >= ghostLimit) {
        if (!m_b1.isEmpty()) {
            m_b1.removeFirst();
        } else {
            m_b2.removeFirst();
        }
        --ghostTotal;
    }

    /* 插入T1尾部 */
    m_t1Data[key] = value;
    m_t1.append(key);
    updateStats();
}

/**
 * @brief 判断键是否存在于缓存中
 * @param key 缓存键
 * @return true = 存在于T1或T2
 */
bool ArcCache2::contains(const QString& key) const
{
    return m_t1Data.contains(key) || m_t2Data.contains(key);
}

/**
 * @brief 移除指定键
 *
 * 从T1/T2/B1/B2中查找并移除。
 * @param key 缓存键
 * @return true = 成功移除
 */
bool ArcCache2::remove(const QString& key)
{
    if (m_t1Data.contains(key)) {
        m_t1Data.remove(key);
        m_t1.removeOne(key);
        return true;
    }
    if (m_t2Data.contains(key)) {
        m_t2Data.remove(key);
        m_t2.removeOne(key);
        return true;
    }
    if (m_b1.contains(key)) {
        m_b1.removeOne(key);
        return true;
    }
    if (m_b2.contains(key)) {
        m_b2.removeOne(key);
        return true;
    }
    return false;
}

/** @brief 清空所有缓存(包括幽灵链表) */
void ArcCache2::clear()
{
    m_t1Data.clear();
    m_t2Data.clear();
    m_t1.clear();
    m_t2.clear();
    m_b1.clear();
    m_b2.clear();
    m_p = 0;
}

/** @brief 当前有效条目数(T1+T2) */
int ArcCache2::size() const
{
    return m_t1.size() + m_t2.size();
}

// ── 统计 ──

/**
 * @brief 获取当前统计数据快照
 * @return Stats结构体副本
 */
ArcCache2::Stats ArcCache2::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计计数器
 */
void ArcCache2::resetStatistics()
{
    m_stats = Stats{};
}

// ── 私有方法 ──

/**
 * @brief ARC替换策略
 *
 * 当T1+T2已满时, 根据|m_t1|与p的关系决定淘汰T1还是T2的LRU条目:
 * - |T1| > p: 淘汰T1头部, 移入B1
 * - |T1| <= p: 淘汰T2头部, 移入B2
 * @param inB2 当前插入是否来自B2幽灵(影响B2大小计算)
 */
void ArcCache2::replace(bool inB2)
{
    int t1Size = m_t1.size();
    int b2Adjust = inB2 ? 1 : 0;

    if (t1Size > 0 && ((t1Size > m_p) || (t1Size == m_p && inB2))) {
        /* 淘汰T1的LRU(头部) → 移入B1 */
        QString victim = m_t1.takeFirst();
        m_t1Data.remove(victim);
        m_b1.append(victim);
        emit entryEvicted(victim);
    } else if (m_t2.size() > 0) {
        /* 淘汰T2的LRU(头部) → 移入B2 */
        QString victim = m_t2.takeFirst();
        m_t2Data.remove(victim);
        m_b2.append(victim);
        emit entryEvicted(victim);
    }
}

/**
 * @brief 更新put操作的统计
 */
void ArcCache2::updateStats()
{
    ++m_stats.totalPuts;
    double elapsed = static_cast<double>(m_timer.nsecsElapsed()) / 1e6;
    quint64 n = m_stats.totalGets + m_stats.totalPuts;
    if (n > 0) {
        m_stats.avgProcessingTimeMs =
            (m_stats.avgProcessingTimeMs * static_cast<double>(n - 1)
             + elapsed) / static_cast<double>(n);
    }
}
