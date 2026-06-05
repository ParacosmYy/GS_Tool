/**
 * @file RadixSorter.cpp
 * @brief 基数排序器实现
 */

#include "utils/sort2/RadixSorter.h"

#include <QElapsedTimer>
#include <algorithm>

RadixSorter::RadixSorter(int radixBits, QObject* parent)
    : QObject(parent), m_radixBits(qBound(4, radixBits, 16)), m_timeSum(0.0)
{
    m_stats.radixBits = m_radixBits;
}

QVector<quint32> RadixSorter::sortLsd(const QVector<quint32>& data,
                                        SortOrder order)
{
    QElapsedTimer timer;
    timer.start();

    QVector<quint32> result = data;
    int n = result.size();
    if (n < 2) return result;

    int radix = 1 << m_radixBits;
    int mask = radix - 1;
    int passes = (32 + m_radixBits - 1) / m_radixBits;
    QVector<quint32> buffer(n);

    for (int p = 0; p < passes; ++p) {
        int shift = p * m_radixBits;

        /* 计数 */
        QVector<int> count(radix, 0);
        for (int i = 0; i < n; ++i) {
            int digit = (result[i] >> shift) & mask;
            count[digit]++;
        }

        /* 前缀和 */
        for (int i = 1; i < radix; ++i) {
            count[i] += count[i - 1];
        }

        /* 分配(从后往前以保持稳定性) */
        for (int i = n - 1; i >= 0; --i) {
            int digit = (result[i] >> shift) & mask;
            buffer[--count[digit]] = result[i];
        }

        result.swap(buffer);
    }

    if (order == SortOrder::Descending) {
        std::reverse(result.begin(), result.end());
    }

    m_stats.totalSorts++;
    m_stats.totalElementsSorted += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalSorts, 1ULL);

    emit sorted(n, timer.elapsed());
    return result;
}

QVector<qint32> RadixSorter::sortLsdSigned(const QVector<qint32>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n < 2) return data;

    /* 转为无符号: 翻转符号位 */
    QVector<quint32> unsignedData(n);
    for (int i = 0; i < n; ++i) {
        unsignedData[i] = static_cast<quint32>(data[i]) ^ 0x80000000u;
    }

    /* LSD排序 */
    QVector<quint32> sorted = sortLsd(unsignedData, SortOrder::Ascending);

    /* 还原 */
    QVector<qint32> result(n);
    for (int i = 0; i < n; ++i) {
        result[i] = static_cast<qint32>(sorted[i] ^ 0x80000000u);
    }

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalSorts, 1ULL);

    return result;
}

QVector<quint32> RadixSorter::sortMsd(const QVector<quint32>& data,
                                        SortOrder order)
{
    QElapsedTimer timer;
    timer.start();

    QVector<quint32> result = data;
    int n = result.size();
    if (n < 2) return result;

    msdRecursion(result, 0, n - 1, 28);

    if (order == SortOrder::Descending) {
        std::reverse(result.begin(), result.end());
    }

    m_stats.totalSorts++;
    m_stats.totalElementsSorted += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalSorts, 1ULL);

    emit sorted(n, timer.elapsed());
    return result;
}

void RadixSorter::msdRecursion(QVector<quint32>& data, int from, int to,
                                int bitOffset)
{
    if (from >= to || bitOffset < 0) return;

    /* 按当前4位切分 */
    int lo = from, hi = to;
    while (lo <= hi) {
        int digit = (data[lo] >> bitOffset) & 0xF;
        if (digit == 0) {
            ++lo;
        } else {
            std::swap(data[lo], data[hi]);
            --hi;
        }
    }

    /* 对每个桶递归 */
    int buckets[16];
    int idx = from;
    for (int b = 0; b < 16; ++b) {
        int start = idx;
        while (idx <= to && ((data[idx] >> bitOffset) & 0xF) == b) ++idx;
        buckets[b] = idx;
    }

    for (int b = 0; b < 16; ++b) {
        int s = (b == 0) ? from : buckets[b - 1];
        int e = buckets[b] - 1;
        if (s < e) msdRecursion(data, s, e, bitOffset - 4);
    }
}

QVector<quint8> RadixSorter::sortByCounting(const QVector<quint8>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n < 2) return data;

    /* 计数 */
    int count[256] = {};
    for (int i = 0; i < n; ++i) {
        count[static_cast<int>(data[i])]++;
    }

    /* 输出 */
    QVector<quint8> result;
    result.reserve(n);
    for (int v = 0; v < 256; ++v) {
        for (int c = 0; c < count[v]; ++c) {
            result.append(static_cast<quint8>(v));
        }
    }

    m_stats.totalSorts++;
    m_stats.totalElementsSorted += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalSorts, 1ULL);

    emit sorted(n, timer.elapsed());
    return result;
}

void RadixSorter::sortLsdInPlace(QVector<quint32>& data, SortOrder order)
{
    data = sortLsd(data, order);
}

void RadixSorter::resetStatistics()
{
    m_stats = Stats{};
    m_stats.radixBits = m_radixBits;
    m_timeSum = 0.0;
}
