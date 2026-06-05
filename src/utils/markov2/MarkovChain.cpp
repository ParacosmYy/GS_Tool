/**
 * @file MarkovChain.cpp
 * @brief 一阶马尔可夫链引擎实现
 */

#include "MarkovChain.h"

#include <QElapsedTimer>
#include <algorithm>

// ═══════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════

MarkovChain::MarkovChain(QObject* parent)
    : QObject(parent)
{
}

MarkovChain::~MarkovChain() = default;

// ═══════════════════════════════════════════════════════════
// 核心操作
// ═══════════════════════════════════════════════════════════

void MarkovChain::addTransition(const QString& from, const QString& to)
{
    QElapsedTimer timer;
    timer.start();

    ensureState(from);
    ensureState(to);

    m_transitions[from][to] += 1;
    m_rowTotals[from] += 1;

    m_stats.totalSteps += 1;
    updateAvgTime(timer.nsecsElapsed() / 1000);

    emit transitionAdded(from, to);
}

QString MarkovChain::predictNext(const QString& current) const
{
    if (!m_transitions.contains(current)) {
        return {};
    }

    const auto& row = m_transitions[current];
    QString bestState;
    quint64 bestCount = 0;

    for (auto it = row.constBegin(); it != row.constEnd(); ++it) {
        if (it.value() > bestCount) {
            bestCount = it.value();
            bestState = it.key();
        }
    }

    return bestState;
}

QMap<QString, double> MarkovChain::transitionProbabilities(
    const QString& state) const
{
    QMap<QString, double> result;

    if (!m_transitions.contains(state)) {
        return result;
    }

    const quint64 total = m_rowTotals.value(state, 0);
    if (total == 0) {
        return result;
    }

    const auto& row = m_transitions[state];
    for (auto it = row.constBegin(); it != row.constEnd(); ++it) {
        result[it.key()] = static_cast<double>(it.value()) /
                           static_cast<double>(total);
    }

    return result;
}

QMap<QString, double> MarkovChain::stationaryDistribution(
    int iterations) const
{
    QMap<QString, double> dist;

    if (m_stateList.isEmpty()) {
        return dist;
    }

    const int n = m_stateList.size();
    const double initVal = 1.0 / static_cast<double>(n);
    for (const auto& s : m_stateList) {
        dist[s] = initVal;
    }

    for (int iter = 0; iter < iterations; ++iter) {
        QMap<QString, double> next;
        for (const auto& s : m_stateList) {
            next[s] = 0.0;
        }

        for (const auto& from : m_stateList) {
            const auto probs = transitionProbabilities(from);
            const double fromProb = dist[from];

            if (probs.isEmpty()) {
                const double spread = fromProb / static_cast<double>(n);
                for (const auto& s : m_stateList) {
                    next[s] += spread;
                }
            } else {
                for (auto it = probs.constBegin(); it != probs.constEnd();
                     ++it) {
                    next[it.key()] += fromProb * it.value();
                }
            }
        }

        dist = next;
    }

    return dist;
}

// ═══════════════════════════════════════════════════════════
// 批量操作
// ═══════════════════════════════════════════════════════════

void MarkovChain::addSequence(const QVector<QString>& states)
{
    if (states.size() < 2) {
        emit error(tr("序列长度不足, 至少需要2个状态"));
        return;
    }

    QElapsedTimer timer;
    timer.start();

    for (int i = 0; i < states.size() - 1; ++i) {
        ensureState(states[i]);
        ensureState(states[i + 1]);
        m_transitions[states[i]][states[i + 1]] += 1;
        m_rowTotals[states[i]] += 1;
        m_stats.totalSteps += 1;
    }

    updateAvgTime(timer.nsecsElapsed() / 1000);
}

void MarkovChain::clear()
{
    m_transitions.clear();
    m_rowTotals.clear();
    m_stateList.clear();
    m_stats = Stats{};
}

// ═══════════════════════════════════════════════════════════
// 查询
// ═══════════════════════════════════════════════════════════

QVector<QString> MarkovChain::states() const
{
    return m_stateList;
}

quint64 MarkovChain::transitionCount(const QString& from) const
{
    return m_rowTotals.value(from, 0);
}

// ═══════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════

MarkovChain::Stats MarkovChain::stats() const
{
    return m_stats;
}

void MarkovChain::resetStatistics()
{
    m_stats = Stats{};
}

// ═══════════════════════════════════════════════════════════
// 内部辅助
// ═══════════════════════════════════════════════════════════

void MarkovChain::ensureState(const QString& state)
{
    if (!m_rowTotals.contains(state)) {
        m_stateList.append(state);
        m_rowTotals[state] = 0;
        m_stats.totalStates = static_cast<quint64>(m_stateList.size());
        emit stateAdded(state);
    }
}

void MarkovChain::updateAvgTime(qint64 elapsedUs)
{
    const auto n = m_stats.totalSteps;
    if (n == 1) {
        m_stats.avgProcessingTime = static_cast<double>(elapsedUs);
    } else {
        m_stats.avgProcessingTime =
            m_stats.avgProcessingTime *
                static_cast<double>(n - 1) / static_cast<double>(n) +
            static_cast<double>(elapsedUs) / static_cast<double>(n);
    }
}
