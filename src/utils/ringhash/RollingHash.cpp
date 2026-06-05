/**
 * @file RollingHash.cpp
 * @brief 滚动哈希实现
 */

#include "utils/ringhash/RollingHash.h"

#include <QElapsedTimer>
#include <QHash>

RollingHash::RollingHash(quint64 base, quint64 mod, QObject* parent)
    : QObject(parent), m_base(base), m_mod(mod), m_timeSum(0.0) {}

quint64 RollingHash::hashPattern(const QByteArray& pattern)
{
    quint64 h = 0;
    for (int i = 0; i < pattern.size(); ++i) {
        h = (h * m_base + static_cast<quint8>(pattern[i])) % m_mod;
    }
    return h;
}

QVector<int> RollingHash::search(const QByteArray& text,
                                  const QByteArray& pattern)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> matches;
    int n = text.size();
    int m = pattern.size();
    if (m == 0 || n < m) return matches;

    /* 预计算base^m */
    quint64 hpm = 1;
    for (int i = 0; i < m; ++i) hpm = (hpm * m_base) % m_mod;

    /* 初始窗口哈希 */
    quint64 textHash = 0, patHash = 0;
    for (int i = 0; i < m; ++i) {
        textHash = (textHash * m_base + static_cast<quint8>(text[i])) % m_mod;
        patHash = (patHash * m_base + static_cast<quint8>(pattern[i])) % m_mod;
    }

    for (int i = 0; i <= n - m; ++i) {
        if (textHash == patHash) {
            bool match = true;
            for (int j = 0; j < m; ++j) {
                if (text[i + j] != pattern[j]) {
                    match = false;
                    break;
                }
            }
            if (match) {
                matches.append(i);
                m_stats.totalMatches++;
            } else {
                m_stats.totalFalsePositives++;
            }
        }

        /* 滑动窗口 */
        if (i < n - m) {
            quint64 out = (static_cast<quint8>(text[i]) * hpm) % m_mod;
            textHash = (textHash - out + m_mod) % m_mod;
            textHash = (textHash * m_base + static_cast<quint8>(text[i + m])) % m_mod;
        }

        m_stats.totalWindowsHashed++;
    }

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalWindowsHashed, 1ULL);

    return matches;
}

QVector<quint64> RollingHash::rollingHashes(const QByteArray& data,
                                             int windowSize)
{
    QElapsedTimer timer;
    timer.start();

    QVector<quint64> hashes;
    int n = data.size();
    if (n < windowSize || windowSize < 1) return hashes;

    quint64 hpm = 1;
    for (int i = 0; i < windowSize; ++i) hpm = (hpm * m_base) % m_mod;

    quint64 h = 0;
    for (int i = 0; i < windowSize; ++i) {
        h = (h * m_base + static_cast<quint8>(data[i])) % m_mod;
    }
    hashes.append(h);

    for (int i = windowSize; i < n; ++i) {
        quint64 out = (static_cast<quint8>(data[i - windowSize]) * hpm) % m_mod;
        h = (h - out + m_mod) % m_mod;
        h = (h * m_base + static_cast<quint8>(data[i])) % m_mod;
        hashes.append(h);
        m_stats.totalWindowsHashed++;
    }

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalWindowsHashed, 1ULL);

    return hashes;
}

QHash<QByteArray, QVector<int>> RollingHash::multiSearch(
    const QByteArray& text, const QVector<QByteArray>& patterns)
{
    QHash<QByteArray, QVector<int>> results;
    for (const auto& pat : patterns) {
        results[pat] = search(text, pat);
    }
    return results;
}

void RollingHash::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
