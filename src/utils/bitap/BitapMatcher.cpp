/**
 * @file BitapMatcher.cpp
 * @brief Bitap近似字符串匹配引擎实现 — 位移掩码模糊搜索
 */

#include "utils/bitap/BitapMatcher.h"

#include <QElapsedTimer>
#include <QtGlobal>

/** @brief 构造函数 @param parent 父对象 */
BitapMatcher::BitapMatcher(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 近似字符串搜索 @param text 文本 @param pattern 模式 @param maxErrors 最大编辑距离 @return 匹配位置列表 */
QVector<int> BitapMatcher::search(const QString& text, const QString& pattern,
                                   int maxErrors)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result;
    int n = text.length();
    int m = pattern.length();

    if (m == 0 || n == 0 || m > 63) {
        /* 模式过长或为空，回退到简单搜索 */
        if (m > 0 && m <= n && maxErrors == 0) {
            for (int i = 0; i <= n - m; ++i) {
                bool match = true;
                for (int j = 0; j < m; ++j) {
                    if (text[i + j] != pattern[j]) { match = false; break; }
                }
                if (match) result.append(i);
            }
        }

        ++m_stats.totalSearches;
        m_stats.totalMatches += static_cast<quint64>(result.size());
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;
        emit searchCompleted(result.size());
        return result;
    }

    maxErrors = qBound(0, maxErrors, m);

    /* 构建字符掩码表(使用Unicode码点取模简化) */
    const int ALPHABET_SIZE = 256;
    QVector<quint64> charMask(ALPHABET_SIZE, ~0ULL);

    for (int j = 0; j < m; ++j) {
        int idx = pattern[j].unicode() % ALPHABET_SIZE;
        charMask[idx] &= ~(1ULL << j);
    }

    /* 初始化状态: R[d]表示允许d个错误的匹配状态 */
    QVector<quint64> R(maxErrors + 1, ~0ULL);

    for (int i = 0; i < n; ++i) {
        int ch = text[i].unicode() % ALPHABET_SIZE;
        quint64 oldRd0 = R[0];

        /* 精确匹配状态更新 */
        R[0] = (R[0] << 1) | charMask[ch];

        /* 近似匹配状态更新(允许0..maxErrors个错误) */
        for (int d = 1; d <= maxErrors; ++d) {
            quint64 tmp = R[d];
            /* 替换 + 删除 + 插入 + 精确传播 */
            R[d] = ((R[d] << 1) | charMask[ch])
                 & (((oldRd0) << 1) | 1ULL)     /* 替换 */
                 & (oldRd0)                       /* 删除 */
                 & (R[d - 1] << 1);              /* 插入 */
            oldRd0 = tmp;
        }

        /* 检查是否有匹配完成 */
        if ((R[maxErrors] & (1ULL << (m - 1))) == 0) {
            result.append(i - m + 1);
        }
    }

    /* 更新统计 */
    ++m_stats.totalSearches;
    m_stats.totalMatches += static_cast<quint64>(result.size());
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;

    emit searchCompleted(result.size());
    return result;
}

/** @brief 精确字符串搜索(Shift-Or) @param text 文本 @param pattern 模式 @return 匹配位置列表 */
QVector<int> BitapMatcher::exactSearch(const QString& text,
                                        const QString& pattern)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result;
    int n = text.length();
    int m = pattern.length();

    if (m == 0 || n == 0 || m > 63) {
        /* 回退到朴素搜索 */
        if (m > 0 && m <= n) {
            for (int i = 0; i <= n - m; ++i) {
                bool match = true;
                for (int j = 0; j < m; ++j) {
                    if (text[i + j] != pattern[j]) { match = false; break; }
                }
                if (match) result.append(i);
            }
        }

        ++m_stats.totalSearches;
        m_stats.totalMatches += static_cast<quint64>(result.size());
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;
        emit searchCompleted(result.size());
        return result;
    }

    /* 构建字符掩码 */
    const int ALPHABET_SIZE = 256;
    QVector<quint64> charMask(ALPHABET_SIZE, ~0ULL);

    for (int j = 0; j < m; ++j) {
        int idx = pattern[j].unicode() % ALPHABET_SIZE;
        charMask[idx] &= ~(1ULL << j);
    }

    /* Shift-Or搜索 */
    quint64 state = ~0ULL;
    quint64 matchBit = 1ULL << (m - 1);

    for (int i = 0; i < n; ++i) {
        int ch = text[i].unicode() % ALPHABET_SIZE;
        state = (state << 1) | charMask[ch];
        if ((state & matchBit) == 0) {
            result.append(i - m + 1);
        }
    }

    /* 更新统计 */
    ++m_stats.totalSearches;
    m_stats.totalMatches += static_cast<quint64>(result.size());
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;

    emit searchCompleted(result.size());
    return result;
}

/** @brief 重置统计 */
void BitapMatcher::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
