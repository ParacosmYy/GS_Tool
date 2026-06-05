/**
 * @file ViterbiDecoder.cpp
 * @brief Viterbi算法解码器实现 — 网格构造 + ACS + 回溯
 */

#include "utils/signal16/ViterbiDecoder.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ========== 构造 ========== */

ViterbiDecoder::ViterbiDecoder(int constraintLength,
                                const QVector<int>& generators,
                                QObject* parent)
    : QObject(parent),
      m_constraintLength(qBound(3, constraintLength, 9)),
      m_generators(generators.isEmpty() ? QVector<int>{0171, 0133} : generators),
      m_lastDepth(0), m_timeSum(0.0)
{
    m_numStates = 1 << (m_constraintLength - 1);
    m_rate = m_generators.size();
}

/* ========== 编码 ========== */

QVector<int> ViterbiDecoder::encode(const QVector<int>& input) const
{
    QVector<int> output;
    output.reserve(input.size() * m_rate);

    int shiftReg = 0;
    for (int bit : input) {
        shiftReg = ((shiftReg << 1) | bit) & (m_numStates - 1);
        for (int gen : m_generators) {
            int outBit = 0;
            for (int k = 0; k < m_constraintLength; ++k) {
                if (gen & (1 << k)) outBit ^= ((shiftReg >> k) & 1);
            }
            output.append(outBit);
        }
    }
    return output;
}

/* ========== 硬判决网格构造(ACS) ========== */

QVector<QVector<ViterbiDecoder::TrellisNode>> ViterbiDecoder::buildTrellisHard(
    const QVector<int>& received, int depth) const
{
    QVector<QVector<TrellisNode>> trellis(
        depth + 1, QVector<TrellisNode>(m_numStates, {-1, 0,
            std::numeric_limits<double>::infinity()}));
    trellis[0][0].pathMetric = 0.0;

    for (int t = 0; t < depth; ++t) {
        /* 提取当前时刻的接收符号 */
        QVector<int> recv(m_rate);
        for (int r = 0; r < m_rate; ++r) {
            int idx = t * m_rate + r;
            recv[r] = (idx < received.size()) ? received[idx] : 0;
        }

        for (int state = 0; state < m_numStates; ++state) {
            if (trellis[t][state].pathMetric >=
                std::numeric_limits<double>::infinity() / 2)
                continue;

            for (int input = 0; input <= 1; ++input) {
                int nextState = ((state << 1) | input) & (m_numStates - 1);
                int cost = branchMetricHard(state, input, recv);
                double newMetric = trellis[t][state].pathMetric + cost;

                if (newMetric < trellis[t + 1][nextState].pathMetric) {
                    trellis[t + 1][nextState].pathMetric = newMetric;
                    trellis[t + 1][nextState].prevState = state;
                    trellis[t + 1][nextState].inputBit = input;
                }
            }
        }
    }
    return trellis;
}

/* ========== 软判决网格构造(ACS) ========== */

QVector<QVector<ViterbiDecoder::TrellisNode>> ViterbiDecoder::buildTrellisSoft(
    const QVector<double>& softReceived, int depth) const
{
    QVector<QVector<TrellisNode>> trellis(
        depth + 1, QVector<TrellisNode>(m_numStates, {-1, 0,
            std::numeric_limits<double>::infinity()}));
    trellis[0][0].pathMetric = 0.0;

    for (int t = 0; t < depth; ++t) {
        QVector<double> recv(m_rate);
        for (int r = 0; r < m_rate; ++r) {
            int idx = t * m_rate + r;
            recv[r] = (idx < softReceived.size()) ? softReceived[idx] : 0.0;
        }

        for (int state = 0; state < m_numStates; ++state) {
            if (trellis[t][state].pathMetric >=
                std::numeric_limits<double>::infinity() / 2)
                continue;

            for (int input = 0; input <= 1; ++input) {
                int nextState = ((state << 1) | input) & (m_numStates - 1);
                double cost = branchMetricSoft(state, input, recv);
                double newMetric = trellis[t][state].pathMetric + cost;

                if (newMetric < trellis[t + 1][nextState].pathMetric) {
                    trellis[t + 1][nextState].pathMetric = newMetric;
                    trellis[t + 1][nextState].prevState = state;
                    trellis[t + 1][nextState].inputBit = input;
                }
            }
        }
    }
    return trellis;
}

/* ========== 硬判决解码 ========== */

