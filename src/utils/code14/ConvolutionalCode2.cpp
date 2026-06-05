/**
 * @file ConvolutionalCode2.cpp
 * @brief 卷积码编解码器实现 — Viterbi硬/软判决译码
 */

#include "utils/code14/ConvolutionalCode2.h"

#include <QElapsedTimer>

#include <algorithm>
#include <limits>
#include <cmath>

// ═══════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════

ConvolutionalCode2::ConvolutionalCode2(QObject* parent)
    : QObject(parent)
    , m_numStates(64)
    , m_mask(63)
{
    /* 默认配置: rate 1/2, K=7, 业界标准多项式 */
    m_config.constraintLength = 7;
    m_config.generators = {0171, 0133};  /* 八进制: 171, 133 */
    m_config.enableTermination = true;
    m_numStates = 1 << (m_config.constraintLength - 1);
    m_mask = m_numStates - 1;
}

ConvolutionalCode2::~ConvolutionalCode2() = default;

// ═══════════════════════════════════════════════════════════
// 配置
// ═══════════════════════════════════════════════════════════

void ConvolutionalCode2::configure(const Config& config)
{
    m_config = config;
    m_numStates = 1 << (config.constraintLength - 1);
    m_mask = m_numStates - 1;
}

ConvolutionalCode2::Config ConvolutionalCode2::configuration() const
{
    return m_config;
}

// ═══════════════════════════════════════════════════════════
// 编码
// ═══════════════════════════════════════════════════════════

QVector<int> ConvolutionalCode2::encodeBits(const QVector<int>& bits)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> output;
    int state = 0;
    int R = m_config.generators.size();

    for (int bit : bits) {
        /* 将输入比特移入状态寄存器最高位 */
        auto trans = transition(state, bit);
        state = trans.first;

        for (int outBit : trans.second) {
            output.append(outBit);
        }
    }

    /* 终止: 输入K-1个零使状态归零 */
    if (m_config.enableTermination) {
        for (int i = 0; i < m_config.constraintLength - 1; ++i) {
            auto trans = transition(state, 0);
            state = trans.first;
            for (int outBit : trans.second) {
                output.append(outBit);
            }
        }
    }

    m_stats.totalEncodes++;
    m_stats.totalBitsEncoded += bits.size();
    m_timeSum += static_cast<double>(timer.elapsed());
    quint64 totalOps = m_stats.totalEncodes + m_stats.totalDecodes;
    m_stats.avgProcessingTimeMs =
        (totalOps > 0) ? m_timeSum / static_cast<double>(totalOps) : 0.0;

    emit encodeCompleted(bits.size(), output.size());
    return output;
}

QVector<int> ConvolutionalCode2::encodeBytes(const QByteArray& data)
{
    QVector<int> allBits;
    allBits.reserve(data.size() * 8);

    for (unsigned char byte : data) {
        for (int i = 7; i >= 0; --i) {
            allBits.append((byte >> i) & 1);
        }
    }

    return encodeBits(allBits);
}

// ═══════════════════════════════════════════════════════════
// 译码 — 硬判决
// ═══════════════════════════════════════════════════════════

ConvolutionalCode2::DecodeResult ConvolutionalCode2::decodeHard(
    const QVector<int>& encodedBits)
{
    QElapsedTimer timer;
    timer.start();

    DecodeResult result;
    int R = m_config.generators.size();
    int K = m_config.constraintLength;

    int termBits = m_config.enableTermination ? (K - 1) * R : 0;
    int totalSymbols = encodedBits.size();
    if (totalSymbols < R || (totalSymbols % R) != 0) {
        result.success = false;
        return result;
    }

    int totalSteps = totalSymbols / R;
    int dataSteps = m_config.enableTermination
                        ? totalSteps - (K - 1) : totalSteps;

    buildTrellisHard(encodedBits, totalSteps);
    QVector<int> decodedBits = traceback(totalSteps, m_config.enableTermination);

    /* 截取数据部分 */
    decodedBits = decodedBits.mid(0, dataSteps);

    /* 转换为字节数组 */
    int numBytes = decodedBits.size() / 8;
    result.decoded.resize(numBytes);
    for (int i = 0; i < numBytes; ++i) {
        unsigned char byte = 0;
        for (int b = 0; b < 8 && (i * 8 + b) < decodedBits.size(); ++b) {
            if (decodedBits[i * 8 + b]) {
                byte |= (1 << (7 - b));
            }
        }
        result.decoded[i] = static_cast<char>(byte);
    }

    /* 获取最终路径度量 */
    int finalState = 0;
    if (totalSteps > 0 && finalState < m_numStates) {
        result.pathMetric = static_cast<double>(
            m_pathMetric[totalSteps][finalState]);
    }
    result.success = true;

    m_stats.totalDecodes++;
    m_stats.totalBitsDecoded += decodedBits.size();
    m_timeSum += static_cast<double>(timer.elapsed());
    quint64 totalOps = m_stats.totalEncodes + m_stats.totalDecodes;
    m_stats.avgProcessingTimeMs =
        (totalOps > 0) ? m_timeSum / static_cast<double>(totalOps) : 0.0;

    emit decodeCompleted(result.decoded.size(), result.pathMetric);
    return result;
}

