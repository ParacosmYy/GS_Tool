/**
 * @file DfaMinimizer.cpp
 * @brief DFA最小化实现 — Hopcroft分区细化算法
 */

#include "DfaMinimizer.h"

#include <QElapsedTimer>
#include <QQueue>
#include <QStack>
#include <algorithm>

/* ---------- 构造函数 ---------- */

DfaMinimizer::DfaMinimizer(QObject* parent)
    : QObject(parent)
{
}

/* ---------- Hopcroft最小化 ---------- */

DfaMinimizer::MinimizedDFA DfaMinimizer::minimize(
    const TransitionTable& transitions,
    const QSet<int>& acceptingStates,
    const QSet<int>& alphabet) const
{
    QElapsedTimer timer;
    timer.start();
    int refinementSteps = 0;

    MinimizedDFA result;
    int numStates = transitions.size();

    if (numStates == 0) {
        result.originalStateCount = 0;
        result.minimizedStateCount = 0;
        result.reductionRatio = 0.0;
        m_stats.totalMinimizations++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = (m_stats.totalMinimizations > 0)
            ? m_timeSum / m_stats.totalMinimizations : 0.0;
        return result;
    }

    result.originalStateCount = numStates;

    /* Step 1: 先移除不可达状态 */
    auto cleaned = removeUnreachableStates(transitions, acceptingStates, alphabet);
    auto& workTransitions = cleaned.transitions;
    auto& workAccepting = cleaned.acceptingStates;
    auto& cleanMapping = cleaned.stateMapping;
    int workStates = workTransitions.size();

    if (workStates == 0) {
        result.minimizedStateCount = 0;
        result.reductionRatio = 1.0;
        m_stats.totalMinimizations++;
        m_stats.totalStatesRemoved += numStates;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = (m_stats.totalMinimizations > 0)
            ? m_timeSum / m_stats.totalMinimizations : 0.0;
        emit minimizationCompleted(numStates, 0, 1.0);
        return result;
    }

    /* Step 2: Hopcroft分区细化 */
    /* 初始分区: 接受状态 vs 非接受状态 */
    QVector<QSet<int>> partitions;
    QMap<int, int> stateToPartition;

    QSet<int> accepting, nonAccepting;
    for (int s = 0; s < workStates; ++s) {
        if (workAccepting.contains(s)) {
            accepting.insert(s);
        } else {
            nonAccepting.insert(s);
        }
    }

    if (!accepting.isEmpty()) {
        int idx = partitions.size();
        for (int s : accepting) stateToPartition[s] = idx;
        partitions.append(accepting);
    }
    if (!nonAccepting.isEmpty()) {
        int idx = partitions.size();
        for (int s : nonAccepting) stateToPartition[s] = idx;
        partitions.append(nonAccepting);
    }

    /* 构建逆转移表 */
    auto inverse = buildInverseTransitions(workTransitions, alphabet);

    /* 工作表: 待处理的分区(使用索引) */
    QSet<int> worklist;
    for (int i = 0; i < partitions.size(); ++i) {
        worklist.insert(i);
    }

    /* 分区细化主循环 */
    while (!worklist.isEmpty()) {
        /* 取出最小分区(启发式: 小分区优先) */
        int minIdx = *worklist.begin();
        int minSize = partitions[minIdx].size();
        for (int idx : worklist) {
            if (partitions[idx].size() < minSize) {
                minSize = partitions[idx].size();
                minIdx = idx;
            }
        }
        worklist.remove(minIdx);

        QSet<int> splitter = partitions[minIdx];

        /* 对每个符号尝试细化 */
        for (int symbol : alphabet) {
            /* 找到能通过symbol到达splitter中某状态的所有状态 */
            QSet<int> preimage;
            for (int target : splitter) {
                if (inverse.contains(symbol) && inverse[symbol].contains(target)) {
                    for (int src : inverse[symbol][target]) {
                        preimage.insert(src);
                    }
                }
            }

            if (preimage.isEmpty()) continue;

            /* 用preimage细化每个分区 */
            QVector<QSet<int>> newPartitions;
            QMap<int, int> newStateToPart;

            for (int p = 0; p < partitions.size(); ++p) {
                QSet<int> intersect = partitions[p] & preimage;
                QSet<int> diff = partitions[p] - preimage;

                if (intersect.isEmpty() || diff.isEmpty()) {
                    /* 无需分割 */
                    int idx = newPartitions.size();
                    for (int s : partitions[p]) newStateToPart[s] = idx;
                    newPartitions.append(partitions[p]);
                    continue;
                }

                refinementSteps++;

                /* 分割为两个新区 */
                int idx1 = newPartitions.size();
                for (int s : intersect) newStateToPart[s] = idx1;
                newPartitions.append(intersect);

                int idx2 = newPartitions.size();
                for (int s : diff) newStateToPart[s] = idx2;
                newPartitions.append(diff);

                /* 更新工作表 */
                if (worklist.contains(p)) {
                    worklist.remove(p);
                    worklist.insert(idx1);
                    worklist.insert(idx2);
                } else {
                    /* 较小的分区加入工作表 */
                    if (intersect.size() <= diff.size()) {
                        worklist.insert(idx1);
                    } else {
                        worklist.insert(idx2);
                    }
                }
            }

            partitions = newPartitions;
            stateToPartition = newStateToPart;
        }
    }

    /* Step 3: 构建最小化DFA */
    int numMinimized = partitions.size();
    result.transitions.resize(numMinimized);

    for (int i = 0; i < numMinimized; ++i) {
        int representative = *partitions[i].begin();
        for (int symbol : alphabet) {
            int next = workTransitions[representative].value(symbol, -1);
            if (next >= 0) {
                result.transitions[i][symbol] = stateToPartition[next];
            }
        }
    }

    for (int i = 0; i < numMinimized; ++i) {
        if (workAccepting.contains(*partitions[i].begin())) {
            result.acceptingStates.insert(i);
        }
    }

    /* 构建原始状态到最小化状态的映射 */
    for (int s = 0; s < numStates; ++s) {
        if (cleanMapping.contains(s)) {
            int cleanedState = cleanMapping[s];
            result.stateMapping[s] = stateToPartition[cleanedState];
        }
    }

    result.minimizedStateCount = numMinimized;
    result.reductionRatio = (numStates > 0)
        ? 1.0 - static_cast<double>(numMinimized) / numStates : 0.0;

    m_stats.totalMinimizations++;
    m_stats.totalStatesRemoved += (numStates - numMinimized);
    m_stats.totalRefinementSteps += refinementSteps;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalMinimizations > 0)
        ? m_timeSum / m_stats.totalMinimizations : 0.0;

    emit minimizationCompleted(numStates, numMinimized, result.reductionRatio);
    return result;
}

