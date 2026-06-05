/**
 * @file LevenshteinAutomaton.h
 * @brief Levenshtein自动机 — 模糊字符串搜索
 *
 * 基于编辑距离的Levenshtein自动机, 支持有限状态自动机构建、
 * 状态转移、以及基于自动机的高效模糊匹配。
 * 用于拼写检查、模糊搜索、近似字符串匹配等场景。
 */
#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <QSet>
#include <QMap>
#include <QPair>

/**
 * @class LevenshteinAutomaton
 * @brief Levenshtein自动机 — 基于编辑距离的模糊匹配
 *
 * 为目标字符串和最大编辑距离构建NFA, 然后对候选字符串
 * 进行在线匹配, 或转为DFA进行高效批量匹配。
 */
class LevenshteinAutomaton : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalMatches = 0;      ///< 总匹配次数
        quint64 totalCandidates = 0;   ///< 总候选字符串数
        double  avgProcessingTimeMs = 0.0; ///< 平均耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit LevenshteinAutomaton(QObject* parent = nullptr);

    /**
     * @brief 计算两个字符串的Levenshtein编辑距离
     * @param s1 第一个字符串
     * @param s2 第二个字符串
     * @return 编辑距离(插入/删除/替换各计1)
     */
    int editDistance(const QString& s1, const QString& s2) const;

    /**
     * @brief 在线模糊匹配: 检查候选字符串是否在编辑距离内
     * @param target 目标字符串
     * @param candidate 候选字符串
     * @param maxDistance 最大编辑距离
     * @return true表示候选字符串匹配
     */
    bool fuzzyMatch(const QString& target,
                    const QString& candidate,
                    int maxDistance) const;

    /**
     * @brief 批量模糊搜索
     * @param target 目标字符串
     * @param candidates 候选字符串列表
     * @param maxDistance 最大编辑距离
     * @return 匹配的候选字符串及其编辑距离
     */
    QVector<QPair<QString, int>> fuzzySearch(
        const QString& target,
        const QVector<QString>& candidates,
        int maxDistance) const;

    /**
     * @brief 获取所有编辑距离<=maxDist的邻居
     * @param word 输入词
     * @param maxDistance 最大距离
     * @param alphabet 字母表(用于生成变体)
     * @return 所有可达变体
     */
    QSet<QString> generateNeighbors(const QString& word,
                                     int maxDistance,
                                     const QString& alphabet = "abcdefghijklmnopqrstuvwxyz") const;

    /**
     * @brief 计算归一化相似度 [0,1]
     * @param s1 第一个字符串
     * @param s2 第二个字符串
     * @return 相似度(1=完全相同, 0=完全不同)
     */
    double similarity(const QString& s1, const QString& s2) const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 匹配完成 @param target 目标 @param matches 匹配数 */
    void matchCompleted(const QString& target, int matches);

private:
    /** @brief NFA状态: (位置, 编辑数) */
    using NfaState = QPair<int, int>;

    /** @brief 构建Levenshtein NFA状态集 */
    QSet<NfaState> buildNfaStates(const QString& target,
                                   int maxDistance) const;

    /** @brief NFA状态转移 */
    QSet<NfaState> nfaStep(const QSet<NfaState>& current,
                            const QString& target,
                            QChar symbol,
                            int maxDistance) const;

    mutable Stats m_stats;     ///< 操作统计
    mutable double m_timeSum = 0.0; ///< 累计耗时
};