// ═══════════════════════════════════════════════════════════
// 译码 — 软判决
// ═══════════════════════════════════════════════════════════

ConvolutionalCode2::DecodeResult ConvolutionalCode2::decodeSoft(
    const QVector<int>& softValues, int precision)
{
    QElapsedTimer timer;
    timer.start();

    DecodeResult result;
    int R = m_config.generators.size();
    int K = m_config.constraintLength;

    int totalSymbols = softValues.size();
    if (totalSymbols < R || (totalSymbols % R) != 0) {
        result.success = false;
        return result;
    }

    int totalSteps = totalSymbols / R;
    int dataSteps = m_config.enableTermination
                        ? totalSteps - (K - 1) : totalSteps;

    buildTrellisSoft(softValues, totalSteps, precision);
    QVector<int> decodedBits = traceback(totalSteps, m_config.enableTermination);

    decodedBits = decodedBits.mid(0, dataSteps);

    int numBytes = decodedBits.size() / 8;
    result.decoded.resize(numBytes);
    for (int i = 0; i < numBytes; ++i) {
        unsigned char byte = 0;
        for (int b = 0; b < 8 && (i * 8 + b) < decodedBits.size(); ++b) {
            if (decodedBits[i * 8 + b]) {
                byte |= (1 << (7 - b));
            }
        }
        result.decoded[i] = static_cast<char>(byte);
    }

    int finalState = 0;
    if (totalSteps > 0 && finalState < m_numStates) {
        result.pathMetric = static_cast<double>(
            m_pathMetric[totalSteps][finalState]);
    }
    result.success = true;

    m_stats.totalDecodes++;
    m_stats.totalBitsDecoded += decodedBits.size();
    m_timeSum += static_cast<double>(timer.elapsed());
    quint64 totalOps = m_stats.totalEncodes + m_stats.totalDecodes;
    m_stats.avgProcessingTimeMs =
        (totalOps > 0) ? m_timeSum / static_cast<double>(totalOps) : 0.0;

    emit decodeCompleted(result.decoded.size(), result.pathMetric);
    return result;
}

// ═══════════════════════════════════════════════════════════
// 辅助
// ═══════════════════════════════════════════════════════════

int ConvolutionalCode2::rate() const
{
    return m_config.generators.size();
}

int ConvolutionalCode2::stateCount() const
{
    return m_numStates;
}

// ═══════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════

ConvolutionalCode2::Stats ConvolutionalCode2::stats() const
{
    return m_stats;
}

void ConvolutionalCode2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

// ═══════════════════════════════════════════════════════════
// 内部实现
// ═══════════════════════════════════════════════════════════

QPair<int, QVector<int>> ConvolutionalCode2::transition(
    int fromState, int inputBit) const
{
    /* 移位寄存器: 新比特在MSB, 旧比特右移 */
    int registerState = ((fromState << 1) | inputBit) & ((1 << m_config.constraintLength) - 1);
    int nextState = (fromState << 1 | inputBit) & m_mask;

    QVector<int> output;
    output.reserve(m_config.generators.size());

    for (int gen : m_config.generators) {
        /* 计算生成多项式与寄存器的模2内积 */
        int outBit = 0;
        for (int bit = 0; bit < m_config.constraintLength; ++bit) {
            if (gen & (1 << bit)) {
                outBit ^= (registerState >> bit) & 1;
            }
        }
        output.append(outBit);
    }

    return {nextState, output};
}

int ConvolutionalCode2::branchMetricHard(
    const QVector<int>& received,
    const QVector<int>& expected) const
{
    /* 汉明距离 */
    int dist = 0;
    int n = qMin(received.size(), expected.size());
    for (int i = 0; i < n; ++i) {
        if (received[i] != expected[i]) ++dist;
    }
    return dist;
}

