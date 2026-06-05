/**
 * @file ArcCache.cpp
 * @brief 自适应替换缓存(ARC)实现 — 平衡频率与最近性
 */

#include "ArcCache.h"

#include <QElapsedTimer>
#include <algorithm>

// ═══════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════

ArcCache::ArcCache(QObject* parent)
    : QObject(parent)
{
}

ArcCache::~ArcCache() = default;

// ═══════════════════════════════════════════════════════════
// 配置
// ═══════════════════════════════════════════════════════════

void ArcCache::setCapacity(int capacity)
{
    m_capacity = qMax(1, capacity);
    m_p = qMin(m_p, m_capacity);
    clear();
}

int ArcCache::capacity() const
{
    return m_capacity;
}

// ═══════════════════════════════════════════════════════════
// 缓存操作
// ═══════════════════════════════════════════════════════════

QVariant ArcCache::get(const QString& key)
{
    QElapsedTimer timer;
    timer.start();

    m_stats.totalGets += 1;

    // 先查T2(频繁使用)
    if (m_t2Map.contains(key)) {
        auto it = m_t2Map[key];
        QVariant value = it->second;
        moveToFront(m_t2, it);
        m_stats.totalHits += 1;
        const double elapsedMs = static_cast<double>(timer.elapsed());
        updateAvgTime(elapsedMs);
        emit cacheHit(key);
        return value;
    }

    // 再查T1(最近使用)
    if (m_t1Map.contains(key)) {
        auto it = m_t1Map[key];
        QVariant value = it->second;
        // 从T1提升到T2
        m_t1Map.remove(key);
        m_t1.erase(it);
        m_t2.emplace_front(key, value);
        m_t2Map[key] = m_t2.begin();
        m_stats.totalHits += 1;
        const double elapsedMs = static_cast<double>(timer.elapsed());
        updateAvgTime(elapsedMs);
        emit cacheHit(key);
        return value;
    }

    m_stats.totalMisses += 1;
    const double elapsedMs = static_cast<double>(timer.elapsed());
    updateAvgTime(elapsedMs);
    emit cacheMiss(key);
    return QVariant();
}

void ArcCache::put(const QString& key, const QVariant& value)
{
    QElapsedTimer timer;
    timer.start();

    m_stats.totalPuts += 1;

    // 已在T2中, 更新值并移到前端
    if (m_t2Map.contains(key)) {
        auto it = m_t2Map[key];
        it->second = value;
        moveToFront(m_t2, it);
        const double elapsedMs = static_cast<double>(timer.elapsed());
        updateAvgTime(elapsedMs);
        return;
    }

    // 已在T1中, 提升到T2
    if (m_t1Map.contains(key)) {
        auto it = m_t1Map[key];
        m_t1Map.remove(key);
        m_t1.erase(it);
        m_t2.emplace_front(key, value);
        m_t2Map[key] = m_t2.begin();
        const double elapsedMs = static_cast<double>(timer.elapsed());
        updateAvgTime(elapsedMs);
        return;
    }

    // 在幽灵列表B1中: 增加p, 替换策略偏向T2
    if (m_b1Set.contains(key)) {
        m_p = qMin(m_p + qMax(1, static_cast<int>(m_b2.size()) /
                              qMax(1, static_cast<int>(m_b1.size()))),
                   m_capacity);
        m_b1Set.remove(key);
        auto it1 = std::find_if(m_b1.begin(), m_b1.end(),
                     [&key](const auto& p) { return p.first == key; });
        if (it1 != m_b1.end()) m_b1.erase(it1);
        arcReplace(key);
    }
    // 在幽灵列表B2中: 减小p, 替换策略偏向T1
    else if (m_b2Set.contains(key)) {
        m_p = qMax(m_p - qMax(1, static_cast<int>(m_b1.size()) /
                              qMax(1, static_cast<int>(m_b2.size()))),
                   0);
        m_b2Set.remove(key);
        auto it2 = std::find_if(m_b2.begin(), m_b2.end(),
                     [&key](const auto& p) { return p.first == key; });
        if (it2 != m_b2.end()) m_b2.erase(it2);
        arcReplace(key);
    }

    /* 缓存满时先淘汰 */
    if (static_cast<int>(m_t1.size() + m_t2.size()) >= m_capacity) {
        arcReplace(key);
    }

    // 插入到T1前端
    m_t1.emplace_front(key, value);
    m_t1Map[key] = m_t1.begin();

    const double elapsedMs = static_cast<double>(timer.elapsed());
    updateAvgTime(elapsedMs);
}

bool ArcCache::contains(const QString& key) const
{
    return m_t1Map.contains(key) || m_t2Map.contains(key);
}

void ArcCache::clear()
{
    m_t1.clear();
    m_t2.clear();
    m_b1.clear();
    m_b2.clear();
    m_t1Map.clear();
    m_t2Map.clear();
    m_b1Set.clear();
    m_b2Set.clear();
    m_p = 0;
}

int ArcCache::size() const
{
    return static_cast<int>(m_t1.size() + m_t2.size());
}

// ═══════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════

ArcCache::Stats ArcCache::stats() const
{
    return m_stats;
}

void ArcCache::resetStatistics()
{
    m_stats = Stats{};
}

// ═══════════════════════════════════════════════════════════
// 内部辅助
// ═══════════════════════════════════════════════════════════

void ArcCache::moveToFront(EntryList& list, EntryIter it)
{
    auto pair = std::move(*it);
    list.erase(it);
    list.emplace_front(std::move(pair));
}

void ArcCache::arcReplace(const QString& key)
{
    Q_UNUSED(key)
    int t1Size = static_cast<int>(m_t1.size());
    /* 当T1非空且(|T1|>p或key在B2且|T1|==p)时从T1淘汰 */
    if (t1Size > 0 && (t1Size > m_p ||
        (m_b2Set.contains(key) && t1Size == m_p))) {
        /* 从T1尾部淘汰到B1 */
        auto last = std::prev(m_t1.end());
        QString evictKey = last->first;
        m_t1Map.remove(evictKey);
        m_b1.emplace_back(std::move(*last));
        m_b1Set[evictKey] = true;
        m_t1.erase(last);
        emit evicted(evictKey);
    } else if (t1Size > 0 || !m_t2.empty()) {
        /* 从T2尾部淘汰到B2 */
        if (!m_t2.empty()) {
            auto last = std::prev(m_t2.end());
            QString evictKey = last->first;
            m_t2Map.remove(evictKey);
            m_b2.emplace_back(std::move(*last));
            m_b2Set[evictKey] = true;
            m_t2.erase(last);
            emit evicted(evictKey);
        }
    }
}

void ArcCache::updateAvgTime(double elapsedMs) const
{
    const auto total = m_stats.totalGets + m_stats.totalPuts;
    if (total <= 1) {
        m_stats.avgProcessingTimeMs = elapsedMs;
    } else {
        m_stats.avgProcessingTimeMs =
            m_stats.avgProcessingTimeMs *
                static_cast<double>(total - 1) / static_cast<double>(total) +
            elapsedMs / static_cast<double>(total);
    }
}
