/**
 * @file DamerauLevenshtein.h
 * @brief Damerau-Levenshtein距离 — 含相邻交换的编辑距离
 *
 * 功能: 计算最优字符串对齐距离(含插入/删除/替换/相邻交换)，
 *       支持归一化距离和相似度，统计计算次数/耗时。
 */
#pragma once

#include <QObject>
#include <QString>

class DamerauLevenshtein : public QObject {
    Q_OBJECT
public:
    /** 统计信息 */
    struct Stats {
        quint64 totalComputed = 0;      ///< 总计算次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit DamerauLevenshtein(QObject* parent = nullptr);

    /**
     * @brief 计算Damerau-Levenshtein距离(OSA变体)
     * @param a 字符串a
     * @param b 字符串b
     * @return 编辑距离
     */
    int distance(const QString& a, const QString& b);

    /**
     * @brief 计算归一化距离(0~1)
     * @param a 字符串a
     * @param b 字符串b
     * @return 归一化距离
     */
    double normalizedDistance(const QString& a, const QString& b);

    /**
     * @brief 计算相似度(1 - 归一化距离)
     * @param a 字符串a
     * @param b 字符串b
     * @return 相似度(0~1)
     */
    double similarity(const QString& a, const QString& b);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 计算完成信号 @param distance 计算出的距离 */
    void computed(int distance);

private:
    Stats  m_stats;
    double m_timeSum;
};