int ConvolutionalCode2::branchMetricSoft(
    const QVector<int>& received,
    const QVector<int>& expected) const
{
    /* 软距离: 将期望0/1映射为+/-最大值, 然后计算相关 */
    int metric = 0;
    int n = qMin(received.size(), expected.size());
    for (int i = 0; i < n; ++i) {
        /* 期望比特0 -> 符号-, 期望比特1 -> 符号+ */
        int sign = (expected[i] == 1) ? 1 : -1;
        /* 度量 = -sign * received (负号: 越小越好) */
        metric -= sign * received[i];
    }
    return metric;
}

void ConvolutionalCode2::buildTrellisHard(
    const QVector<int>& encodedBits, int numSteps)
{
    int R = m_config.generators.size();
    int NS = m_numStates;

    m_pathMetric.assign(numSteps + 1, QVector<int>(NS, std::numeric_limits<int>::max() / 2));
    m_survivor.assign(numSteps + 1, QVector<int>(NS, -1));
    m_inputBit.assign(numSteps + 1, QVector<int>(NS, -1));

    m_pathMetric[0][0] = 0;

    for (int step = 0; step < numSteps; ++step) {
        /* 提取当前步骤的R个接收符号 */
        QVector<int> received(R);
        for (int r = 0; r < R; ++r) {
            received[r] = (step * R + r < encodedBits.size())
                              ? encodedBits[step * R + r] : 0;
        }

        for (int state = 0; state < NS; ++state) {
            if (m_pathMetric[step][state] >= std::numeric_limits<int>::max() / 2)
                continue;

            for (int input = 0; input <= 1; ++input) {
                auto trans = transition(state, input);
                int nextState = trans.first;
                int bm = branchMetricHard(received, trans.second);
                int newMetric = m_pathMetric[step][state] + bm;

                if (newMetric < m_pathMetric[step + 1][nextState]) {
                    m_pathMetric[step + 1][nextState] = newMetric;
                    m_survivor[step + 1][nextState] = state;
                    m_inputBit[step + 1][nextState] = input;
                }
            }
        }
    }
}

void ConvolutionalCode2::buildTrellisSoft(
    const QVector<int>& softValues, int numSteps, int /*precision*/)
{
    int R = m_config.generators.size();
    int NS = m_numStates;

    m_pathMetric.assign(numSteps + 1, QVector<int>(NS, std::numeric_limits<int>::max() / 2));
    m_survivor.assign(numSteps + 1, QVector<int>(NS, -1));
    m_inputBit.assign(numSteps + 1, QVector<int>(NS, -1));

    m_pathMetric[0][0] = 0;

    for (int step = 0; step < numSteps; ++step) {
        QVector<int> received(R);
        for (int r = 0; r < R; ++r) {
            received[r] = (step * R + r < softValues.size())
                              ? softValues[step * R + r] : 0;
        }

        for (int state = 0; state < NS; ++state) {
            if (m_pathMetric[step][state] >= std::numeric_limits<int>::max() / 2)
                continue;

            for (int input = 0; input <= 1; ++input) {
                auto trans = transition(state, input);
                int nextState = trans.first;
                int bm = branchMetricSoft(received, trans.second);
                int newMetric = m_pathMetric[step][state] + bm;

                if (newMetric < m_pathMetric[step + 1][nextState]) {
                    m_pathMetric[step + 1][nextState] = newMetric;
                    m_survivor[step + 1][nextState] = state;
                    m_inputBit[step + 1][nextState] = input;
                }
            }
        }
    }
}

QVector<int> ConvolutionalCode2::traceback(int numSteps, bool terminate)
{
    QVector<int> bits;
    bits.reserve(numSteps);

    int state = 0;

    /* 如果有终止, 从全零状态回溯; 否则找最优状态 */
    if (!terminate && numSteps > 0) {
        int bestMetric = std::numeric_limits<int>::max();
        for (int s = 0; s < m_numStates; ++s) {
            if (m_pathMetric[numSteps][s] < bestMetric) {
                bestMetric = m_pathMetric[numSteps][s];
                state = s;
            }
        }
    }

    /* 从最后一步向回回溯 */
    for (int step = numSteps; step > 0; --step) {
        if (step <= numSteps && state >= 0 && state < m_numStates) {
            int input = m_inputBit[step][state];
            if (input >= 0) {
                bits.prepend(input);
            }
            state = m_survivor[step][state];
        }
    }

    return bits;
}
