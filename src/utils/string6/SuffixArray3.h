/**
 * @file SuffixArray3.h
 * @brief 后缀数组 — SA-IS线性时间构造算法
 *
 * 功能: 使用SA-IS(Suffix Array Induced Sorting)算法在线性时间内
 *       构造后缀数组, 支持LCP数组构造和高效字符串匹配。
 *
 * 协作: BytePatternAnalyzer(模式分析) / DataPatternDetector(模式检测)
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QPair>

/**
 * @brief SA-IS线性时间后缀数组构造器
 *
 * SA-IS算法由Nong, Zhang, Chan (2009)提出, 是目前实际应用中
 * 最快的后缀数组构造算法之一, 时间复杂度O(n), 空间复杂度O(n)。
 *
 * 后缀数组SA[i]表示第i小的后缀的起始位置。
 * LCP数组LCP[i]表示SA[i]和SA[i-1]的最长公共前缀长度。
 */
class SuffixArray3 : public QObject
{
    Q_OBJECT

public:
    /** @brief 搜索结果 */
    struct SearchResult {
        QVector<int> positions;            ///< 匹配位置列表
        int count = 0;                      ///< 匹配总数
        double matchRatio = 0.0;            ///< 匹配比率(匹配数/文本长度)
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalArraysBuilt = 0;           ///< 累计构造后缀数组次数
        int totalCharactersProcessed = 0;   ///< 累计处理字符数
        int totalSearches = 0;              ///< 累计搜索次数
        int totalMatches = 0;               ///< 累计匹配总数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
    };

    explicit SuffixArray3(QObject* parent = nullptr);

    /**
     * @brief 构造后缀数组(SA-IS算法)
     * @param text 输入文本(整数数组, 要求所有值>=0)
     * @param n 文本长度
     *param alphabetSize 字母表大小(最大值+1)
     * @return 后缀数组SA
     */
    QVector<int> build(const QVector<int>& text, int n, int alphabetSize);

    /**
     * @brief 从字符串构造后缀数组
     * @param text 输入字符串
     * @return 后缀数组SA
     */
    QVector<int> buildFromString(const QString& text);

    /**
     * @brief 从字节数组构造后缀数组
     * @param data 输入字节数组
     * @return 后缀数组SA
     */
    QVector<int> buildFromBytes(const QByteArray& data);

    /**
     * @brief 构造LCP数组(Kasai算法)
     * @param text 原始文本
     * @param sa 后缀数组
     * @return LCP数组(LCP[0]=0)
     */
    QVector<int> buildLCP(const QVector<int>& text,
                          const QVector<int>& sa);

    /**
     * @brief 二分搜索模式串(精确匹配)
     * @param text 原始文本
     * @param sa 后缀数组
     * @param pattern 模式串
     * @return 搜索结果
     */
    SearchResult search(const QVector<int>& text,
                        const QVector<int>& sa,
                        const QVector<int>& pattern);

    /**
     * @brief 搜索字符串模式
     * @param text 原始文本
     * @param sa 后缀数组
     * @param pattern 模式字符串
     * @return 搜索结果
     */
    SearchResult searchString(const QString& text,
                              const QVector<int>& sa,
                              const QString& pattern);

    /**
     * @brief 查找最长重复子串
     * @param lcp LCP数组
     * @param sa 后缀数组
     * @param minLength 最小长度
     * @return 最长重复子串的起始位置和长度
     */
    QVector<QPair<int, int>> longestRepeats(const QVector<int>& lcp,
                                            const QVector<int>& sa,
                                            int minLength = 2);

    /**
     * @brief 统计不同子串数量
     * @param n 文本长度
     * @param lcp LCP数组
     * @return 不同子串数量
     */
    qint64 distinctSubstrings(int n, const QVector<int>& lcp);

    Stats stats() const;
    void resetStatistics();

signals:
    /** @brief 后缀数组构建完成 @param length 文本长度 @param buildTimeMs 构建耗时 */
    void arrayBuilt(int length, double buildTimeMs);

    /** @brief 搜索完成 @param patternLength 模式长度 @param matchCount 匹配数 */
    void searchCompleted(int patternLength, int matchCount);

private:
    /**
     * @brief SA-IS递归核心
     * @param s 输入序列
     * @param sa 输出后缀数组
     * @param n 序列长度
     * @param K 字母表大小
     */
    void saisCore(const QVector<int>& s, QVector<int>& sa,
                  int n, int K);

    /**
     * @brief 诱导排序(Induced Sorting)步骤
     * @param s 输入序列
     * @param sa 后缀数组(被修改)
     * @param n 序列长度
     * @param K 字母表大小
     * @param lmsOffsets LMS后缀位置
     */
    void induceSort(const QVector<int>& s, QVector<int>& sa,
                    int n, int K, const QVector<int>& lmsOffsets);

    /**
     * @brief 分类字符类型(L型/S型)
     * @param s 输入序列
     * @param n 序列长度
     * @return 类型数组(true=S型, false=L型)
     */
    QVector<bool> classifyTypes(const QVector<int>& s, int n);

    /**
     * @brief 查找LMS后缀
     * @param types 类型数组
     * @param n 序列长度
     * @return LMS后缀位置列表
     */
    QVector<int> findLMS(const QVector<bool>& types, int n);

    Stats m_stats;
    double m_timeSum = 0.0;
};
