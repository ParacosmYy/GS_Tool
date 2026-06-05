/**
 * @file BeamSearcher.cpp
 * @brief Beam Search束搜索解码器实现
 */

#include "BeamSearcher.h"
#include <QElapsedTimer>
#include <algorithm>

BeamSearcher::BeamSearcher(int beamWidth, QObject* parent)
    : QObject(parent)
    , m_beamWidth(qMax(1, beamWidth))
    , m_timeSum(0.0)
{
}

void BeamSearcher::setBeamWidth(int width)
{
    m_beamWidth = qMax(1, width);
}

BeamSearcher::Result BeamSearcher::search(
    std::function<double(const QVector<int>&, int)> scoreFn,
    int vocabSize,
    int maxSteps,
    int eosToken)
{
    QElapsedTimer timer;
    timer.start();

    Result result;
    result.stepsExpanded = 0;
    result.bestScore = -1e18;

    /* 初始化: 空序列 */
    QVector<Candidate> beams;
    beams.append({{}, 0.0});

    for (int step = 0; step < maxSteps; ++step) {
        QVector<Candidate> allCandidates;

        for (const auto& beam : beams) {
            bool hasEos = !beam.tokens.isEmpty() && beam.tokens.last() == eosToken;
            if (hasEos) {
                /* 已终止的候选直接保留 */
                allCandidates.append(beam);
                continue;
            }

            /* 扩展: 尝试所有词汇 */
            for (int tok = 0; tok < vocabSize; ++tok) {
                QVector<int> newTokens = beam.tokens;
                newTokens.append(tok);
                double score = beam.score + scoreFn(beam.tokens, tok);
                allCandidates.append({newTokens, score});
            }
        }

        /* 保留top-k */
        std::sort(allCandidates.begin(), allCandidates.end(),
                  [](const Candidate& a, const Candidate& b) {
                      return a.score > b.score;
                  });

        if (allCandidates.size() > m_beamWidth)
            allCandidates.resize(m_beamWidth);

        beams = allCandidates;
        result.stepsExpanded++;

        /* 检查是否所有beam都已终止 */
        bool allDone = true;
        for (const auto& b : beams) {
            if (b.tokens.isEmpty() || b.tokens.last() != eosToken) {
                allDone = false;
                break;
            }
        }
        if (allDone) break;
    }

    result.beams = beams;
    if (!beams.isEmpty()) result.bestScore = beams.first().score;

    m_stats.totalSearches++;
    m_stats.totalCandidates += beams.size() * vocabSize * result.stepsExpanded;
    m_stats.avgStepsExpanded =
        (m_stats.avgStepsExpanded * (m_stats.totalSearches - 1) + result.stepsExpanded)
        / m_stats.totalSearches;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;

    emit searchCompleted(beams.size(), result.bestScore);
    return result;
}

BeamSearcher::Stats BeamSearcher::stats() const { return m_stats; }

void BeamSearcher::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
