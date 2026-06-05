/**
 * @file RollingHash2.cpp
 * @brief 滚动哈希实现
 */

#include "RollingHash2.h"
#include <QElapsedTimer>

RollingHash2::RollingHash2(int windowSize, QObject* parent)
    : QObject(parent)
    , m_windowSize(qMax(1, windowSize))
    , m_window(m_windowSize, 0)
    , m_windowPos(0)
    , m_hash1(0)
    , m_hash2(0)
    , m_timeSum(0.0)
{
    precomputePowers();
}

void RollingHash2::precomputePowers()
{
    m_basePow1 = 1;
    m_basePow2 = 1;
    for (int i = 0; i < m_windowSize; ++i) {
        m_basePow1 = modMul(m_basePow1, BASE1, MOD1);
        m_basePow2 = modMul(m_basePow2, BASE2, MOD2);
    }
}

quint64 RollingHash2::modMul(quint64 a, quint64 b, quint64 mod) const
{
    /* 使用128位乘法取模 */
    __uint128_t product = static_cast<__uint128_t>(a) * b;
    return static_cast<quint64>(product % mod);
}

void RollingHash2::init(const QByteArray& data)
{
    m_hash1 = 0;
    m_hash2 = 0;
    m_windowPos = 0;

    int len = qMin(data.size(), m_windowSize);
    m_window = QByteArray(m_windowSize, 0);

    for (int i = 0; i < len; ++i) {
        quint8 byte = static_cast<quint8>(data[i]);
        m_window[i] = data[i];
        m_hash1 = (modMul(m_hash1, BASE1, MOD1) + byte) % MOD1;
        m_hash2 = (modMul(m_hash2, BASE2, MOD2) + byte) % MOD2;
    }
    m_windowPos = len % m_windowSize;
}

void RollingHash2::roll(char inByte)
{
    QElapsedTimer timer;
    timer.start();

    quint8 outByte = static_cast<quint8>(m_window[m_windowPos]);

    /* 哈希1更新 */
    m_hash1 = (modMul(m_hash1, BASE1, MOD1) + static_cast<quint8>(inByte)) % MOD1;
    quint64 outContrib1 = modMul(outByte, m_basePow1, MOD1);
    m_hash1 = (m_hash1 + MOD1 - outContrib1) % MOD1;

    /* 哈希2更新 */
    m_hash2 = (modMul(m_hash2, BASE2, MOD2) + static_cast<quint8>(inByte)) % MOD2;
    quint64 outContrib2 = modMul(outByte, m_basePow2, MOD2);
    m_hash2 = (m_hash2 + MOD2 - outContrib2) % MOD2;

    m_window[m_windowPos] = inByte;
    m_windowPos = (m_windowPos + 1) % m_windowSize;

    m_stats.totalUpdates++;
    m_timeSum += timer.elapsed();
    if (m_stats.totalUpdates > 0)
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalUpdates;
}

quint64 RollingHash2::hash1() const { return m_hash1; }
quint64 RollingHash2::hash2() const { return m_hash2; }
quint64 RollingHash2::combinedHash() const { return m_hash1 ^ m_hash2; }

QVector<int> RollingHash2::search(const QByteArray& text, const QByteArray& pattern) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> matches;
    int n = text.size();
    int m = pattern.size();
    if (m == 0 || m > n) return matches;

    RollingHash2 patternHash(m);
    patternHash.init(pattern);

    RollingHash2 textHash(m);
    textHash.init(text.left(m));

    if (textHash.combinedHash() == patternHash.combinedHash()) {
        if (text.left(m) == pattern)
            matches.append(0);
    }

    for (int i = m; i < n; ++i) {
        textHash.roll(text[i]);
        int pos = i - m + 1;
        if (textHash.combinedHash() == patternHash.combinedHash()) {
            if (text.mid(pos, m) == pattern)
                matches.append(pos);
        }
    }

    m_stats.totalMatches += matches.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalUpdates + m_stats.totalMatches);

    return matches;
}

QVector<int> RollingHash2::chunk(const QByteArray& data, int minChunk, int maxChunk, int mask) const
{
    QVector<int> boundaries;
    int pos = 0;
    int n = data.size();

    while (pos < n) {
        int end = qMin(pos + maxChunk, n);
        RollingHash2 rh(m_windowSize);
        rh.init(data.mid(pos, qMin(m_windowSize, end - pos)));

        int chunkEnd = end;
        for (int i = pos + m_windowSize; i < end; ++i) {
            rh.roll(data[i]);
            if (i - pos >= minChunk && (rh.combinedHash() & mask) == 0) {
                chunkEnd = i + 1;
                break;
            }
        }

        boundaries.append(pos);
        pos = chunkEnd;
    }

    return boundaries;
}

QVector<int> RollingHash2::multiSearch(const QByteArray& text,
                                        const QVector<QByteArray>& patterns) const
{
    QVector<int> allMatches;
    for (const auto& pattern : patterns) {
        auto matches = search(text, pattern);
        allMatches.append(matches);
    }
    return allMatches;
}

void RollingHash2::setWindowSize(int size)
{
    m_windowSize = qMax(1, size);
    m_window = QByteArray(m_windowSize, 0);
    m_windowPos = 0;
    m_hash1 = 0;
    m_hash2 = 0;
    precomputePowers();
}

RollingHash2::Stats RollingHash2::stats() const { return m_stats; }

void RollingHash2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
