/**
 * @file LevenshteinDistance.h
 * @brief Levenshtein距离计算器 — 字符串相似度
 *
 * 功能: 计算编辑距离(插入/删除/替换)，支持自定义操作代价，
 *       统计计算次数/平均距离/耗时。
 */
#ifndef LEVENSHTEINDISTANCE_H
#define LEVENSHTEINDISTANCE_H

#include <QObject>
#include <QString>

class LevenshteinDistance : public QObject {
    Q_OBJECT
public:
    struct Stats {
        quint64 totalComputations = 0;
        quint64 totalDistanceSum = 0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit LevenshteinDistance(QObject* parent = nullptr);

    /** @brief 计算编辑距离 @param s1 字符串1 @param s2 字符串2 @return 距离 */
    int compute(const QString& s1, const QString& s2);

    /** @brief 归一化相似度(0~1) @param s1 字符串1 @param s2 字符串2 @return 相似度 */
    double similarity(const QString& s1, const QString& s2);

    /** @brief 带自定义代价 @param s1 字符串1 @param s2 字符串2 @param insertCost 插入代价 @param deleteCost 删除代价 @param replaceCost 替换代价 @return 距离 */
    int computeWeighted(const QString& s1, const QString& s2,
                        int insertCost, int deleteCost, int replaceCost);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void computationCompleted(int distance);

private:
    Stats m_stats;
    double m_timeSum;
};

#endif // LEVENSHTEINDISTANCE_H
