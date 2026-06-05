/**
 * @file BoyerMooreMatcher.cpp
 * @brief Boyer-Moore字符串匹配实现
 */

#include "utils/stringmatch2/BoyerMooreMatcher.h"

#include <QElapsedTimer>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
BoyerMooreMatcher::BoyerMooreMatcher(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 搜索所有匹配 */
QVector<int> BoyerMooreMatcher::search(const QByteArray& text,
                                        const QByteArray& pattern)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> matches;
    int n = text.size();
    int m = pattern.size();

    if (m == 0 || m > n) {
        double elapsed = static_cast<double>(timer.elapsed());
        m_timeSum += elapsed;
        ++m_stats.totalSearches;
        m_stats.avgProcessingTimeMs = m_timeSum
            / static_cast<double>(m_stats.totalSearches);
        return matches;
    }

    QVector<int> badChar = buildBadCharTable(pattern);
    QVector<int> goodSuffix = buildGoodSuffixTable(pattern);

    int s = 0;
    while (s <= n - m) {
        int j = m - 1;
        while (j >= 0 && pattern[j] == text[s + j])
            --j;

        if (j < 0) {
            matches.append(s);
            s += qMax(1, goodSuffix[0]);
        } else {
            int bcShift = j - badChar[static_cast<uchar>(text[s + j])];
            int gsShift = goodSuffix[j + 1];
            s += qMax(1, qMax(bcShift, gsShift));
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

/** @brief 搜索第一个匹配 */
int BoyerMooreMatcher::searchFirst(const QByteArray& text,
                                    const QByteArray& pattern)
{
    int n = text.size();
    int m = pattern.size();
    if (m == 0 || m > n) return -1;

    QVector<int> badChar = buildBadCharTable(pattern);
    QVector<int> goodSuffix = buildGoodSuffixTable(pattern);

    int s = 0;
    while (s <= n - m) {
        int j = m - 1;
        while (j >= 0 && pattern[j] == text[s + j])
            --j;

        if (j < 0) return s;

        int bcShift = j - badChar[static_cast<uchar>(text[s + j])];
        int gsShift = goodSuffix[j + 1];
        s += qMax(1, qMax(bcShift, gsShift));
    }

    return -1;
}

/** @brief 构建坏字符表 */
QVector<int> BoyerMooreMatcher::buildBadCharTable(const QByteArray& pattern)
{
    QVector<int> table(256, -1);
    int m = pattern.size();
    for (int i = 0; i < m; ++i)
        table[static_cast<uchar>(pattern[i])] = i;
    return table;
}

/** @brief 构建好后缀表 */
QVector<int> BoyerMooreMatcher::buildGoodSuffixTable(const QByteArray& pattern)
{
    int m = pattern.size();
    QVector<int> table(m + 1, m);

    /* 后缀匹配 */
    QVector<int> suffix(m, 0);
    suffix[m - 1] = m;
    int g = m - 1;
    int f = 0;

    for (int i = m - 2; i >= 0; --i) {
        if (i > g && suffix[i + m - 1 - f] < i - g) {
            suffix[i] = suffix[i + m - 1 - f];
        } else {
            g = qMin(g, i);
            f = i;
            while (g >= 0 && pattern[g] == pattern[g + m - 1 - f])
                --g;
            suffix[i] = f - g;
        }
    }

    /* 好后缀情况2: 后缀在模式前缀出现 */
    for (int i = 0; i <= m; ++i)
        table[i] = m;

    int j = 0;
    for (int i = m - 1; i >= -1; --i) {
        if (i < 0 || suffix[i] == i + 1) {
            for (; j < m - 1 - i; ++j) {
                if (table[j] == m)
                    table[j] = m - 1 - i;
            }
        }
    }

    /* 好后缀情况1: 后缀在模式中其他位置出现 */
    for (int i = 0; i < m - 1; ++i)
        table[m - 1 - suffix[i]] = m - 1 - i;

    return table;
}

/** @brief 重置统计 */
void BoyerMooreMatcher::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
