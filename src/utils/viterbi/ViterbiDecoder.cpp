/**
 * @file ViterbiDecoder.cpp
 * @brief 维特比解码器实现 — 卷积码网格解码
 */

#include "utils/viterbi/ViterbiDecoder.h"

#include <QtMath>
#include <QElapsedTimer>
#include <algorithm>
#include <limits>

ViterbiDecoder::ViterbiDecoder(int constraintLength,
                                const QVector<int>& generators,
                                QObject* parent)
    : QObject(parent), m_constraintLength(qMax(3, constraintLength)),
      m_generators(generators.isEmpty() ? QVector<int>{0171, 0133} : generators),
      m_timeSum(0.0)
{
    m_numStates = 1 << (m_constraintLength - 1);
    m_rate = m_generators.size();
}

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

QVector<int> ViterbiDecoder::decodeHard(const QVector<int>& received)
{
    QElapsedTimer timer;
    timer.start();

    int T = received.size() / m_rate;
    QVector<QVector<double>> metrics(T + 1, QVector<double>(m_numStates,
        std::numeric_limits<double>::infinity()));
    QVector<QVector<int>> paths(T + 1, QVector<int>(m_numStates, 0));

    metrics[0][0] = 0.0;

    for (int t = 0; t < T; ++t) {
        QVector<int> recv(m_rate);
        for (int r = 0; r < m_rate; ++r) {
            recv[r] = (t * m_rate + r < received.size()) ? received[t * m_rate + r] : 0;
        }

        for (int state = 0; state < m_numStates; ++state) {
            if (metrics[t][state] >= std::numeric_limits<double>::infinity() / 2) continue;

            for (int input = 0; input <= 1; ++input) {
                int nextState = ((state << 1) | input) & (m_numStates - 1);
                int cost = branchMetricHard(state, input, recv);
                double newMetric = metrics[t][state] + cost;

                if (newMetric < metrics[t + 1][nextState]) {
                    metrics[t + 1][nextState] = newMetric;
                    paths[t + 1][nextState] = state;
                }
            }
        }
    }

    int finalState = 0;
    double bestMetric = metrics[T][0];
    for (int s = 1; s < m_numStates; ++s) {
        if (metrics[T][s] < bestMetric) {
            bestMetric = metrics[T][s];
            finalState = s;
        }
    }

    QVector<int> decoded = traceback(paths, finalState, T);

    m_stats.totalDecodings++;
    m_stats.totalBitsProcessed += T;
    m_stats.totalBitsCorrected += static_cast<quint64>(bestMetric);
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalDecodings;

    int corrections = static_cast<int>(bestMetric);
    emit decodingCompleted(T, decoded.size(), corrections);
    return decoded;
}

QVector<int> ViterbiDecoder::decodeSoft(const QVector<double>& softReceived)
{
    QElapsedTimer timer;
    timer.start();

    int T = softReceived.size() / m_rate;
    QVector<QVector<double>> metrics(T + 1, QVector<double>(m_numStates,
        std::numeric_limits<double>::infinity()));
    QVector<QVector<int>> paths(T + 1, QVector<int>(m_numStates, 0));

    metrics[0][0] = 0.0;

    for (int t = 0; t < T; ++t) {
        QVector<double> recv(m_rate);
        for (int r = 0; r < m_rate; ++r) {
            recv[r] = (t * m_rate + r < softReceived.size())
                ? softReceived[t * m_rate + r] : 0.0;
        }

        for (int state = 0; state < m_numStates; ++state) {
            if (metrics[t][state] >= std::numeric_limits<double>::infinity() / 2) continue;

            for (int input = 0; input <= 1; ++input) {
                int nextState = ((state << 1) | input) & (m_numStates - 1);
                double cost = branchMetricSoft(state, input, recv);
                double newMetric = metrics[t][state] + cost;

                if (newMetric < metrics[t + 1][nextState]) {
                    metrics[t + 1][nextState] = newMetric;
                    paths[t + 1][nextState] = state;
                }
            }
        }
    }

    int finalState = 0;
    double bestMetric = metrics[T][0];
    for (int s = 1; s < m_numStates; ++s) {
        if (metrics[T][s] < bestMetric) { bestMetric = metrics[T][s]; finalState = s; }
    }

    QVector<int> decoded = traceback(paths, finalState, T);

    m_stats.totalDecodings++;
    m_stats.totalBitsProcessed += T;
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalDecodings;

    emit decodingCompleted(T, decoded.size(), 0);
    return decoded;
}

int ViterbiDecoder::branchMetricHard(int state, int input,
                                      const QVector<int>& received) const
{
    QVector<int> expected = getOutput(state, input);
    int cost = 0;
    for (int i = 0; i < m_rate; ++i) {
        if (i < received.size() && expected[i] != received[i]) ++cost;
    }
    return cost;
}

double ViterbiDecoder::branchMetricSoft(int state, int input,
                                          const QVector<double>& softReceived) const
{
    QVector<int> expected = getOutput(state, input);
    double cost = 0.0;
    for (int i = 0; i < m_rate; ++i) {
        double ref = (i < expected.size() && expected[i] == 1) ? 1.0 : -1.0;
        double recv = (i < softReceived.size()) ? softReceived[i] : 0.0;
        cost += (ref - recv) * (ref - recv);
    }
    return cost;
}

QVector<int> ViterbiDecoder::getOutput(int state, int input) const
{
    int shiftReg = ((state << 1) | input) & (m_numStates - 1);
    QVector<int> output(m_rate);
    for (int g = 0; g < m_rate; ++g) {
        int outBit = 0;
        for (int k = 0; k < m_constraintLength; ++k) {
            if (m_generators[g] & (1 << k)) outBit ^= ((shiftReg >> k) & 1);
        }
        output[g] = outBit;
    }
    return output;
}

QVector<int> ViterbiDecoder::traceback(const QVector<QVector<int>>& paths,
                                        int finalState, int length) const
{
    QVector<int> decoded(length);
    int state = finalState;
    for (int t = length; t > 0; --t) {
        int prevState = paths[t][state];
        decoded[t - 1] = state & 1;
        state = prevState;
    }
    return decoded;
}

void ViterbiDecoder::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
