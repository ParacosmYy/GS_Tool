/**
 * @file DfaMinimizer.h
 * @brief DFA最小化(Hopcroft算法)
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QMap>
#include <QSet>

/**
 * @class DfaMinimizer
 * @brief DFA最小化 — 将DFA状态数降到最少，保持语言不变
 *
 * 基于Hopcroft分区细化算法，O(n log n)复杂度。
 * 用于正则引擎优化、词法分析器生成等。
 */
class DfaMinimizer : public QObject
{
    Q_OBJECT

public:
    /** @brief DFA转移表: transitions[state][symbol] = nextState */
    using TransitionTable = QVector<QMap<int, int>>;

    /** @brief 统计信息 */
    struct Stats {
        int totalMinimizations = 0; /**< 总最小化次数 */
        int totalStatesRemoved = 0; /**< 总移除状态数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit DfaMinimizer(QObject* parent = nullptr);

    /**
     * @brief 最小化DFA
     * @param transitions 转移表
     * @param acceptingStates 接受状态集合
     * @param alphabet 字母表(符号集合)
     * @return 最小化后的(转移表, 接受状态, 状态映射)
     */
    struct MinimizedDFA {
        TransitionTable transitions;
        QSet<int> acceptingStates;
        QMap<int, int> stateMapping; /**< 原始状态 → 最小化状态 */
    };

    MinimizedDFA minimize(const TransitionTable& transitions,
                           const QSet<int>& acceptingStates,
                           const QSet<int>& alphabet) const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 最小化完成信号 */
    void minimizationCompleted(int originalStates, int minimizedStates);

private:
    mutable Stats m_stats;
    mutable double m_timeSum;
};
