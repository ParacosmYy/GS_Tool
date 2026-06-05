#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief SuffixArray4 - 后缀数组构建与查询
 *
 * SA-IS线性时间构建后缀数组，支持LCP数组构建
 * 和O(m log n)模式搜索，适用于字符串分析。
 */
class SuffixArray4 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalArraysBuilt = 0;
        int totalPatternSearches = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SuffixArray4(QObject* parent = nullptr);

    /** @brief 构建字符串的后缀数组 */
    void build(const QVector<int>& text);

    /** @brief 构建LCP(最长公共前缀)数组 */
    QVector<int> buildLCP() const;

    /** @brief 搜索模式，返回所有出现位置的起始索引 */
    QVector<int> search(const QVector<int>& pattern) const;

    /** @brief 获取后缀数组 */
    QVector<int> suffixArray() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void arrayBuilt(int textLength);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QVector<int> m_sa;
    QVector<int> m_text;
};
