/**
 * @file ConvolutionalCode6.cpp
 * @brief ConvolutionalCode6 实现
 *
 * 实现卷积码Viterbi译码器：格图构建、ACS加比选、
 * 软硬判决度量计算、回溯路径选择。
 */

#include "utils/code164/ConvolutionalCode6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

ConvolutionalCode6::ConvolutionalCode6(QObject* parent)
    : QObject(parent)
{
}

ConvolutionalCode6::~ConvolutionalCode6() = default;

void ConvolutionalCode6::configure(int constraintLength, const QVector<int>& generators)
{
    m_constraintLength = qMax(2, constraintLength);
    m_generators = generators;
    m_nOutput = generators.size();
    m_nStates = 1 << (m_constraintLength - 1);
}

void ConvolutionalCode6::setTracebackDepth(int depth)
{
    m_tracebackDepth = qMax(5 * m_constraintLength, depth);
}

int ConvolutionalCode6::nextState(int state, int input) const
{
    /* Shift register: new bit goes to MSB */
    return ((state >> 1) | (input << (m_constraintLength - 2))) & (m_nStates - 1);
}

QVector<int> ConvolutionalCode6::outputBits(int state, int input) const
{
    QVector<int> output(m_nOutput);
    int reg = (state | (input << (m_constraintLength - 1))) & ((1 << m_constraintLength) - 1);

    for (int g = 0; g < m_nOutput; ++g) {
        int gen = m_generators[g];
        int bit = 0;
        for (int k = 0; k < m_constraintLength; ++k) {
            if (gen & (1 << k)) bit ^= ((reg >> k) & 1);
        }
        output[g] = bit;
    }
    return output;
}

QVector<int> ConvolutionalCode6::decodeSoft(const QVector<double>& softBits)
{
    QElapsedTimer timer;
    timer.start();

    if (softBits.isEmpty() || m_generators.isEmpty()) return QVector<int>();

    int trellisLen = softBits.size() / m_nOutput;

    /* Path metrics */
    QVector<double> oldMetric(m_nStates, std::numeric_limits<double>::infinity());
    QVector<double> newMetric(m_nStates, std::numeric_limits<double>::infinity());
    oldMetric[0] = 0.0;

    /* Survivors: store history for traceback */
    int tbLen = qMin(trellisLen, m_tracebackDepth);
    QVector<QVector<int>> survivors(trellisLen, QVector<int>(m_nStates, 0));

    for (int t = 0; t < trellisLen; ++t) {
        newMetric.fill(std::numeric_limits<double>::infinity());

        /* Extract received symbol for this time step */
        QVector<double> rx(m_nOutput);
        for (int g = 0; g < m_nOutput; ++g) {
            rx[g] = (t * m_nOutput + g < softBits.size()) ? softBits[t * m_nOutput + g] : 0.0;
        }

        for (int s = 0; s < m_nStates; ++s) {
            if (oldMetric[s] == std::numeric_limits<double>::infinity()) continue;

            for (int in = 0; in <= 1; ++in) {
                int ns = nextState(s, in);
                QVector<int> tx = outputBits(s, in);

                /* Soft branch metric: squared Euclidean distance */
                double bm = 0.0;
                for (int g = 0; g < m_nOutput; ++g) {
                    double expected = tx[g] ? 1.0 : -1.0;
                    double diff = rx[g] - expected;
                    bm += diff * diff;
                }

                double cm = oldMetric[s] + bm;
                if (cm < newMetric[ns]) {
                    newMetric[ns] = cm;
                    survivors[t][ns] = s;
                }
            }
        }

        oldMetric = newMetric;
    }

    /* Traceback from best final state */
    QVector<int> decoded = traceback(survivors, trellisLen);

    /* Trim tail bits (constraint length - 1) */
    if (decoded.size() > m_constraintLength - 1) {
        decoded.resize(decoded.size() - m_constraintLength + 1);
    }

    m_stats.totalDecodes++;
    m_stats.lastPathMetric = oldMetric[0];
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalDecodes > 0)
        ? m_timeSum / m_stats.totalDecodes : 0.0;

    emit decodeCompleted(decoded.size());
    return decoded;
}

QVector<int> ConvolutionalCode6::decodeHard(const QVector<int>& hardBits)
{
    if (hardBits.isEmpty()) return QVector<int>();

    /* Convert hard bits to soft: 0->-1, 1->+1 */
    QVector<double> soft(hardBits.size());
    for (int i = 0; i < hardBits.size(); ++i) {
        soft[i] = hardBits[i] ? 1.0 : -1.0;
    }
    return decodeSoft(soft);
}

QVector<int> ConvolutionalCode6::traceback(const QVector<QVector<int>>& survivors,
                                           int trellisLen)
{
    /* Find best final state */
    QVector<double> dummy;
    int bestState = 0;
    /* Start from state 0 (tail-biting assumed) */
    int state = 0;

    QVector<int> path;
    for (int t = trellisLen - 1; t >= 0; --t) {
        int prev = survivors[t][state];
        /* Input bit that caused transition prev -> state */
        int inputBit = (state >> (m_constraintLength - 2)) & 1;
        path.prepend(inputBit);
        state = prev;
    }

    return path;
}

void ConvolutionalCode6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
