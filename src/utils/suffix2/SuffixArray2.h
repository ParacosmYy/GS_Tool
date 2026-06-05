/**
 * @file SuffixArray2.h
 * @brief 后缀数组(简化SA-IS) — 高效字符串索引
 *
 * 功能: 基于SA-IS简化算法构建后缀数组，支持LCP数组计算
 *       和子串搜索，统计构建/搜索次数/耗时。
 */
#pragma once

#include <QObject>
#include <QString>
#include <QVector>

class SuffixArray2 : public QObject {
    Q_OBJECT
public:
    /** 统计信息 */
    struct Stats {
        quint64 totalBuilt = 0;         ///< 总构建次数
        quint64 totalSearches = 0;      ///< 总搜索次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit SuffixArray2(QObject* parent = nullptr);

    /**
     * @brief 构建后缀数组(简化SA-IS)
     * @param text 输入文本
     * @return 后缀数组(排序后的起始位置)
     */
    QVector<int> build(const QString& text);

    /**
     * @brief 获取最近一次构建的LCP数组
     * @return LCP数组(最长公共前缀)
     */
    QVector<int> lcpArray() const;

    /**
     * @brief 在文本中搜索模式串
     * @param pattern 搜索模式
     * @param text 目标文本
     * @return 匹配起始位置列表
     */
    QVector<int> search(const QString& pattern, const QString& text);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 构建完成信号 @param n 文本长度 */
    void buildCompleted(int n);

private:
    /** @brief 构建LCP数组(Kasai算法) */
    QVector<int> buildLcpImpl(const QVector<int>& sa, const QString& text) const;

    /** @brief 简化SA-IS构建 */
    QVector<int> saisBuild(const QVector<int>& input, int alphabetSize) const;

    /** @brief 基数排序辅助 */
    void radixSort(QVector<int>& data, const QVector<int>& keys,
                   int alphabetSize) const;

    Stats  m_stats;
    double m_timeSum;
    QVector<int> m_lastLcp; ///< 最近一次构建的LCP数组
};
