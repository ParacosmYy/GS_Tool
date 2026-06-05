/**
 * @file BoyerMoore2.h
 * @brief Boyer-Moore-Horspool字符串匹配 — 坏字符表快速搜索
 *
 * 功能:
 *   - Boyer-Moore-Horspool变体: 使用简化坏字符表
 *   - 支持单次/多次匹配查找
 *   - 支持大小写敏感/不敏感模式
 *   - 支持字节数组(QByteArray)搜索
 *   - 预处理阶段构建坏字符跳转表
 */

#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <QByteArray>
#include <QPair>

/**
 * @class BoyerMoore2
 * @brief Boyer-Moore-Horspool字符串匹配引擎
 *
 * 预处理模式串生成坏字符跳转表，匹配时从右向左比较字符。
 * 不匹配时利用跳转表跳过不可能匹配的位置，平均复杂度O(N/M)。
 */
class BoyerMoore2 : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalSearches = 0;          /**< 总搜索次数 */
        int totalMatches = 0;           /**< 总匹配次数 */
        int totalCharacters = 0;        /**< 总扫描字符数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 匹配结果 */
    struct MatchResult {
        int position = -1;              /**< 匹配起始位置 */
        int length = 0;                 /**< 匹配长度 */
        QString matchedText;            /**< 匹配到的文本 */
    };

    /** @brief 构造函数 */
    explicit BoyerMoore2(QObject* parent = nullptr);

    /**
     * @brief 预处理模式串
     * @param pattern 搜索模式串
     * @param caseSensitive 是否大小写敏感
     */
    void setPattern(const QString& pattern, bool caseSensitive = true);

    /**
     * @brief 在文本中查找第一个匹配
     * @param text 待搜索文本
     * @return 匹配结果(未找到时position=-1)
     */
    MatchResult findFirst(const QString& text) const;

    /**
     * @brief 在文本中查找所有匹配
     * @param text 待搜索文本
     * @return 所有匹配结果列表
     */
    QVector<MatchResult> findAll(const QString& text) const;

    /**
     * @brief 统计匹配次数
     * @param text 待搜索文本
     * @return 匹配次数
     */
    int countMatches(const QString& text) const;

    /**
     * @brief 在字节数组中查找第一个匹配
     * @param data 待搜索字节流
     * @return 匹配位置(-1表示未找到)
     */
    int findInBytes(const QByteArray& data) const;

    /**
     * @brief 在字节数组中查找所有匹配位置
     * @param data 待搜索字节流
     * @return 匹配位置列表
     */
    QVector<int> findAllInBytes(const QByteArray& data) const;

    /**
     * @brief 高亮所有匹配(返回标记片段列表)
     * @param text 原始文本
     * @param prefix 匹配前缀标记
     * @param suffix 匹配后缀标记
     * @return 标记后的文本
     */
    QString highlight(const QString& text,
                       const QString& prefix = QStringLiteral("["),
                       const QString& suffix = QStringLiteral("]")) const;

    /** @brief 获取当前模式串 */
    QString pattern() const;

    /** @brief 获取坏字符跳转表(调试用) */
    QVector<int> badCharTable() const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 搜索完成信号 */
    void searchCompleted(int matchCount, int scanLength);

private:
    /** @brief 构建坏字符跳转表(QString) */
    void buildBadCharTable(const QString& pat);

    /** @brief 构建坏字符跳转表(QByteArray) */
    void buildBadCharTableBytes(const QByteArray& pat);

    QString m_pattern;               /**< 模式串 */
    QByteArray m_patternBytes;        /**< 模式串字节 */
    QVector<int> m_badChar;           /**< 坏字符跳转表(Unicode) */
    QVector<int> m_badCharBytes;      /**< 坏字符跳转表(字节) */
    bool m_caseSensitive = true;      /**< 大小写敏感 */
    mutable Stats m_stats;            /**< 统计信息 */
    mutable double m_timeSum = 0.0;   /**< 累计时间 */
};
