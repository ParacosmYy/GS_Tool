/**
 * @file CommentzWalter.h
 * @brief Commentz-Walter多模式匹配算法 — 高效字符串搜索引擎
 *
 * 功能: 基于Commentz-Walter算法实现多模式串匹配，从右向左
 *       扫描文本，利用坏字符和好后缀跳跃加速搜索。
 *
 * 协作: BytePatternAnalyzer(模式分析) / DataPatternDetector(模式检测)
 */
#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <QPair>
#include <QMap>
#include <QChar>
#include <QElapsedTimer>

/**
 * @brief Commentz-Walter多模式匹配引擎
 */
class CommentzWalter : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSearches = 0;        ///< 累计搜索次数
        quint64 totalMatches = 0;         ///< 累计匹配总数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit CommentzWalter(QObject* parent = nullptr);

    /**
     * @brief 构建匹配自动机
     * @param patterns 模式串列表
     */
    void build(const QVector<QString>& patterns);

    /**
     * @brief 搜索文本中所有匹配
     * @param text 待搜索文本
     * @return 匹配列表(模式索引, 文本位置)
     */
    QVector<QPair<int, int>> search(const QString& text);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 搜索完成 @param matchCount 匹配数量 */
    void searchCompleted(int matchCount);

private:
    /**
     * @brief Trie节点
     */
    struct TrieNode {
        QMap<QChar, int> children;  ///< 子节点映射
        int              fail = 0;  ///< 失败指针
        int              patternIdx = -1;  ///< 匹配的模式索引(-1=非终点)
        int              depth = 0; ///< 节点深度
    };

    /** @brief 构建Trie */
    void buildTrie(const QVector<QString>& patterns);

    /** @brief 计算坏字符跳跃表 */
    void computeBadCharShift(const QVector<QString>& patterns);

    /** @brief 计算最小模式长度 */
    void computeMinPatternLen(const QVector<QString>& patterns);

    QVector<TrieNode> m_trie;          ///< Trie自动机
    QMap<QChar, int>  m_badCharShift;  ///< 坏字符跳跃表
    int               m_minPatLen = 0; ///< 最短模式长度
    bool              m_built = false; ///< 是否已构建

    mutable Stats        m_stats;
    mutable double       m_totalTimeMs = 0.0;
    mutable QElapsedTimer m_timer;
};
