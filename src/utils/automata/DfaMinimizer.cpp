/**
 * @file DfaMinimizer.cpp
 * @brief DFA最小化实现
 */

#include "DfaMinimizer.h"
#include <QElapsedTimer>
#include <QQueue>

DfaMinimizer::MinimizedDFA DfaMinimizer::minimize(
    const TransitionTable& transitions,
    const QSet<int>& acceptingStates,
    const QSet<int>& alphabet) const
{
    QElapsedTimer timer;
    timer.start();

    int numStates = transitions.size();
    MinimizedDFA result;

    if (numStates == 0) {
        m_stats.totalMinimizations++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalMinimizations;
        return result;
    }

    /* 初始分区: 接受状态 vs 非接受状态 */
    QMap<int, int> stateToPartition;
    QVector<QSet<int>> partitions;

    QSet<int> accepting, nonAccepting;
    for (int s = 0; s < numStates; ++s) {
        if (acceptingStates.contains(s)) accepting.insert(s);
        else nonAccepting.insert(s);
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

    /* 分区细化 */
    bool changed = true;
    while (changed) {
        changed = false;
        QVector<QSet<int>> newPartitions;

        for (const auto& partition : partitions) {
            QMap<QVector<int>, QSet<int>> splitGroups;

            for (int state : partition) {
                QVector<int> signature;
                for (int symbol : alphabet) {
                    int next = transitions[state].value(symbol, -1);
                    int nextPart = (next >= 0) ? stateToPartition.value(next, -1) : -1;
                    signature.append(nextPart);
                }
                splitGroups[signature].insert(state);
            }

            if (splitGroups.size() > 1) changed = true;

            for (auto it = splitGroups.begin(); it != splitGroups.end(); ++it) {
                int idx = newPartitions.size();
                for (int s : it.value()) stateToPartition[s] = idx;
                newPartitions.append(it.value());
            }
        }

        partitions = newPartitions;
    }

    /* 构建最小化DFA */
    int numMinimized = partitions.size();

    result.transitions.resize(numMinimized);
    for (int i = 0; i < numMinimized; ++i) {
        int representative = *partitions[i].begin();
        for (int symbol : alphabet) {
            int next = transitions[representative].value(symbol, -1);
            if (next >= 0)
                result.transitions[i][symbol] = stateToPartition[next];
        }
    }

    for (int i = 0; i < numMinimized; ++i) {
        if (acceptingStates.contains(*partitions[i].begin()))
            result.acceptingStates.insert(i);
    }

    for (int s = 0; s < numStates; ++s)
        result.stateMapping[s] = stateToPartition[s];

    m_stats.totalMinimizations++;
    m_stats.totalStatesRemoved += (numStates - numMinimized);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalMinimizations;

    emit minimizationCompleted(numStates, numMinimized);
    return result;
}

DfaMinimizer::Stats DfaMinimizer::stats() const { return m_stats; }

void DfaMinimizer::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
