/**
 * @file MarkovChain.cpp
 * @brief 马尔可夫链实现
 */

#include "utils/markov2/MarkovChain.h"
#include <QElapsedTimer>
#include <QtMath>
#include <random>

MarkovChain::MarkovChain(QObject* parent) : QObject(parent), m_timeSum(0.0) {}

void MarkovChain::train(const QVector<int>& sequence)
{
    if (sequence.size() < 2) return;

    for (int i = 0; i < sequence.size() - 1; ++i) {
        int from = sequence[i];
        int to = sequence[i + 1];
        m_transitions[from][to]++;
        m_stateCounts[from]++;
    }

    /* 确保所有状态都有记录 */
    for (int state : sequence) {
        if (!m_stateCounts.contains(state)) m_stateCounts[state] = 0;
    }
    m_stats.stateCount = m_stateCounts.size();
}

int MarkovChain::predict(int currentState)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_transitions.contains(currentState)) return -1;

    int bestNext = -1;
    int bestCount = 0;
    for (auto it = m_transitions[currentState].begin();
         it != m_transitions[currentState].end(); ++it) {
        if (it.value() > bestCount) { bestCount = it.value(); bestNext = it.key(); }
    }

    double prob = (m_stateCounts[currentState] > 0)
        ? static_cast<double>(bestCount) / m_stateCounts[currentState] : 0.0;

    ++m_stats.totalPredictions;
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalPredictions;

    emit predictionMade(currentState, bestNext, prob);
    return bestNext;
}

QMap<int, double> MarkovChain::predictProba(int currentState)
{
    QMap<int, double> proba;
    if (!m_stateCounts.contains(currentState) || m_stateCounts[currentState] == 0) return proba;

    int total = m_stateCounts[currentState];
    if (m_transitions.contains(currentState)) {
        for (auto it = m_transitions[currentState].begin();
             it != m_transitions[currentState].end(); ++it) {
            proba[it.key()] = static_cast<double>(it.value()) / total;
        }
    }
    return proba;
}

QMap<int, double> MarkovChain::stationaryDistribution() const
{
    /* 幂迭代法求平稳分布 */
    QMap<int, double> pi;
    for (auto it = m_stateCounts.begin(); it != m_stateCounts.end(); ++it)
        pi[it.key()] = 1.0 / m_stateCounts.size();

    for (int iter = 0; iter < 100; ++iter) {
        QMap<int, double> newPi;
        for (auto from = m_stateCounts.begin(); from != m_stateCounts.end(); ++from) {
            if (!m_transitions.contains(from.key())) continue;
            int totalFrom = m_stateCounts[from.key()];
            for (auto to = m_transitions[from.key()].begin();
                 to != m_transitions[from.key()].end(); ++to) {
                double tProb = static_cast<double>(to.value()) / totalFrom;
                newPi[to.key()] += pi[from.key()] * tProb;
            }
        }
        pi = newPi;
    }
    return pi;
}

QVector<int> MarkovChain::generate(int startState, int length)
{
    QVector<int> result;
    result.reserve(length);
    int current = startState;
    std::mt19937 rng(42);

    for (int i = 0; i < length; ++i) {
        result.append(current);
        QMap<int, double> proba = predictProba(current);
        if (proba.isEmpty()) break;

        double r = static_cast<double>(rng()) / rng.max();
        double cumulative = 0.0;
        for (auto it = proba.begin(); it != proba.end(); ++it) {
            cumulative += it.value();
            if (r <= cumulative) { current = it.key(); break; }
        }
    }
    return result;
}

int MarkovChain::stateCount() const { return m_stateCounts.size(); }

double MarkovChain::transitionProb(int from, int to) const
{
    if (!m_stateCounts.contains(from) || m_stateCounts[from] == 0) return 0.0;
    if (!m_transitions.contains(from) || !m_transitions[from].contains(to)) return 0.0;
    return static_cast<double>(m_transitions[from][to]) / m_stateCounts[from];
}

void MarkovChain::reset()
{
    m_transitions.clear();
    m_stateCounts.clear();
}

void MarkovChain::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
