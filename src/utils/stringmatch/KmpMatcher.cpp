/**
 * @file KmpMatcher.cpp
 * @brief KMP字符串匹配实现
 */

#include "utils/stringmatch/KmpMatcher.h"

#include <QElapsedTimer>

/** @brief 构造函数 @param parent 父对象 */
KmpMatcher::KmpMatcher(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 搜索所有匹配位置 */
QVector<int> KmpMatcher::search(const QByteArray& text,
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

    QVector<int> fail = buildFailureFunction(pattern);

    int q = 0;
    for (int i = 0; i < n; ++i) {
        while (q > 0 && pattern[q] != text[i])
            q = fail[q - 1];
        if (pattern[q] == text[i])
            ++q;
        if (q == m) {
            matches.append(i - m + 1);
            q = fail[q - 1];
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
int KmpMatcher::searchFirst(const QByteArray& text,
                             const QByteArray& pattern)
{
    QElapsedTimer timer;
    timer.start();

    int n = text.size();
    int m = pattern.size();
    if (m == 0 || m > n) return -1;

    QVector<int> fail = buildFailureFunction(pattern);

    int q = 0;
    for (int i = 0; i < n; ++i) {
        while (q > 0 && pattern[q] != text[i])
            q = fail[q - 1];
        if (pattern[q] == text[i])
            ++q;
        if (q == m) {
            return i - m + 1;
        }
    }

    return -1;
}

/** @brief 整数序列匹配 */
QVector<int> KmpMatcher::searchIntSequence(const QVector<int>& data,
                                            const QVector<int>& pattern)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> matches;
    int n = data.size();
    int m = pattern.size();
    if (m == 0 || m > n) return matches;

    QVector<int> fail = buildFailureInt(pattern);

    int q = 0;
    for (int i = 0; i < n; ++i) {
        while (q > 0 && pattern[q] != data[i])
            q = fail[q - 1];
        if (pattern[q] == data[i])
            ++q;
        if (q == m) {
            matches.append(i - m + 1);
            q = fail[q - 1];
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

/** @brief 构建KMP失败函数 */
QVector<int> KmpMatcher::buildFailureFunction(const QByteArray& pattern)
{
    int m = pattern.size();
    QVector<int> fail(m, 0);
    int k = 0;

    for (int q = 1; q < m; ++q) {
        while (k > 0 && pattern[k] != pattern[q])
            k = fail[k - 1];
        if (pattern[k] == pattern[q])
            ++k;
        fail[q] = k;
    }

    return fail;
}

/** @brief 整数序列失败函数 */
QVector<int> KmpMatcher::buildFailureInt(const QVector<int>& pattern)
{
    int m = pattern.size();
    QVector<int> fail(m, 0);
    int k = 0;

    for (int q = 1; q < m; ++q) {
        while (k > 0 && pattern[k] != pattern[q])
            k = fail[k - 1];
        if (pattern[k] == pattern[q])
            ++k;
        fail[q] = k;
    }

    return fail;
}

/** @brief 重置统计 */
void KmpMatcher::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
