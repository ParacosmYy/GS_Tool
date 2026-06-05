/**
 * @file RollingHash.h
 * @brief 滚动哈希 — Rabin-Karp字符串匹配哈希
 *
 * 功能: 基于多项式滚动哈希的字符串匹配，支持多模式、
 *       窗口滑动O(1)更新，统计哈希计算/匹配数/耗时。
 */
#ifndef ROLLINGHASH_H
#define ROLLINGHASH_H

#include <QObject>
#include <QByteArray>
#include <QVector>
#include <QSet>

class RollingHash : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalWindowsHashed = 0;
        quint64 totalMatches = 0;
        quint64 totalFalsePositives = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit RollingHash(quint64 base = 257, quint64 mod = 1000000007,
                          QObject* parent = nullptr);

    /** @brief 计算模式哈希 @param pattern 模式串 @return 哈希值 */
    quint64 hashPattern(const QByteArray& pattern);

    /** @brief 在文本中搜索模式 @param text 文本 @param pattern 模式 @return 匹配位置列表 */
    QVector<int> search(const QByteArray& text, const QByteArray& pattern);

    /** @brief 滑动窗口哈希序列 @param data 数据 @param windowSize 窗口大小 @return 哈希序列 */
    QVector<quint64> rollingHashes(const QByteArray& data, int windowSize);

    /** @brief 多模式搜索 @param text 文本 @param patterns 模式列表 @return pattern→匹配位置 */
    QHash<QByteArray, QVector<int>> multiSearch(
        const QByteArray& text, const QVector<QByteArray>& patterns);

    quint64 base() const { return m_base; }
    quint64 mod() const { return m_mod; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void matchFound(int position, const QByteArray& pattern);

private:
    quint64 m_base;
    quint64 m_mod;
    Stats m_stats;
    double m_timeSum;
};

#endif // ROLLINGHASH_H
