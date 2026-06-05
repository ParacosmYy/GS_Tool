/**
 * @file SuffixAutomaton.h
 * @brief 后缀自动机(Suffix Automaton / SAM)
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QMap>

/**
 * @class SuffixAutomaton
 * @brief 后缀自动机 — 在线构建字符串的所有子串信息
 *
 * O(n)构建时间，支持子串存在性查询、不同子串计数、
 * 最长公共子串、子串出现次数等操作。
 */
class SuffixAutomaton : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalQueries = 0;       /**< 总查询次数 */
        int totalMatched = 0;       /**< 总匹配次数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit SuffixAutomaton(QObject* parent = nullptr);

    /**
     * @brief 在线扩展: 追加一个字符
     * @param c 字符(Unicode码点)
     */
    void extend(int c);

    /**
     * @brief 构建字符串的后缀自动机
     * @param str 字符串
     */
    void build(const QString& str);

    /** @brief 查询子串是否存在 */
    bool contains(const QString& substr) const;

    /** @brief 统计不同子串数量 */
    long long distinctSubstrings() const;

    /** @brief 查询子串出现次数 */
    int occurrences(const QString& substr) const;

    /**
     * @brief 查询两个字符串的最长公共子串
     * @param s1 第一个字符串
     * @param s2 第二个字符串
     * @return 最长公共子串
     */
    QString longestCommonSubstring(const QString& s1, const QString& s2) const;

    /** @brief 清空自动机 */
    void clear();

    /** @brief 获取状态数 */
    int stateCount() const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 构建完成信号 */
    void buildCompleted(int length, int stateCount);

private:
    struct State {
        int len;              /**< 最长子串长度 */
        int link;             /**< 后缀链接 */
        QMap<int, int> next;  /**< 转移表 */
        int occurrences;      /**< 出现次数 */
    };

    QVector<State> m_states;
    int m_last;
    int m_length;
    Stats m_stats;
    double m_timeSum;
};
