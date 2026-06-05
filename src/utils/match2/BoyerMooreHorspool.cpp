/**
 * @file BoyerMooreHorspool.cpp
 * @brief BoyerMooreHorspool 实现 — 坏字符跳跃快速字符串/字节匹配
 *
 * BMH算法核心:
 * 1. 预计算坏字符跳跃表(对于模式长度m, 表大小为字母表大小)
 * 2. 从文本左侧开始, 模式从右向左比较
 * 3. 不匹配时, 根据文本中对应的字符查表决定跳跃距离
 * 4. 平均时间复杂度O(n/m), 最坏O(nm)
 */

#include "utils/match2/BoyerMooreHorspool.h"

#include <algorithm>

/** @brief 字母表大小(ASCII + 扩展到256) */
static constexpr int ALPHABET_SIZE = 256;

// ── 构造 / 析构 ──

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
BoyerMooreHorspool::BoyerMooreHorspool(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("BoyerMooreHorspool"));
    m_timer.start();
}

BoyerMooreHorspool::~BoyerMooreHorspool() = default;

// ── 搜索接口 ──

/**
 * @brief 在QString中搜索模式(返回第一个匹配位置)
 *
 * 将QString转为UTF-16码点数组, 使用坏字符表进行BMH搜索。
 * 返回只包含第一个匹配的单元素列表, 或空列表表示未找到。
 * @param text    待搜索文本
 * @param pattern 搜索模式
 * @return 包含第一个匹配位置的列表(或空)
 */
