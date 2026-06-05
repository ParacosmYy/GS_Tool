/**
 * @file VotingCompositor.cpp
 * @brief 投票组合器实现
 */

#include "utils/votecomp/VotingCompositor.h"
#include <QElapsedTimer>
#include <QtMath>

VotingCompositor::VotingCompositor(QObject* parent)
    : QObject(parent), m_strategy(Strategy::Majority), m_timeSum(0.0) {}

void VotingCompositor::setStrategy(Strategy s) { m_strategy = s; }
void VotingCompositor::setModelWeight(int modelId, double weight) { m_weights[modelId] = weight; }

int VotingCompositor::vote(const QVector<int>& predictions)
{
    QElapsedTimer timer;
    timer.start();

    if (predictions.isEmpty()) return -1;

    QMap<int, double> counts;
    for (int i = 0; i < predictions.size(); ++i) {
        double w = m_weights.value(i, 1.0);
        switch (m_strategy) {
        case Strategy::Majority:
            counts[predictions[i]] += 1.0;
            break;
        case Strategy::Weighted:
            counts[predictions[i]] += w;
            break;
        case Strategy::SoftVoting:
            counts[predictions[i]] += 1.0;
            break;
        }
    }

    int bestPred = -1;
    double bestCount = -1.0;
    for (auto it = counts.begin(); it != counts.end(); ++it) {
        if (it.value() > bestCount) { bestCount = it.value(); bestPred = it.key(); }
    }

    /* 计算一致性 */
    double totalVotes = predictions.size();
    double agreement = bestCount / totalVotes;

    ++m_stats.totalVotes;
    double agreeSum = m_stats.avgAgreement * (m_stats.totalVotes - 1) + agreement;
    m_stats.avgAgreement = agreeSum / m_stats.totalVotes;
    if (agreement >= 1.0) ++m_stats.unanimousCount;

    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalVotes;

    emit voteComplete(bestPred, agreement);
    return bestPred;
}

int VotingCompositor::softVote(const QVector<QMap<int, double>>& probaDistributions)
{
    QElapsedTimer timer;
    timer.start();

    if (probaDistributions.isEmpty()) return -1;

    /* 累加概率 */
    QMap<int, double> avgProba;
    for (int i = 0; i < probaDistributions.size(); ++i) {
        double w = m_weights.value(i, 1.0);
        for (auto it = probaDistributions[i].begin(); it != probaDistributions[i].end(); ++it) {
            avgProba[it.key()] += it.value() * w;
        }
    }

    /* 归一化 */
    double totalW = 0.0;
    for (int i = 0; i < probaDistributions.size(); ++i)
        totalW += m_weights.value(i, 1.0);
    for (auto it = avgProba.begin(); it != avgProba.end(); ++it)
        it.value() /= totalW;

    int bestClass = -1;
    double bestProba = -1.0;
    for (auto it = avgProba.begin(); it != avgProba.end(); ++it) {
        if (it.value() > bestProba) { bestProba = it.value(); bestClass = it.key(); }
    }

    ++m_stats.totalVotes;
    double agreeSum = m_stats.avgAgreement * (m_stats.totalVotes - 1) + bestProba;
    m_stats.avgAgreement = agreeSum / m_stats.totalVotes;

    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalVotes;

    emit voteComplete(bestClass, bestProba);
    return bestClass;
}

void VotingCompositor::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
