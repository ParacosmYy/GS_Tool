/**
 * @file NfaSimulator.h
 * @brief NFA模拟器 — 支持epsilon闭包的非确定性有限自动机
 *
 * 支持带epsilon转移的NFA模拟, 包含epsilon闭包计算、
 * 多状态并行匹配、以及将NFA转换为DFA(子集构造法)。
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QMap>
#include <QSet>
#include <QChar>

/**
 * @class NfaSimulator
 * @brief NFA模拟器 — epsilon闭包+多状态并行匹配
 *
 * NFA定义: 状态集合、字母表、转移函数(含epsilon)、起始状态、接受状态。
 * 支持: epsilon闭包、单步转移、字符串匹配、子集构造转DFA。
 */
class NfaSimulator : public QObject
{
    Q_OBJECT

public:
    /** @brief NFA转移表: transitions[state][symbol] = {目标状态集合} */
    using TransitionMap = QMap<QChar, QSet<int>>;
    using NfaTransitions = QVector<TransitionMap>;

    /** @brief NFA定义结构 */
    struct NfaDefinition {
        int numStates = 0;            ///< 状态总数
        QSet<QChar> alphabet;         ///< 字母表
        NfaTransitions transitions;   ///< 转移表
        int startState = 0;           ///< 起始状态
        QSet<int> acceptStates;       ///< 接受状态集合
    };

    /** @brief DFA转换结果(子集构造) */
    struct DfaResult {
        int numStates = 0;                   ///< DFA状态数
        QVector<QMap<QChar, int>> transitions; ///< DFA转移表
        QSet<int> acceptStates;              ///< DFA接受状态
        int startState = 0;                  ///< DFA起始状态
    };

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalSimulations = 0;    ///< 总模拟次数
        quint64 totalStepsExecuted = 0;  ///< 总执行步数
        double  avgProcessingTimeMs = 0.0; ///< 平均耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit NfaSimulator(QObject* parent = nullptr);

    /**
     * @brief 计算epsilon闭包
     * @param nfa NFA定义
     * @param states 输入状态集合
     * @return 经epsilon可达的状态集合
     */
    QSet<int> epsilonClosure(const NfaDefinition& nfa,
                              const QSet<int>& states) const;

    /**
     * @brief 单步转移(含epsilon闭包)
     * @param nfa NFA定义
     * @param states 当前状态集合
     * @param symbol 输入符号
     * @return 转移+epsilon闭包后的状态集合
     */
    QSet<int> move(const NfaDefinition& nfa,
                   const QSet<int>& states,
                   QChar symbol) const;

    /**
     * @brief 模拟NFA匹配字符串
     * @param nfa NFA定义
     * @param input 输入字符串
     * @return 是否匹配(到达接受状态)
     */
    bool simulate(const NfaDefinition& nfa,
                  const QString& input) const;

    /**
     * @brief 子集构造法: NFA转DFA
     * @param nfa NFA定义
     * @return 等价DFA
     */
    DfaResult convertToDfa(const NfaDefinition& nfa) const;

    /**
     * @brief 查找所有匹配位置
     * @param nfa NFA定义
     * @param input 输入字符串
     * @return 匹配的结束位置列表
     */
    QVector<int> findAllMatches(const NfaDefinition& nfa,
                                const QString& input) const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 模拟完成 @param accepted 是否匹配 @param steps 总步数 */
    void simulationCompleted(bool accepted, int steps);

private:
    mutable Stats m_stats;     ///< 操作统计
    mutable double m_timeSum = 0.0; ///< 累计耗时(ms)
};
