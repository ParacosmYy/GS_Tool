/**
 * @file TrellisCode.cpp
 * @brief TrellisCode 实现
 *
 * 实现网格编码调制：Ungerboeck集合分割、卷积编码、Viterbi最优序列检测。
 */

#include "utils/code166/TrellisCode.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

TrellisCode::TrellisCode(QObject* parent)
    : QObject(parent)
{
    partition8PSK();
}

TrellisCode::~TrellisCode() = default;

void TrellisCode::setModulation(Modulation mod)
{
    m_mod = mod;
    partition8PSK();
}

void TrellisCode::setConstraintLength(int k)
{
    m_constraintLen = qMax(2, k);
    m_numStates = 1 << (m_constraintLen - 1);
}

int TrellisCode::trellisStates() const { return m_numStates; }

void TrellisCode::partition8PSK()
{
    /* 8-PSK constellation points (unit circle, pi/8 offset for Ungerboeck) */
    m_constellation.resize(8);
    for (int i = 0; i < 8; ++i) {
        double angle = M_PI / 8.0 + i * M_PI_4;
        m_constellation[i] = qMakePair(qCos(angle), qSin(angle));
    }

    /* Ungerboeck 3-level partition for 8-PSK:
     * Level 0: {0,1,2,3,4,5,6,7}
     * Level 1: {0,2,4,6}, {1,3,5,7}
     * Level 2: {0,4},{2,6}, {1,5},{3,7} */
    m_partitions = {
        {0, 2, 4, 6}, {1, 3, 5, 7},
        {0, 4}, {2, 6}, {1, 5}, {3, 7}
    };

    /* Build trellis for rate 2/3 code: 4 states, 2-bit input -> 3-bit output */
    m_numStates = 4;
    m_nextState = {{0, 2}, {0, 2}, {1, 3}, {1, 3}};
    /* Output symbols map to partition subsets */
    m_outputSymbol = {{0, 4}, {1, 5}, {2, 6}, {3, 7}};
}

double TrellisCode::symbolDistance(const QPair<double, double>& a,
                                   const QPair<double, double>& b)
{
    double di = a.first - b.first;
    double dq = a.second - b.second;
    return qSqrt(di * di + dq * dq);
}

int TrellisCode::convolutionalEncode(int state, int input) const
{
    if (state < 0 || state >= m_numStates) return 0;
    int idx = qBound(0, input, 1);
    return m_outputSymbol[state][idx];
}

double TrellisCode::branchMetric(const QPair<double, double>& received,
                                  int symbolIdx) const
{
    if (symbolIdx < 0 || symbolIdx >= m_constellation.size()) return 1e9;
    return symbolDistance(received, m_constellation[symbolIdx]);
}

QVector<QPair<double, double>> TrellisCode::encode(const QVector<int>& bits)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<double, double>> symbols;
    int state = 0;
    /* Process bits in pairs */
    for (int i = 0; i + 1 < bits.size(); i += 2) {
        int b0 = bits[i] & 1;
        int b1 = bits[i + 1] & 1;
        /* b1 goes through convolutional encoder -> selects subset */
        int outSym = m_outputSymbol[state][b1];
        /* b0 selects within subset (toggle LSB) */
        int sym = outSym + (b0 * (m_constellation.size() / m_numStates / 2));
        sym = sym % m_constellation.size();
        symbols.append(m_constellation[sym]);
        state = m_nextState[state][b1];
    }

    m_stats.totalEncodes++;
    m_stats.lastBlockSize = symbols.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalEncodes + m_stats.totalDecodes > 0)
        ? m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes) : 0.0;

    emit encodeCompleted(symbols.size());
    return symbols;
}

QVector<int> TrellisCode::decode(const QVector<QPair<double, double>>& symbols)
{
    QElapsedTimer timer;
    timer.start();

    int T = symbols.size();
    if (T == 0) return QVector<int>();

    int S = m_numStates;
    const double INF = std::numeric_limits<double>::max();

    /* Viterbi: path metric and survivor */
    QVector<double> pm(S, INF);
    QVector<QVector<int>> survivors(S);
    pm[0] = 0.0; /* Start from state 0 */

    for (int t = 0; t < T; ++t) {
        QVector<double> newPm(S, INF);
        QVector<QVector<int>> newSurvivors(S);

        for (int s = 0; s < S; ++s) {
            for (int inp = 0; inp < 2; ++inp) {
                /* Find predecessor state that transitions to s with input inp */
                for (int ps = 0; ps < S; ++ps) {
                    if (m_nextState[ps][inp] != s) continue;
                    if (pm[ps] >= INF) continue;

                    int outSym = m_outputSymbol[ps][inp];
                    double cost = pm[ps] + branchMetric(symbols[t], outSym);
                    if (cost < newPm[s]) {
                        newPm[s] = cost;
                        newSurvivors[s] = survivors[ps];
                        newSurvivors[s].append(inp);
                    }
                }
            }
        }
        pm = newPm;
        survivors = newSurvivors;
    }

    /* Traceback from best final state */
    int bestState = 0;
    double bestMetric = INF;
    for (int s = 0; s < S; ++s) {
        if (pm[s] < bestMetric) { bestMetric = pm[s]; bestState = s; }
    }

    /* Convert survivor inputs to output bits */
    QVector<int> decodedBits;
    for (int inp : survivors[bestState]) {
        decodedBits.append(0); /* b0 assumed 0 */
        decodedBits.append(inp);
    }

    m_stats.totalDecodes++;
    m_stats.lastBlockSize = T;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalEncodes + m_stats.totalDecodes > 0)
        ? m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes) : 0.0;

    emit decodeCompleted(decodedBits.size());
    return decodedBits;
}

void TrellisCode::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
