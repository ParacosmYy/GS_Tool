/**
 * @file LcpArray.h
 * @brief LCP数组 — 最长公共前缀数组(Kasai算法)
 *
 * 功能: 构建后缀数组和LCP数组，查找最长重复子串，
 *       统计构建次数/耗时。
 */
#ifndef LCPARRAY_H
#define LCPARRAY_H

#include <QObject>
#include <QVector>
#include <QString>

class LcpArray : public QObject {
    Q_OBJECT
public:
    /** 操作统计 */
    struct Stats {
        quint64 totalBuilds = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit LcpArray(QObject* parent = nullptr);

    /** @brief 构建LCP数组(Kasai算法) @param text 输入文本 @return LCP数组 */
    QVector<int> build(const QString& text);

    /** @brief 查找最长重复子串 @param text 输入文本 @return 最长重复子串 */
    QString longestRepeatSubstring(const QString& text);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 构建完成 @param length 原始字符串长度 */
    void buildCompleted(int length);

private:
    /** @brief 构建后缀数组 @param text 输入文本 @return 后缀数组SA */
    QVector<int> buildSuffixArray(const QString& text);

    Stats m_stats;
    double m_timeSum;
};

#endif // LCPARRAY_H
