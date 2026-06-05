/**
 * @file ViterbiDecoder2.cpp
 * @brief 维特比解码器增强版实现
 */

#include "ViterbiDecoder2.h"
#include <QElapsedTimer>
#include <cmath>
#include <limits>

ViterbiDecoder2::ViterbiDecoder2(QObject* parent)
    : QObject(parent)
    , m_constraintLength(7)
    , m_stateCount(64)
    , m_rate(2)
    , m_timeSum(0.0)
{
}

void ViterbiDecoder2::setup(int constraintLength, const QVector<int>& polynomials)
{
    m_constraintLength = qMax(2, constraintLength);
    m_stateCount = 1 << (m_constraintLength - 1);
    m_polynomials = polynomials;
    m_rate = polynomials.size();

    m_transitions.resize(m_stateCount);
    for (int s = 0; s < m_stateCount; ++s) {
        m_transitions[s].resize(2);
        for (int bit = 0; bit < 2; ++bit) {
            int nextState = (s >> 1) | (bit << (m_constraintLength - 2));
            int reg = s | (bit << (m_constraintLength - 1));
            int out = 0;
            for (int p = 0; p < m_polynomials.size(); ++p) {
                int parity = 0;
                int poly = m_polynomials[p];
                int tmp = reg & poly;
                while (tmp) { parity ^= (tmp & 1); tmp >>= 1; }
                out = (out << 1) | parity;
            }
            m_transitions[s][bit] = {s, nextState, out, bit};
        }
    }
}

ViterbiDecoder2::DecodeResult ViterbiDecoder2::decodeHard(const QVector<int>& received)
{
    QElapsedTimer timer;
    timer.start();

    DecodeResult result;
    int nSymbols = received.size() / m_rate;
    if (nSymbols == 0 || m_transitions.isEmpty()) {
        m_stats.totalDecoded++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecoded;
        return result;
    }

    /* 初始化路径度量 */
    QVector<double> pathMetric(m_stateCount, std::numeric_limits<double>::max());
    QVector<QVector<int>> traceback(nSymbols, QVector<int>(m_stateCount, 0));
    pathMetric[0] = 0.0;

    for (int t = 0; t < nSymbols; ++t) {
        QVector<double> newMetric(m_stateCount, std::numeric_limits<double>::max());
        int rxSymbol = 0;
        for (int r = 0; r < m_rate; ++r)
            rxSymbol = (rxSymbol << 1) | received[t * m_rate + r];

        for (int s = 0; s < m_stateCount; ++s) {
            if (pathMetric[s] >= std::numeric_limits<double>::max() / 2) continue;
            for (int bit = 0; bit < 2; ++bit) {
                const auto& tr = m_transitions[s][bit];
                double bm = branchMetric(rxSymbol, tr.output);
                double cand = pathMetric[s] + bm;
                if (cand < newMetric[tr.toState]) {
                    newMetric[tr.toState] = cand;
                    traceback[t][tr.toState] = s;
                }
            }
        }
        pathMetric = newMetric;
    }

    /* 找最佳终点 */
    int bestState = 0;
    double bestMetric = pathMetric[0];
    for (int s = 1; s < m_stateCount; ++s) {
        if (pathMetric[s] < bestMetric) {
            bestMetric = pathMetric[s];
            bestState = s;
        }
    }

    /* 回溯 */
    result.pathMetric = bestMetric;
    result.bits.resize(nSymbols);
    int state = bestState;
    for (int t = nSymbols - 1; t >= 0; --t) {
        int prevState = traceback[t][state];
        for (int bit = 0; bit < 2; ++bit) {
            if (m_transitions[prevState][bit].toState == state) {
                result.bits[t] = m_transitions[prevState][bit].input;
                break;
            }
        }
        state = prevState;
    }

    m_stats.totalDecoded++;
    m_stats.totalBits += result.bits.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecoded;

    emit decodingCompleted(result.bits.size(), result.pathMetric);
    return result;
}

ViterbiDecoder2::DecodeResult ViterbiDecoder2::decodeSoft(const QVector<double>& received)
{
    QElapsedTimer timer;
    timer.start();

    DecodeResult result;
    int nSymbols = received.size() / m_rate;
    if (nSymbols == 0 || m_transitions.isEmpty()) {
        m_stats.totalDecoded++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecoded;
        return result;
    }

    QVector<double> pathMetric(m_stateCount, std::numeric_limits<double>::max());
    QVector<QVector<int>> traceback(nSymbols, QVector<int>(m_stateCount, 0));
    pathMetric[0] = 0.0;

    for (int t = 0; t < nSymbols; ++t) {
        QVector<double> newMetric(m_stateCount, std::numeric_limits<double>::max());

        for (int s = 0; s < m_stateCount; ++s) {
            if (pathMetric[s] >= std::numeric_limits<double>::max() / 2) continue;
            for (int bit = 0; bit < 2; ++bit) {
                const auto& tr = m_transitions[s][bit];
                double bm = 0.0;
                for (int r = 0; r < m_rate; ++r) {
                    int expected = (tr.output >> (m_rate - 1 - r)) & 1;
                    bm += softBranchMetric(received[t * m_rate + r], expected);
                }
                double cand = pathMetric[s] + bm;
                if (cand < newMetric[tr.toState]) {
                    newMetric[tr.toState] = cand;
                    traceback[t][tr.toState] = s;
                }
            }
        }
        pathMetric = newMetric;
    }

    int bestState = 0;
    double bestMetric = pathMetric[0];
    for (int s = 1; s < m_stateCount; ++s) {
        if (pathMetric[s] < bestMetric) {
            bestMetric = pathMetric[s];
            bestState = s;
        }
    }

    result.pathMetric = bestMetric;
    result.bits.resize(nSymbols);
    int state = bestState;
    for (int t = nSymbols - 1; t >= 0; --t) {
        int prevState = traceback[t][state];
        for (int bit = 0; bit < 2; ++bit) {
            if (m_transitions[prevState][bit].toState == state) {
                result.bits[t] = m_transitions[prevState][bit].input;
                break;
            }
        }
        state = prevState;
    }

    m_stats.totalDecoded++;
    m_stats.totalBits += result.bits.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecoded;

    emit decodingCompleted(result.bits.size(), result.pathMetric);
    return result;
}

QVector<QPair<int, double>> ViterbiDecoder2::survivorPaths() const
{
    return m_survivors;
}

int ViterbiDecoder2::stateCount() const { return m_stateCount; }

double ViterbiDecoder2::branchMetric(int symbol, int expected) const
{
    int diff = symbol ^ expected;
    int dist = 0;
    while (diff) { dist += diff & 1; diff >>= 1; }
    return static_cast<double>(dist);
}

double ViterbiDecoder2::softBranchMetric(double received, int expected) const
{
    double ref = expected ? 1.0 : -1.0;
    return (received - ref) * (received - ref);
}

ViterbiDecoder2::Stats ViterbiDecoder2::stats() const { return m_stats; }

void ViterbiDecoder2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
