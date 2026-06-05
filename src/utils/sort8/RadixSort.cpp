/**
 * @file RadixSort.cpp
 * @brief 基数排序实现 — LSD与MSD双模式
 */

#include "RadixSort.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---------- 构造/设置 ---------- */

RadixSort::RadixSort(int radix, QObject* parent)
    : QObject(parent), m_radix(radix)
{
    /* 基数至少为2 */
    m_radix = qMax(2, m_radix);
}

void RadixSort::setRadix(int radix)
{
    m_radix = qMax(2, radix);
}

/* ---------- LSD基数排序 ---------- */

QVector<QVariant> RadixSort::sortLSD(const QVector<QVariant>& data,
                                       KeyExtractor keyExtractor,
                                       int maxDigits)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVariant> result = data;
    int n = result.size();

    if (n <= 1) {
        m_stats.totalSorts++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSorts;
        return result;
    }

    /* 从最低位到最高位逐趟计数排序 */
    for (int d = 0; d < maxDigits; ++d) {
        countingSort(result, keyExtractor, d);
    }

    m_stats.totalSorts++;
    m_stats.totalElementsSorted += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSorts;

    emit sortCompleted(n, timer.elapsed());
    return result;
}

/* ---------- MSD基数排序 ---------- */

QVector<QVariant> RadixSort::sortMSD(const QVector<QVariant>& data,
                                       KeyExtractor keyExtractor,
                                       int maxDigits)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVariant> result = data;

    if (result.size() <= 1) {
        m_stats.totalSorts++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSorts;
        return result;
    }

    msdRecursive(result, keyExtractor, 0, maxDigits);

    m_stats.totalSorts++;
    m_stats.totalElementsSorted += result.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSorts;

    emit sortCompleted(result.size(), timer.elapsed());
    return result;
}

/* ---------- 整数便捷排序 ---------- */

QVector<qint64> RadixSort::sortIntegers(const QVector<qint64>& values)
{
    QElapsedTimer timer;
    timer.start();

    int n = values.size();
    if (n <= 1) {
        m_stats.totalSorts++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSorts;
        return values;
    }

    /* 分离正数和负数: 负数取反后按正数排序再反转 */
    QVector<quint64> keys;
    keys.reserve(n);

    /* 偏移量: 将有符号转为无符号(翻转符号位) */
    for (int i = 0; i < n; ++i) {
        quint64 key = static_cast<quint64>(values[i])
                      ^ (1ULL << 63);
        keys.append(key);
    }

    /* LSD按256基排序8个字节 */
    QVector<int> indices(n);
    for (int i = 0; i < n; ++i) indices[i] = i;

    for (int byte = 0; byte < 8; ++byte) {
        QVector<int> output(n);
        QVector<int> count(256, 0);
        int shift = byte * 8;

        for (int i = 0; i < n; ++i) {
            int digit = (keys[indices[i]] >> shift) & 0xFF;
            count[digit]++;
        }

        /* 前缀和 */
        for (int i = 1; i < 256; ++i) {
            count[i] += count[i - 1];
        }

        /* 从后向前放置(保持稳定性) */
        for (int i = n - 1; i >= 0; --i) {
            int digit = (keys[indices[i]] >> shift) & 0xFF;
            output[--count[digit]] = indices[i];
        }

        indices = output;
    }

    QVector<qint64> result;
    result.reserve(n);
    for (int i = 0; i < n; ++i) {
        result.append(values[indices[i]]);
    }

    m_stats.totalSorts++;
    m_stats.totalElementsSorted += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSorts;

    emit sortCompleted(n, timer.elapsed());
    return result;
}

/* ---------- 统计 ---------- */

RadixSort::Stats RadixSort::stats() const { return m_stats; }

void RadixSort::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/* ---------- 私有: LSD单趟计数排序 ---------- */

void RadixSort::countingSort(QVector<QVariant>& data,
                              KeyExtractor& extractor,
                              int digitIndex) const
{
    int n = data.size();
    QVector<QVariant> output(n);
    QVector<int> count(m_radix, 0);

    /* 统计每个数位值的出现次数 */
    for (int i = 0; i < n; ++i) {
        quint64 digitVal = extractor(data[i], digitIndex);
        int digit = static_cast<int>(digitVal % m_radix);
        count[digit]++;
    }

    /* 前缀和: 将计数转为位置 */
    for (int i = 1; i < m_radix; ++i) {
        count[i] += count[i - 1];
    }

    /* 从后向前放置，保证稳定性 */
    for (int i = n - 1; i >= 0; --i) {
        quint64 digitVal = extractor(data[i], digitIndex);
        int digit = static_cast<int>(digitVal % m_radix);
        output[--count[digit]] = data[i];
    }

    data = output;
}

/* ---------- 私有: MSD递归 ---------- */

void RadixSort::msdRecursive(QVector<QVariant>& data,
                              KeyExtractor& extractor,
                              int digitIndex, int maxDigits) const
{
    if (data.size() <= 1 || digitIndex >= maxDigits) return;

    int n = data.size();

    /* 按当前数位分桶 */
    QMap<int, QVector<QVariant>> buckets;
    for (int i = 0; i < n; ++i) {
        quint64 digitVal = extractor(data[i], digitIndex);
        int digit = static_cast<int>(digitVal % m_radix);
        buckets[digit].append(data[i]);
    }

    /* 递归排序每个桶 */
    QVector<QVariant> sorted;
    sorted.reserve(n);
    for (auto it = buckets.begin(); it != buckets.end(); ++it) {
        if (it.value().size() > 1) {
            msdRecursive(it.value(), extractor, digitIndex + 1, maxDigits);
        }
        for (const auto& item : it.value()) {
            sorted.append(item);
        }
    }

    data = sorted;
}
