/**
 * @file PatternMatch.h
 * @brief 多模式匹配引擎(Multi-Pattern Matching Engine)
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QString>

/**
 * @class PatternMatch
 * @brief 多模式匹配引擎 — 统一接口的多策略匹配器
 *
 * 集成KMP/Rabin-Karp/Boyer-Moore/暴力匹配四种策略,
 * 根据模式特征自动选择最优算法。
 * 适用于协议解析、日志搜索、数据检测等场景。
 */
class PatternMatch : public QObject
{
    Q_OBJECT

public:
    /** @brief 匹配策略 */
    enum Strategy {
        Auto,       /**< 自动选择 */
        KMP,        /**< KMP算法 */
        RabinKarp,  /**< Rabin-Karp哈希 */
        BoyerMoore, /**< Boyer-Moore */
        BruteForce  /**< 暴力匹配 */
    };
    Q_ENUM(Strategy)

    /** @brief 匹配结果 */
    struct Match {
        int position;      /**< 匹配位置 */
        int patternIndex;  /**< 匹配的模式索引 */
        int length;        /**< 匹配长度 */
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalSearches = 0;   /**< 总搜索次数 */
        int totalMatches = 0;    /**< 总匹配数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit PatternMatch(QObject* parent = nullptr);

    /**
     * @brief 添加搜索模式
     * @param pattern 模式字符串
     */
    void addPattern(const QString& pattern);

    /**
     * @brief 清除所有模式
     */
    void clearPatterns();

    /**
     * @brief 搜索文本中所有模式的出现
     * @param text 目标文本
     * @param strategy 匹配策略
     * @return 所有匹配结果
     */
    QVector<Match> search(const QString& text,
                           Strategy strategy = Auto);

    /**
     * @brief 搜索单个模式
     * @param text 目标文本
     * @param pattern 搜索模式
     * @param strategy 匹配策略
     * @return 匹配位置列表
     */
    QVector<int> searchSingle(const QString& text,
                                const QString& pattern,
                                Strategy strategy = Auto);

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 搜索完成信号 */
    void searchCompleted(int matchCount);

private:
    QVector<int> kmpSearch(const QString& text, const QString& pattern) const;
    QVector<int> rabinKarpSearch(const QString& text, const QString& pattern) const;
    QVector<int> boyerMooreSearch(const QString& text, const QString& pattern) const;
    QVector<int> bruteSearch(const QString& text, const QString& pattern) const;
    Strategy selectStrategy(const QString& pattern) const;

    QVector<QString> m_patterns;
    Stats m_stats;
    double m_timeSum;
};
