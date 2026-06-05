/**
 * @file StateTracker.cpp
 * @brief 状态追踪器实现
 */

#include "utils/state/StateTracker.h"

#include <QDateTime>

StateTracker::StateTracker(QObject* parent)
    : QObject(parent), m_maxHistory(10000)
{
    m_stateTimer.start();
}

void StateTracker::addState(const QString& name)
{
    if (!m_stats.stateVisitCounts.contains(name)) {
        m_stats.stateVisitCounts[name] = 0;
        m_stats.stateDwellTime[name] = 0.0;
    }
}

void StateTracker::addTransition(const QString& from, const QString& to,
                                  double minVal, double maxVal)
{
    TransitionRule rule;
    rule.toState = to;
    rule.minVal = minVal;
    rule.maxVal = maxVal;
    m_transitions[from].append(rule);
}

void StateTracker::setInitialState(const QString& state)
{
    m_currentState = state;
    m_lastState = state;
    ++m_stats.stateVisitCounts[state];
    m_stats.uniqueStatesVisited = m_stats.stateVisitCounts.size();
    m_stateTimer.restart();
}

void StateTracker::evaluate(double value)
{
    if (m_currentState.isEmpty()) return;

    if (!m_transitions.contains(m_currentState)) return;

    for (const auto& rule : m_transitions[m_currentState]) {
        if (value >= rule.minVal && value <= rule.maxVal) {
            if (rule.toState == m_currentState) break;

            qint64 elapsed = m_stateTimer.elapsed();
            m_stats.stateDwellTime[m_currentState] += elapsed;

            Transition t;
            t.fromState = m_currentState;
            t.toState = rule.toState;
            t.timestamp = QDateTime::currentMSecsSinceEpoch();
            t.triggerValue = value;
            m_history.append(t);
            while (m_history.size() > m_maxHistory) m_history.removeFirst();

            ++m_stats.totalTransitions;
            ++m_stats.stateVisitCounts[rule.toState];
            m_stats.uniqueStatesVisited = m_stats.stateVisitCounts.size();

            emit stateChanged(m_currentState, rule.toState, value);
            m_currentState = rule.toState;
            m_stateTimer.restart();
            break;
        }
    }
}

QString StateTracker::currentState() const { return m_currentState; }

QList<StateTracker::Transition> StateTracker::transitionHistory() const
{
    return m_history;
}

void StateTracker::clearHistory() { m_history.clear(); }

void StateTracker::resetStatistics()
{
    m_stats = Stats{};
    for (auto it = m_stats.stateVisitCounts.begin();
         it != m_stats.stateVisitCounts.end(); ++it) {
        it.value() = 0;
    }
    m_stateTimer.restart();
}
