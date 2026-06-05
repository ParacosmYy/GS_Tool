/**
 * @file CuckooHashTable.cpp
 * @brief 布谷鸟哈希表实现
 */

#include "utils/cuckoo_hash/CuckooHashTable.h"

#include <QElapsedTimer>

/** @brief 构造函数 @param initialSize 初始表大小 @param parent 父对象 */
CuckooHashTable::CuckooHashTable(int initialSize, QObject* parent)
    : QObject(parent)
    , m_tableSize(qMax(16, initialSize))
    , m_size(0)
    , m_table1(m_tableSize)
    , m_table2(m_tableSize)
{
}

/**
 * @brief 哈希函数1 (MurmurHash3变体)
 * @param key 键
 * @param tableSize 表大小
 * @return 桶索引
 */
quint32 CuckooHashTable::hash1(int key, int tableSize)
{
    quint32 h = static_cast<quint32>(key);
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h % static_cast<quint32>(tableSize);
}

/**
 * @brief 哈希函数2 (不同的哈希常数)
 * @param key 键
 * @param tableSize 表大小
 * @return 桶索引
 */
quint32 CuckooHashTable::hash2(int key, int tableSize)
{
    quint32 h = static_cast<quint32>(key);
    h = (h + 0x9e3779b9) & 0xFFFFFFFF;
    h ^= h >> 16;
    h *= 0x45d9f3b;
    h ^= h >> 16;
    h *= 0x45d9f3b;
    h ^= h >> 16;
    return h % static_cast<quint32>(tableSize);
}

/**
 * @brief 扩容并重哈希所有元素
 *
 * 将表大小翻倍，重新插入所有现有元素。
 */
void CuckooHashTable::rehash()
{
    int oldSize = m_tableSize;
    m_tableSize *= 2;

    /* 收集所有现有条目 */
    QVector<QPair<int, QByteArray>> entries;
    entries.reserve(m_size);
    for (int i = 0; i < oldSize; ++i) {
        if (m_table1[i].occupied) {
            entries.append({m_table1[i].key, m_table1[i].value});
        }
        if (m_table2[i].occupied) {
            entries.append({m_table2[i].key, m_table2[i].value});
        }
    }

    /* 重建表 */
    m_table1 = QVector<Entry>(m_tableSize);
    m_table2 = QVector<Entry>(m_tableSize);
    m_size = 0;

    /* 重新插入所有条目 */
    for (const auto& e : entries) {
        insert(e.first, e.second);
    }

    emit rehashed(m_tableSize);
}

/**
 * @brief 插入键值对
 * @param key 整数键
 * @param value 字节数组值
 *
 * 尝试将键值对放入两个候选位置之一。若都被占用，
 * 踢出已有元素使其重新定位。超过最大踢出次数则扩容重哈希。
 */
void CuckooHashTable::insert(int key, const QByteArray& value)
{
    QElapsedTimer timer;
    timer.start();

    /* 先检查是否已存在(更新) */
    quint32 h1 = hash1(key, m_tableSize);
    quint32 h2 = hash2(key, m_tableSize);

    if (m_table1[h1].occupied && m_table1[h1].key == key) {
        m_table1[h1].value = value;
        m_timeSumMs += timer.nsecsElapsed() / 1e6;
        ++m_stats.totalInserts;
        m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalInserts;
        return;
    }
    if (m_table2[h2].occupied && m_table2[h2].key == key) {
        m_table2[h2].value = value;
        m_timeSumMs += timer.nsecsElapsed() / 1e6;
        ++m_stats.totalInserts;
        m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalInserts;
        return;
    }

    /* 尝试直接插入 */
    if (!m_table1[h1].occupied) {
        m_table1[h1] = {key, value, true};
        ++m_size;
        m_timeSumMs += timer.nsecsElapsed() / 1e6;
        ++m_stats.totalInserts;
        m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalInserts;
        return;
    }
    if (!m_table2[h2].occupied) {
        m_table2[h2] = {key, value, true};
        ++m_size;
        m_timeSumMs += timer.nsecsElapsed() / 1e6;
        ++m_stats.totalInserts;
        m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalInserts;
        return;
    }

    /* 需要踢出: 从table1位置开始 */
    int currentKey = key;
    QByteArray currentValue = value;
    quint32 idx = h1;
    bool useTable1 = true;

    for (int kick = 0; kick < MAX_KICKS; ++kick) {
        Entry& entry = useTable1 ? m_table1[idx] : m_table2[idx];

        /* 踢出当前条目 */
        int evictedKey = entry.key;
        QByteArray evictedValue = entry.value;

        /* 放入新条目 */
        entry = {currentKey, currentValue, true};

        /* 被踢出的元素去备用位置 */
        currentKey = evictedKey;
        currentValue = evictedValue;
        ++m_stats.totalKicks;

        /* 计算被踢出元素的备用位置 */
        if (useTable1) {
            idx = hash2(currentKey, m_tableSize);
            useTable1 = false;
        } else {
            idx = hash1(currentKey, m_tableSize);
            useTable1 = true;
        }

        /* 检查备用位置是否空闲 */
        Entry& target = useTable1 ? m_table1[idx] : m_table2[idx];
        if (!target.occupied) {
            target = {currentKey, currentValue, true};
            ++m_size;
            m_timeSumMs += timer.nsecsElapsed() / 1e6;
            ++m_stats.totalInserts;
            m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalInserts;
            return;
        }
    }

    /* 超过最大踢出次数，扩容重哈希后重试 */
    rehash();
    insert(currentKey, currentValue);
}

/**
 * @brief 查找键对应的值
 * @param key 整数键
 * @return 值(未找到返回空QByteArray)
 *
 * 布谷鸟哈希的查找只需检查两个位置，最坏O(1)。
 */
QByteArray CuckooHashTable::lookup(int key) const
{
    quint32 h1 = hash1(key, m_tableSize);
    quint32 h2 = hash2(key, m_tableSize);

    ++m_stats.totalLookups;

    if (m_table1[h1].occupied && m_table1[h1].key == key) {
        return m_table1[h1].value;
    }
    if (m_table2[h2].occupied && m_table2[h2].key == key) {
        return m_table2[h2].value;
    }

    return QByteArray();
}

/**
 * @brief 删除键
 * @param key 整数键
 */
void CuckooHashTable::remove(int key)
{
    quint32 h1 = hash1(key, m_tableSize);
    quint32 h2 = hash2(key, m_tableSize);

    if (m_table1[h1].occupied && m_table1[h1].key == key) {
        m_table1[h1] = Entry{};
        --m_size;
        return;
    }
    if (m_table2[h2].occupied && m_table2[h2].key == key) {
        m_table2[h2] = Entry{};
        --m_size;
    }
}

/**
 * @brief 计算负载因子
 * @return 负载因子 [0.0, 1.0]
 */
double CuckooHashTable::loadFactor() const
{
    int capacity = m_tableSize * 2;
    return (capacity > 0) ? static_cast<double>(m_size) / capacity : 0.0;
}

/** @brief 重置统计信息 */
void CuckooHashTable::resetStatistics()
{
    m_stats = Stats{};
    m_timeSumMs = 0.0;
}
