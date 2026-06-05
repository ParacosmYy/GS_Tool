/**
 * @file SuffixAutomaton.h
 * @brief 后缀自动机 — 字符串所有不同子串
 *
 * 功能: 构建SAM，统计不同子串数，子串存在性检查，
 *       两字符串最长公共子串，统计构建/查询次数/耗时。
 */
#ifndef SUFFIXAUTOMATON_H
#define SUFFIXAUTOMATON_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QHash>

class SuffixAutomaton : public QObject {
    Q_OBJECT
public:
    /** 操作统计 */
    struct Stats {
        quint64 totalBuilds = 0;
        quint64 totalQueries = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit SuffixAutomaton(QObject* parent = nullptr);

    /** @brief 从文本构建后缀自动机 @param text 输入文本 */
    void build(const QString& text);

    /** @brief 统计不同子串数量 @return 不同子串总数 */
    qint64 countDistinctSubstrings();

    /** @brief 检查子串是否存在 @param substring 待查子串 @return 是否存在 */
    bool contains(const QString& substring);

    /** @brief 最长公共子串 @param text2 第二个字符串 @return LCP字符串 */
    QString longestCommonSubstring(const QString& text2);

    /** @brief 自动机状态数 */
    int stateCount() const { return m_states.size(); }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 自动机构建完成 @param stateCount 状态数量 */
    void automatonBuilt(int stateCount);

private:
    /** SAM状态 */
    struct State {
        int len = 0;                          ///< 该状态代表的最长子串长度
        int link = -1;                        ///< 后缀链接
        QHash<QChar, int> next;               ///< 转移函数
        qint64 distinctCount = -1;            ///< 缓存的不同子串数(-1=未计算)
    };

    void extend(QChar c);
    qint64 calcDistinct(int v);

    QVector<State> m_states;  ///< 所有状态
    int m_last;               ///< 上一个状态
    Stats m_stats;
    double m_timeSum;
};

#endif // SUFFIXAUTOMATON_H
