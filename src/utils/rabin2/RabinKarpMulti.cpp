/**
 * @file RabinKarpMulti.cpp
 * @brief Rabin-Karp匹配实现
 */

#include "utils/rabin2/RabinKarpMulti.h"

#include <QElapsedTimer>

/** @brief 构造函数 @param parent 父对象 */
RabinKarpMulti::RabinKarpMulti(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 搜索所有匹配 */
QVector<int> RabinKarpMulti::search(const QByteArray& text,
                                       const QByteArray& pattern)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> matches;
    int n = text.size();
    int m = pattern.size();
    if (m == 0 || m > n) return matches;

    quint64 patHash = computeHash(pattern, 0, m);
    quint64 txtHash = computeHash(text, 0, m);

    /* 预计算h = BASE^(m-1) % MOD */
    quint64 h = 1;
    for (int i = 0; i < m - 1; ++i)
        h = (h * BASE) % MOD;

    for (int i = 0; i <= n - m; ++i) {
        if (txtHash == patHash) {
            bool match = true;
            for (int j = 0; j < m; ++j) {
                if (text[i + j] != pattern[j]) {
                    match = false;
                    break;
                }
            }
            if (match) matches.append(i);
        }

        if (i < n - m) {
            txtHash = (txtHash - static_cast<uchar>(text[i]) * h % MOD + MOD) % MOD;
            txtHash = (txtHash * BASE + static_cast<uchar>(text[i + m])) % MOD;
        }
    }

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalSearches;
    m_stats.totalMatches += static_cast<quint64>(matches.size());
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalSearches);

    emit searchCompleted(matches.size());
    return matches;
}

/** @brief 多模式搜索 */
QVector<QVector<int>> RabinKarpMulti::multiSearch(
    const QByteArray& text,
    const QVector<QByteArray>& patterns)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<int>> results(patterns.size());

    /* 使用相同长度分桶 */
    int n = text.size();
    for (int p = 0; p < patterns.size(); ++p) {
        results[p] = search(text, patterns[p]);
    }

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalSearches;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalSearches);

    return results;
}

/** @brief 计算滚动哈希 */
quint64 RabinKarpMulti::computeHash(const QByteArray& data,
                                       int start, int len) const
{
    quint64 hash = 0;
    for (int i = start; i < start + len; ++i)
        hash = (hash * BASE + static_cast<uchar>(data[i])) % MOD;
    return hash;
}

/** @brief 重置统计 */
void RabinKarpMulti::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