QVector<int> ViterbiDecoder::decodeHard(const QVector<int>& received)
{
    QElapsedTimer timer;
    timer.start();

    int T = received.size() / m_rate;
    if (T <= 0) return QVector<int>();

    m_lastDepth = T;
    auto trellis = buildTrellisHard(received, T);

    /* 找最佳终止状态 */
    int finalState = 0;
    double bestMetric = trellis[T][0].pathMetric;
    for (int s = 1; s < m_numStates; ++s) {
        if (trellis[T][s].pathMetric < bestMetric) {
            bestMetric = trellis[T][s].pathMetric;
            finalState = s;
        }
    }

    /* 回溯 */
    QVector<int> decoded = traceback(trellis, T);
    int corrections = static_cast<int>(bestMetric);

    /* 统计更新 */
    ++m_stats.totalDecodings;
    ++m_stats.totalTracebacks;
    m_stats.totalBitsProcessed += T;
    m_stats.totalBitsCorrected += corrections;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecodings;

    emit decodingCompleted(T, decoded.size(), corrections);
    return decoded;
}

/* ========== 软判决解码 ========== */

QVector<int> ViterbiDecoder::decodeSoft(const QVector<double>& softReceived)
{
    QElapsedTimer timer;
    timer.start();

    int T = softReceived.size() / m_rate;
    if (T <= 0) return QVector<int>();

    m_lastDepth = T;
    auto trellis = buildTrellisSoft(softReceived, T);

    /* 找最佳终止状态 */
    int finalState = 0;
    double bestMetric = trellis[T][0].pathMetric;
    for (int s = 1; s < m_numStates; ++s) {
        if (trellis[T][s].pathMetric < bestMetric) {
            bestMetric = trellis[T][s].pathMetric;
            finalState = s;
        }
    }

    /* 回溯 */
    QVector<int> decoded = traceback(trellis, T);

    /* 统计更新 */
    ++m_stats.totalDecodings;
    ++m_stats.totalTracebacks;
    m_stats.totalBitsProcessed += T;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecodings;

    emit decodingCompleted(T, decoded.size(), 0);
    return decoded;
}

/* ========== 回溯 ========== */

QVector<int> ViterbiDecoder::traceback(
    const QVector<QVector<TrellisNode>>& trellis, int depth) const
{
    QVector<int> decoded(depth);

    /* 找最小路径度量的最终状态 */
    int state = 0;
    double bestMetric = trellis[depth][0].pathMetric;
    for (int s = 1; s < m_numStates; ++s) {
        if (trellis[depth][s].pathMetric < bestMetric) {
            bestMetric = trellis[depth][s].pathMetric;
            state = s;
        }
    }

    /* 从尾部向头部回溯 */
    for (int t = depth; t > 0; --t) {
        decoded[t - 1] = trellis[t][state].inputBit;
        state = trellis[t][state].prevState;
    }
    return decoded;
}

/* ========== 分支度量(硬) ========== */

int ViterbiDecoder::branchMetricHard(int state, int input,
                                      const QVector<int>& recv) const
{
    QVector<int> expected = getEncoderOutput(state, input);
    int cost = 0;
    for (int i = 0; i < m_rate; ++i) {
        if (i < recv.size() && expected[i] != recv[i]) ++cost;
    }
    return cost;
}

/* ========== 分支度量(软) ========== */

double ViterbiDecoder::branchMetricSoft(int state, int input,
                                          const QVector<double>& recv) const
{
    QVector<int> expected = getEncoderOutput(state, input);
    double cost = 0.0;
    for (int i = 0; i < m_rate; ++i) {
        double ref = (i < expected.size() && expected[i] == 1) ? 1.0 : -1.0;
        double r = (i < recv.size()) ? recv[i] : 0.0;
        cost += (ref - r) * (ref - r);
    }
    return cost;
}

/* ========== 编码器输出 ========== */

QVector<int> ViterbiDecoder::getEncoderOutput(int state, int input) const
{
    int shiftReg = ((state << 1) | input) & (m_numStates - 1);
    QVector<int> output(m_rate);
    for (int g = 0; g < m_rate; ++g) {
        int outBit = 0;
        for (int k = 0; k < m_constraintLength; ++k) {
            if (m_generators[g] & (1 << k))
                outBit ^= ((shiftReg >> k) & 1);
        }
        output[g] = outBit;
    }
    return output;
}

/* ========== 重置统计 ========== */

void ViterbiDecoder::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
