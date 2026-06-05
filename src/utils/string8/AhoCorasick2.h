/**
 * @file AhoCorasick2.h
 * @brief Aho-Corasick增强多模式匹配 — 支持通配符的多模式搜索引擎
 *
 * 功能: 在标准Aho-Corasick自动机基础上增加通配符('?')支持，
 *       允许模式中包含单字符通配符。通过扩展Trie节点在匹配时
 *       展开通配符分支，保持O(n+m+z)的时间复杂度。
 *
 * 协作: CommentzWalter(替代匹配) / DataPatternDetector(模式检测)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QChar>
#include <QSet>
#include <QElapsedTimer>

/**
 * @brief Aho-Corasick增强版 — 通配符多模式匹配
 */
class AhoCorasick2 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        int    totalSearches = 0;         ///< 累计搜索次数
        int    totalMatches = 0;          ///< 累计匹配总数
        int    totalWildcards = 0;        ///< 累计通配符展开次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit AhoCorasick2(QObject* parent = nullptr);

    /**
     * @brief 构建自动机
     * @param patterns 模式串列表(支持'?'作为单字符通配符)
     */
    void build(const QStringList& patterns);

    /**
     * @brief 搜索文本中所有匹配
     * @param text 待搜索文本
     * @return 匹配列表 (模式索引, 文本起始位置)
     */
    QVector<QPair<int, int>> search(const QString& text) const;

    /**
     * @brief 搜索并返回匹配的模式文本
     * @param text 待搜索文本
     * @return 匹配列表 (模式文本, 文本起始位置)
     */
    QVector<QPair<QString, int>> searchWithText(const QString& text) const;

    /**
     * @brief 检查文本是否包含任意模式
     * @param text 待检测文本
     * @return 第一个匹配位置，无匹配返回-1
     */
    int containsAny(const QString& text) const;

    /**
     * @brief 替换所有匹配为指定字符串
     * @param text 原始文本
     * @param replacement 替换字符串
     * @return 替换后的文本
     */
    QString replaceAll(const QString& text,
                       const QString& replacement) const;

    /** @brief 获取已构建的模式数量 */
    int patternCount() const { return m_patterns.size(); }
    /** @brief 是否已构建 */
    bool isBuilt() const { return m_built; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 搜索完成 @param matchCount 匹配数量 */
    void searchCompleted(int matchCount);

private:
    /**
     * @brief Trie节点(支持通配符)
     */
    struct TrieNode {
        QMap<QChar, int> children;       ///< 子节点映射
        int              fail = 0;       ///< 失败指针
        int              output = -1;    ///< 匹配的模式索引(-1=非终点)
        QVector<int>     outputList;     ///< 输出链表(字典序链接)
        bool             isWildcard = false; ///< 是否为通配符节点
    };

    /** @brief 构建Trie(含通配符展开) */
    void buildTrie(const QStringList& patterns);

    /** @brief 构建失败指针(BFS) */
    void buildFailureLinks();

    /** @brief 递归展开通配符 */
    void expandWildcard(int nodeIdx, const QStringList& patterns,
                        int patIdx, int charPos);

    /** @brief 收集某个节点所有输出 */
    void collectOutputs(int nodeIdx,
                        QVector<QPair<int, int>>& results,
                        int textPos) const;

    QVector<TrieNode> m_trie;       ///< Trie自动机
    QStringList       m_patterns;   ///< 原始模式列表
    bool              m_built = false;

    mutable Stats        m_stats;
    mutable double       m_timeSum = 0.0;
    mutable QElapsedTimer m_timer;
};
