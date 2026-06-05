/**
 * @file SuffixArray.h
 * @brief 后缀数组 — 高效字符串匹配与LCP
 *
 * 功能: 构建后缀数组，支持最长公共前缀(LCP)数组、
 *       二分搜索子串、最长重复子串，统计构建/查询/耗时。
 */
#ifndef SUFFIXARRAY_H
#define SUFFIXARRAY_H

#include <QObject>
#include <QVector>
#include <QByteArray>

class SuffixArray : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalBuilds = 0;
        quint64 totalQueries = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit SuffixArray(QObject* parent = nullptr);

    /** @brief 构建后缀数组 @param data 输入数据 @return 后缀数组(排序后的起始位置) */
    QVector<int> build(const QByteArray& data);

    /** @brief 构建LCP数组 @param data 输入数据 @param sa 后缀数组 @return LCP数组 */
    QVector<int> buildLcp(const QByteArray& data, const QVector<int>& sa);

    /** @brief 查找子串(二分搜索) @param data 原始数据 @param sa 后缀数组 @param pattern 搜索模式 @return 匹配起始位置列表 */
    QVector<int> search(const QByteArray& data, const QVector<int>& sa,
                        const QByteArray& pattern);

    /** @brief 最长重复子串 @param data 原始数据 @param sa 后缀数组 @param lcp LCP数组 @return 最长重复子串 */
    QByteArray longestRepeatedSubstring(const QByteArray& data,
                                        const QVector<int>& sa,
                                        const QVector<int>& lcp);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void buildCompleted(int textLength);
    void queryCompleted(int matchCount);

private:
    Stats m_stats;
    double m_timeSum;
};

#endif // SUFFIXARRAY_H
