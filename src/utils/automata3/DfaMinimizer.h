/**
 * @file DfaMinimizer.h
 * @brief DFA最小化 — Hopcroft分区细化算法
 *
 * 功能:
 *   - Hopcroft算法: O(n log n)复杂度的DFA最小化
 *   - 支持自定义字母表和转移表
 *   - 环检测与无效状态识别
 *   - 最小化前后状态映射
 *   - 统计最小化次数、状态减少数、处理时间
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QMap>
#include <QSet>
#include <QPair>

/**
 * @class DfaMinimizer
 * @brief DFA最小化引擎 — Hopcroft分区细化算法
 *
 * Hopcroft算法通过维护分区集合和待细化工作表，
 * 反复细化分区直到不变。时间复杂度O(|Sigma| * |Q| * log |Q|)，
 * 是理论最优的DFA最小化算法。
 */
class DfaMinimizer : public QObject
{
    Q_OBJECT

public:
    /** @brief DFA转移表: transitions[state][symbol] = nextState */
    using TransitionTable = QVector<QMap<int, int>>;

    /** @brief 统计信息 */
    struct Stats {
        int totalMinimizations = 0;     /**< 总最小化次数 */
        int totalStatesRemoved = 0;     /**< 总移除状态数 */
        int totalRefinementSteps = 0;   /**< 总分区细化步骤数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 最小化结果 */
    struct MinimizedDFA {
        TransitionTable transitions;     /**< 最小化后的转移表 */
        QSet<int> acceptingStates;       /**< 最小化后的接受状态 */
        QMap<int, int> stateMapping;     /**< 原始状态 → 最小化状态 */
        int originalStateCount = 0;      /**< 原始状态数 */
        int minimizedStateCount = 0;     /**< 最小化后状态数 */
        double reductionRatio = 0.0;     /**< 状态减少比例 */
    };

    /** @brief 构造函数 */
    explicit DfaMinimizer(QObject* parent = nullptr);

    /**
     * @brief Hopcroft算法最小化DFA
     * @param transitions 转移表
     * @param acceptingStates 接受状态集合
     * @param alphabet 字母表(符号集合)
     * @return 最小化DFA
     */
    MinimizedDFA minimize(const TransitionTable& transitions,
                           const QSet<int>& acceptingStates,
                           const QSet<int>& alphabet) const;

    /**
     * @brief 检查DFA是否已经最小化
     * @param transitions 转移表
     * @param acceptingStates 接受状态集合
     * @param alphabet 字母表
     * @return 是否已最小化
     */
    bool isAlreadyMinimal(const TransitionTable& transitions,
                           const QSet<int>& acceptingStates,
                           const QSet<int>& alphabet) const;

    /**
     * @brief 查找不可达状态(从状态0出发)
     * @param transitions 转移表
     * @param alphabet 字母表
     * @return 不可达状态集合
     */
    QSet<int> findUnreachableStates(const TransitionTable& transitions,
                                     const QSet<int>& alphabet) const;

    /**
     * @brief 移除不可达状态后的DFA
     * @param transitions 转移表
     * @param acceptingStates 接受状态
     * @param alphabet 字母表
     * @return 清理后的(转移表, 接受状态, 状态映射)
     */
    struct CleanedDFA {
        TransitionTable transitions;
        QSet<int> acceptingStates;
        QMap<int, int> stateMapping;
    };

    CleanedDFA removeUnreachableStates(const TransitionTable& transitions,
                                        const QSet<int>& acceptingStates,
                                        const QSet<int>& alphabet) const;

    /** @brief 获取统计信息 */
    Stats stats() const;

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 最小化完成信号 */
    void minimizationCompleted(int originalStates, int minimizedStates,
                               double reductionRatio);

private:
    /** @brief 构建逆转移表: preimage[symbol][targetState] = {sourceStates} */
    QMap<int, QMap<int, QSet<int>>> buildInverseTransitions(
        const TransitionTable& transitions,
        const QSet<int>& alphabet) const;

    /** @brief 分区细化一步: 用splitter细化所有分区 */
    QVector<QSet<int>> refinePartition(
        const QVector<QSet<int>>& partitions,
        const QSet<int>& splitter,
        int symbol,
        const QMap<int, QMap<int, QSet<int>>>& inverse) const;

    mutable Stats m_stats;       /**< 统计信息 */
    mutable double m_timeSum = 0.0; /**< 累计时间 */
};
