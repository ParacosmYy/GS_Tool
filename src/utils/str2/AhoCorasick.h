/**
 * @file AhoCorasick.h
 * @brief Aho-Corasick多模式字符串匹配
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QMap>
#include <QQueue>

/**
 * @class AhoCorasick
 * @brief Aho-Corasick自动机 — O(n+m+z)多模式匹配
 *
 * 一次扫描文本同时查找所有模式串的匹配位置。
 * 支持动态添加模式、Trie构建和failure link。
 */
class AhoCorasick : public QObject
{
    Q_OBJECT

public:
    /** @brief 匹配结果 */
    struct Match {
        int position;            /**< 文本中的位置 */
        int patternIndex;        /**< 模式编号 */
        QString pattern;         /**< 匹配的模式串 */
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalSearches = 0;       /**< 总搜索次数 */
        int totalMatches = 0;        /**< 总匹配数 */
        int totalPatterns = 0;       /**< 总模式数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit AhoCorasick(QObject* parent = nullptr);

    /**
     * @brief 添加模式串
     * @param pattern 模式串
     */
    void addPattern(const QString& pattern);

    /** @brief 构建自动机(failure link) */
    void build();

    /**
     * @brief 搜索文本
     * @param text 目标文本
     * @return 匹配列表
     */
    QVector<Match> search(const QString& text) const;

    /** @brief 清空 */
    void clear();

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 搜索完成信号 */
    void searchCompleted(int matchCount);

private:
    struct Node {
        QMap<int, int> children;  /**< 子节点 */
        int fail;                  /**< failure link */
        QVector<int> output;       /**< 匹配的模式索引 */
    };

    QVector<Node> m_nodes;
    QVector<QString> m_patterns;
    bool m_built;

    mutable Stats m_stats;
    mutable double m_timeSum;
};