/* ---------- 是否已最小化 ---------- */

bool DfaMinimizer::isAlreadyMinimal(const TransitionTable& transitions,
                                     const QSet<int>& acceptingStates,
                                     const QSet<int>& alphabet) const
{
    auto result = minimize(transitions, acceptingStates, alphabet);
    return result.minimizedStateCount == result.originalStateCount;
}

/* ---------- 查找不可达状态 ---------- */

QSet<int> DfaMinimizer::findUnreachableStates(
    const TransitionTable& transitions,
    const QSet<int>& alphabet) const
{
    int numStates = transitions.size();
    if (numStates == 0) return {};

    /* BFS从状态0出发 */
    QSet<int> reachable;
    QQueue<int> queue;
    queue.enqueue(0);
    reachable.insert(0);

    while (!queue.isEmpty()) {
        int state = queue.dequeue();
        for (int symbol : alphabet) {
            int next = transitions[state].value(symbol, -1);
            if (next >= 0 && !reachable.contains(next)) {
                reachable.insert(next);
                queue.enqueue(next);
            }
        }
    }

    QSet<int> unreachable;
    for (int i = 0; i < numStates; ++i) {
        if (!reachable.contains(i)) {
            unreachable.insert(i);
        }
    }
    return unreachable;
}

/* ---------- 移除不可达状态 ---------- */

DfaMinimizer::CleanedDFA DfaMinimizer::removeUnreachableStates(
    const TransitionTable& transitions,
    const QSet<int>& acceptingStates,
    const QSet<int>& alphabet) const
{
    CleanedDFA result;
    auto unreachable = findUnreachableStates(transitions, alphabet);

    /* 建立旧状态到新状态的映射 */
    QMap<int, int> oldToNew;
    int newIdx = 0;
    for (int i = 0; i < transitions.size(); ++i) {
        if (!unreachable.contains(i)) {
            oldToNew[i] = newIdx++;
        }
    }

    result.transitions.resize(newIdx);
    for (auto it = oldToNew.begin(); it != oldToNew.end(); ++it) {
        int oldState = it.key();
        int newState = it.value();
        result.stateMapping[oldState] = newState;

        for (int symbol : alphabet) {
            int next = transitions[oldState].value(symbol, -1);
            if (next >= 0 && oldToNew.contains(next)) {
                result.transitions[newState][symbol] = oldToNew[next];
            }
        }

        if (acceptingStates.contains(oldState)) {
            result.acceptingStates.insert(newState);
        }
    }

    return result;
}

/* ---------- 统计 ---------- */

DfaMinimizer::Stats DfaMinimizer::stats() const { return m_stats; }

void DfaMinimizer::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/* ---------- 私有: 构建逆转移表 ---------- */

QMap<int, QMap<int, QSet<int>>> DfaMinimizer::buildInverseTransitions(
    const TransitionTable& transitions,
    const QSet<int>& alphabet) const
{
    QMap<int, QMap<int, QSet<int>>> inverse;

    for (int s = 0; s < transitions.size(); ++s) {
        for (int symbol : alphabet) {
            int next = transitions[s].value(symbol, -1);
            if (next >= 0) {
                inverse[symbol][next].insert(s);
            }
        }
    }

    return inverse;
}
