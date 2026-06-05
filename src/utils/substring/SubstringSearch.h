/**
 * @file SubstringSearch.h
 * @brief 子串搜索统计引擎(Substring Search Statistics)
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QMap>

/**
 * @class SubstringSearch
 * @brief 子串搜索统计 — 多算法子串搜索+统计分析
 *
 * 集成多种搜索算法并提供搜索统计分析。
 * 适用于文本挖掘、日志分析、数据检测等场景。
 */
class SubstringSearch : public QObject
{
    Q_OBJECT

public:
    /** @brief 搜索统计 */
    struct SearchResult {
        int count;           /**< 匹配次数 */
        QVector<int> positions; /**< 匹配位置列表 */
        double timeMs;       /**< 搜索耗时(ms) */
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalSearches = 0;   /**< 总搜索次数 */
        int totalMatches = 0;    /**< 总匹配数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit SubstringSearch(QObject* parent = nullptr);

    /**
     * @brief KMP搜索
     * @param text 文本
     * @param pattern 模式
     * @return 搜索结果
     */
    SearchResult kmp(const QString& text, const QString& pattern);

    /**
     * @brief Z-algorithm搜索
     * @param text 文本
     * @param pattern 模式
     * @return 搜索结果
     */
    SearchResult zAlgorithm(const QString& text, const QString& pattern);

    /**
     * @brief Rabin-Karp搜索
     * @param text 文本
     * @param pattern 模式
     * @return 搜索结果
     */
    SearchResult rabinKarp(const QString& text, const QString& pattern);

    /**
     * @brief 统计所有长度为k的子串频率
     * @param text 文本
     * @param k 子串长度
     * @return 子串→频率映射
     */
    static QMap<QString, int> kGramFrequency(const QString& text, int k);

    /**
     * @brief 找最长重复子串
     * @param text 文本
     * @param minLength 最小长度
     * @return 最长重复子串及其出现次数
     */
    static QPair<QString, int> longestRepeatedSubstring(const QString& text,
                                                          int minLength = 2);

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 搜索完成信号 */
    void searchCompleted(const QString& algorithm, int matchCount);

private:
    Stats m_stats;
    double m_timeSum;
};
