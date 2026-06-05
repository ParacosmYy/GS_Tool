/**
 * @file TinyLfuCache.cpp
 * @brief TinyLFU缓存实现
 */

#include "utils/lfu2/TinyLfuCache.h"

#include <QElapsedTimer>

TinyLfuCache::TinyLfuCache(int capacity, double windowRatio, QObject* parent)
    : QObject(parent), m_capacity(qMax(10, capacity)),
      m_windowCapacity(qMax(1, static_cast<int>(capacity * windowRatio))),
      m_mainCapacity(m_capacity - m_windowCapacity),
      m_minFreq(0), m_totalAccesses(0),
      m_windowSize(0), m_mainSize(0), m_timeSum(0.0)
{
}

bool TinyLfuCache::access(int key)
{
    QElapsedTimer timer;
    timer.start();

    updateFrequency(key);
    m_stats.totalAccesses++;
    m_totalAccesses++;

    bool hit = m_window.contains(key) || m_main.contains(key);

    if (hit) {
        m_stats.totalHits++;
        /* 如果在窗口缓存中，更新LRU位置 */
        if (m_window.contains(key)) {
            m_windowLru.removeOne(key);
            m_windowLru.append(key);
        }
        /* 如果在主缓存中，更新频率链表 */
        if (m_main.contains(key)) {
            int oldFreq = m_main[key].freq;
            m_freqList[oldFreq].removeOne(key);
            if (m_freqList[oldFreq].isEmpty()) {
                m_freqList.remove(oldFreq);
                if (oldFreq == m_minFreq) ++m_minFreq;
            }
            m_main[key].freq++;
            m_freqList[m_main[key].freq].append(key);
        }
        emit cacheHit(key);
    } else {
        m_stats.totalMisses++;
        emit cacheMiss(key);
    }

    m_stats.hitRate = (m_stats.totalAccesses > 0)
        ? static_cast<double>(m_stats.totalHits) / m_stats.totalAccesses
        : 0.0;

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalAccesses, 1ULL);

    return hit;
}

void TinyLfuCache::put(int key, const QByteArray& value)
{
    QElapsedTimer timer;
    timer.start();

    /* 如果已存在，更新值 */
    if (m_window.contains(key)) {
        m_window[key].value = value;
        access(key);
        return;
    }
    if (m_main.contains(key)) {
        m_main[key].value = value;
        access(key);
        return;
    }

    /* 新元素: 先放入窗口缓存 */
    if (m_windowSize >= m_windowCapacity) {
        evictFromWindow();
    }

    m_window[key] = {value, 1};
    m_windowLru.prepend(key);
    m_windowSize++;

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalAccesses, 1ULL);
}

QByteArray TinyLfuCache::get(int key)
{
    if (m_main.contains(key)) {
        access(key);
        return m_main[key].value;
    }
    if (m_window.contains(key)) {
        access(key);
        return m_window[key].value;
    }
    access(key);
    return QByteArray();
}

bool TinyLfuCache::contains(int key) const
{
    return m_window.contains(key) || m_main.contains(key);
}

bool TinyLfuCache::remove(int key)
{
    if (m_window.contains(key)) {
        m_window.remove(key);
        m_windowLru.removeOne(key);
        m_windowSize--;
        return true;
    }
    if (m_main.contains(key)) {
        int freq = m_main[key].freq;
        m_freqList[freq].removeOne(key);
        if (m_freqList[freq].isEmpty()) m_freqList.remove(freq);
        m_main.remove(key);
        m_mainSize--;
        return true;
    }
    return false;
}

void TinyLfuCache::clear()
{
    m_window.clear();
    m_windowLru.clear();
    m_main.clear();
    m_freqList.clear();
    m_freqCounter.clear();
    m_windowSize = 0;
    m_mainSize = 0;
    m_minFreq = 0;
    m_totalAccesses = 0;
}

void TinyLfuCache::updateFrequency(int key)
{
    m_freqCounter[key]++;

    /* 简单衰减: 每N次访问将所有频率减半 */
    if (m_totalAccesses > 0 && m_totalAccesses % m_capacity == 0) {
        QHash<int, int> decayed;
        for (auto it = m_freqCounter.constBegin();
             it != m_freqCounter.constEnd(); ++it) {
            int v = it.value() / 2;
            if (v > 0) decayed[it.key()] = v;
        }
        m_freqCounter = decayed;
    }
}

int TinyLfuCache::frequency(int key) const
{
    return m_freqCounter.value(key, 0);
}

void TinyLfuCache::evictFromWindow()
{
    if (m_windowLru.isEmpty()) return;

    /* LRU淘汰窗口尾部 */
    int victimKey = m_windowLru.last();
    CacheEntry victim = m_window.value(victimKey);
    m_window.remove(victimKey);
    m_windowLru.removeLast();
    m_windowSize--;

    /* 尝试准入到主缓存 */
    tryAdmit(victimKey, victim.value);
}

void TinyLfuCache::evictFromMain()
{
    if (m_minFreq <= 0 || !m_freqList.contains(m_minFreq)
        || m_freqList[m_minFreq].isEmpty()) {
        /* 重新扫描找最小频率 */
        m_minFreq = INT_MAX;
        for (auto it = m_freqList.constBegin(); it != m_freqList.constEnd(); ++it) {
            if (!it.value().isEmpty() && it.key() < m_minFreq) {
                m_minFreq = it.key();
            }
        }
    }

    if (m_minFreq == INT_MAX || !m_freqList.contains(m_minFreq)
        || m_freqList[m_minFreq].isEmpty()) return;

    int victimKey = m_freqList[m_minFreq].first();
    m_freqList[m_minFreq].removeFirst();
    if (m_freqList[m_minFreq].isEmpty()) m_freqList.remove(m_minFreq);
    m_main.remove(victimKey);
    m_mainSize--;

    m_stats.totalEvictions++;
    emit evicted(victimKey);
}

void TinyLfuCache::tryAdmit(int key, const QByteArray& value)
{
    if (m_mainSize >= m_mainCapacity) {
        /* 比较窗口候选者与主缓存中最低频率项 */
        int candidateFreq = frequency(key);
        /* 获取主缓存最低频率 */
        int minMainFreq = INT_MAX;
        for (auto it = m_main.constBegin(); it != m_main.constEnd(); ++it) {
            if (it->freq < minMainFreq) minMainFreq = it->freq;
        }

        if (candidateFreq > minMainFreq) {
            evictFromMain();
        } else {
            /* 不准入，直接丢弃 */
            m_stats.totalEvictions++;
            emit evicted(key);
            return;
        }
    }

    /* 准入到主缓存 */
    int freq = qMax(1, frequency(key));
    m_main[key] = {value, freq};
    m_freqList[freq].append(key);
    m_minFreq = qMin(m_minFreq, freq);
    m_mainSize++;

    m_stats.totalAdmissions++;
    emit admitted(key);
}

void TinyLfuCache::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