QVector<int> BoyerMooreHorspool::search(const QString& text, const QString& pattern)
{
    m_timer.restart();

    QVector<int> results;
    if (pattern.isEmpty() || text.isEmpty() || pattern.length() > text.length()) {
        double elapsed = static_cast<double>(m_timer.nsecsElapsed()) / 1e6;
        ++m_stats.totalSearches;
        ++m_stats.totalSearches;
        m_stats.avgProcessingTimeMs =
            (m_stats.avgProcessingTimeMs * static_cast<double>(m_stats.totalSearches - 1)
             + elapsed) / static_cast<double>(m_stats.totalSearches);
        emit searchCompleted(0, elapsed);
        return results;
    }

    QVector<int> badChar = buildBadCharTable(pattern);
    int n = text.length();
    int m = pattern.length();
    int skip = 0;

    while (skip <= n - m) {
        /* 从右向左比较 */
        int j = m - 1;
        while (j >= 0 && text[skip + j] == pattern[j]) {
            --j;
        }

        if (j < 0) {
            /* 完全匹配 */
            results.append(skip);
            ++m_stats.totalMatches;
            break; // 只返回第一个匹配
        }

        /* 根据坏字符跳跃 */
        int badCharIndex = text[skip + m - 1].unicode() % ALPHABET_SIZE;
        skip += badChar[badCharIndex];
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(m_timer.nsecsElapsed()) / 1e6;
    ++m_stats.totalSearches;
    m_stats.avgProcessingTimeMs =
        (m_stats.avgProcessingTimeMs * static_cast<double>(m_stats.totalSearches - 1)
         + elapsed) / static_cast<double>(m_stats.totalSearches);
    emit searchCompleted(results.size(), elapsed);
    return results;
}

/**
 * @brief 在QByteArray中搜索字节模式(返回第一个匹配位置)
 *
 * 使用字节级坏字符表进行BMH搜索, 适用于二进制数据模式匹配。
 * @param data    待搜索数据
 * @param pattern 字节模式
 * @return 包含第一个匹配位置的列表(或空)
 */
QVector<int> BoyerMooreHorspool::search(const QByteArray& data, const QByteArray& pattern)
{
    m_timer.restart();

    QVector<int> results;
    if (pattern.isEmpty() || data.isEmpty() || pattern.size() > data.size()) {
        double elapsed = static_cast<double>(m_timer.nsecsElapsed()) / 1e6;
        ++m_stats.totalSearches;
        ++m_stats.totalSearches;
        m_stats.avgProcessingTimeMs =
            (m_stats.avgProcessingTimeMs * static_cast<double>(m_stats.totalSearches - 1)
             + elapsed) / static_cast<double>(m_stats.totalSearches);
        emit searchCompleted(0, elapsed);
        return results;
    }

    QVector<int> badChar = buildByteBadCharTable(pattern);
    int n = data.size();
    int m = pattern.size();
    int skip = 0;

    while (skip <= n - m) {
        int j = m - 1;
        while (j >= 0 && data[skip + j] == pattern[j]) {
            --j;
        }

        if (j < 0) {
            results.append(skip);
            ++m_stats.totalMatches;
            break;
        }

        unsigned char badCharVal = static_cast<unsigned char>(data[skip + m - 1]);
        skip += badChar[badCharVal];
    }

    double elapsed = static_cast<double>(m_timer.nsecsElapsed()) / 1e6;
    ++m_stats.totalSearches;
    m_stats.avgProcessingTimeMs =
        (m_stats.avgProcessingTimeMs * static_cast<double>(m_stats.totalSearches - 1)
         + elapsed) / static_cast<double>(m_stats.totalSearches);
    emit searchCompleted(results.size(), elapsed);
    return results;
}

/**
 * @brief 搜索所有匹配位置
 *
 * 找到匹配后从 matchPos + 1 处继续搜索, 收集所有匹配。
 * @param text    待搜索文本
 * @param pattern 搜索模式
 * @return 所有匹配起始位置(升序)
 */
QVector<int> BoyerMooreHorspool::searchAll(const QString& text, const QString& pattern)
{
    m_timer.restart();

    QVector<int> results;
    if (pattern.isEmpty() || text.isEmpty() || pattern.length() > text.length()) {
        double elapsed = static_cast<double>(m_timer.nsecsElapsed()) / 1e6;
        ++m_stats.totalSearches;
        ++m_stats.totalSearches;
        m_stats.avgProcessingTimeMs =
            (m_stats.avgProcessingTimeMs * static_cast<double>(m_stats.totalSearches - 1)
             + elapsed) / static_cast<double>(m_stats.totalSearches);
        emit searchCompleted(0, elapsed);
        return results;
    }

    QVector<int> badChar = buildBadCharTable(pattern);
    int n = text.length();
    int m = pattern.length();
    int skip = 0;

    while (skip <= n - m) {
        int j = m - 1;
        while (j >= 0 && text[skip + j] == pattern[j]) {
            --j;
        }

        if (j < 0) {
            results.append(skip);
            ++m_stats.totalMatches;
            /* 找到匹配后前进1个位置继续搜索 */
            skip += 1;
        } else {
            int badCharIndex = text[skip + m - 1].unicode() % ALPHABET_SIZE;
            skip += badChar[badCharIndex];
        }
    }

    double elapsed = static_cast<double>(m_timer.nsecsElapsed()) / 1e6;
    ++m_stats.totalSearches;
    m_stats.avgProcessingTimeMs =
        (m_stats.avgProcessingTimeMs * static_cast<double>(m_stats.totalSearches - 1)
         + elapsed) / static_cast<double>(m_stats.totalSearches);
    emit searchCompleted(results.size(), elapsed);
    return results;
}

/**
 * @brief 构建坏字符跳跃表(QString版本)
 *
 * 对于Unicode码点映射到[0,255]区间, 预计算每个字符的跳跃距离。
 * 不在模式中出现的字符跳跃距离 = 模式长度。
 * @param pattern 搜索模式
 * @return 跳跃距离表(大小256)
 */
QVector<int> BoyerMooreHorspool::buildBadCharTable(const QString& pattern)
{
    int m = pattern.length();
    QVector<int> table(ALPHABET_SIZE, m);

    for (int i = 0; i < m - 1; ++i) {
        int idx = pattern[i].unicode() % ALPHABET_SIZE;
        table[idx] = m - 1 - i;
    }
    return table;
}

// ── 统计 ──

BoyerMooreHorspool::Stats BoyerMooreHorspool::stats() const
{
    return m_stats;
}

void BoyerMooreHorspool::resetStatistics()
{
    m_stats = Stats{};
}

// ── 私有方法 ──

/**
 * @brief 为字节数组模式构建坏字符表
 * @param pattern 字节模式
 * @return 跳跃距离表(大小256)
 */
QVector<int> BoyerMooreHorspool::buildByteBadCharTable(const QByteArray& pattern)
{
    int m = pattern.size();
    QVector<int> table(ALPHABET_SIZE, m);

    for (int i = 0; i < m - 1; ++i) {
        unsigned char ch = static_cast<unsigned char>(pattern[i]);
        table[ch] = m - 1 - i;
    }
    return table;
}
