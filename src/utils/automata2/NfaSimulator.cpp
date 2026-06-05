/**
 * @file NfaSimulator.cpp
 * @brief NFA模拟器实现
 */

#include "utils/automata2/NfaSimulator.h"

#include <QElapsedTimer>
#include <QQueue>
#include <algorithm>

NfaSimulator::NfaSimulator(QObject* parent)
    : QObject(parent)
{
}

QSet<int> NfaSimulator::epsilonClosure(const NfaDefinition& nfa,
                                        const QSet<int>& states) const
{
    QSet<int> closure = states;
    QQueue<int> queue;
    for (int s : states) {
        queue.enqueue(s);
    }

    while (!queue.isEmpty()) {
        int state = queue.dequeue();
        if (state < 0 || state >= nfa.transitions.size()) continue;

        /* epsilon用 '\0' 表示 */
        auto it = nfa.transitions[state].find(QChar('\0'));
        if (it == nfa.transitions[state].end()) continue;

        for (int next : it.value()) {
            if (!closure.contains(next)) {
                closure.insert(next);
                queue.enqueue(next);
            }
        }
    }
    return closure;
}

QSet<int> NfaSimulator::move(const NfaDefinition& nfa,
                              const QSet<int>& states,
                              QChar symbol) const
{
    QSet<int> result;
    for (int s : states) {
        if (s < 0 || s >= nfa.transitions.size()) continue;
        auto it = nfa.transitions[s].find(symbol);
        if (it != nfa.transitions[s].end()) {
            result.unite(it.value());
        }
    }
    /* 对结果取epsilon闭包 */
    return epsilonClosure(nfa, result);
}

bool NfaSimulator::simulate(const NfaDefinition& nfa,
                             const QString& input) const
{
    QElapsedTimer timer;
    timer.start();

    /* 从起始状态的epsilon闭包开始 */
    QSet<int> current = epsilonClosure(nfa, {nfa.startState});
    int steps = 0;

    for (int i = 0; i < input.size(); ++i) {
        current = move(nfa, current, input[i]);
        ++steps;
        if (current.isEmpty()) break;
    }

    /* 检查是否到达接受状态 */
    bool accepted = false;
    for (int s : current) {
        if (nfa.acceptStates.contains(s)) {
            accepted = true;
            break;
        }
    }

    m_stats.totalSimulations++;
    m_stats.totalStepsExecuted += steps;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSimulations;

    emit simulationCompleted(accepted, steps);
    return accepted;
}

NfaSimulator::DfaResult NfaSimulator::convertToDfa(
    const NfaDefinition& nfa) const
{
    QElapsedTimer timer;
    timer.start();

    DfaResult dfa;

    /* 初始DFA状态 = NFA起始状态的epsilon闭包 */
    QSet<int> startClosure = epsilonClosure(nfa, {nfa.startState});
    QVector<QSet<int>> dfaStates = {startClosure};
    QMap<QSet<int>, int> stateIndex;
    stateIndex[startClosure] = 0;

    /* 检查初始状态是否为接受 */
    for (int s : startClosure) {
        if (nfa.acceptStates.contains(s)) {
            dfa.acceptStates.insert(0);
            break;
        }
    }

    QQueue<int> workQueue;
    workQueue.enqueue(0);

    while (!workQueue.isEmpty()) {
        int dfaState = workQueue.dequeue();
        const QSet<int>& nfaStates = dfaStates[dfaState];

        for (QChar symbol : nfa.alphabet) {
            if (symbol == QChar('\0')) continue; /* 跳过epsilon */

            QSet<int> moved = move(nfa, nfaStates, symbol);
            if (moved.isEmpty()) continue;

            int targetIdx;
            auto it = stateIndex.find(moved);
            if (it == stateIndex.end()) {
                targetIdx = dfaStates.size();
                dfaStates.append(moved);
                stateIndex[moved] = targetIdx;
                workQueue.enqueue(targetIdx);

                /* 检查新状态是否为接受 */
                for (int s : moved) {
                    if (nfa.acceptStates.contains(s)) {
                        dfa.acceptStates.insert(targetIdx);
                        break;
                    }
                }
            } else {
                targetIdx = it.value();
            }

            /* 确保转移表足够大 */
            if (dfaState >= dfa.transitions.size()) {
                dfa.transitions.resize(dfaState + 1);
            }
            dfa.transitions[dfaState][symbol] = targetIdx;
        }
    }

    dfa.numStates = dfaStates.size();
    dfa.startState = 0;

    m_stats.totalSimulations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSimulations;

    return dfa;
}

QVector<int> NfaSimulator::findAllMatches(const NfaDefinition& nfa,
                                           const QString& input) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> matchPositions;
    QSet<int> current = epsilonClosure(nfa, {nfa.startState});

    for (int i = 0; i < input.size(); ++i) {
        current = move(nfa, current, input[i]);

        for (int s : current) {
            if (nfa.acceptStates.contains(s)) {
                matchPositions.append(i);
                break;
            }
        }
    }

    m_stats.totalSimulations++;
    m_stats.totalStepsExecuted += input.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSimulations;

    return matchPositions;
}

NfaSimulator::Stats NfaSimulator::stats() const
{
    return m_stats;
}

void NfaSimulator::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
