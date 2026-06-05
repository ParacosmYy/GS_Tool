/**
 * @file ViterbiDecoder2.cpp
 * @brief 软判决维特比解码器实现 — 分支度量+幸存路径+SOVA回溯
 */

#include "utils/code21/ViterbiDecoder2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/** @brief 构造函数 @param parent 父对象 */
ViterbiDecoder2::ViterbiDecoder2(QObject* parent)
    : QObject(parent)
    , m_constraintLength(7)
    , m_tracebackDepth(40)
    , m_channelModel(ChannelModel::AWGN)
    , m_sovaEnabled(false)
    , m_metricSum(0.0)
{
    setGeneratorPolynomials({0133, 0171});
}

void ViterbiDecoder2::setConstraintLength(int K)
{
    m_constraintLength = qMax(2, K);
    m_numStates = 1 << (m_constraintLength - 1);
}

void ViterbiDecoder2::setGeneratorPolynomials(const QVector<int>& polys)
{
    m_polynomials = polys;
    m_rate = polys.size();
    m_numStates = 1 << (m_constraintLength - 1);
    buildTrellis();
}

void ViterbiDecoder2::setTracebackDepth(int depth) { m_tracebackDepth = qMax(1, depth); }
void ViterbiDecoder2::setChannelModel(ChannelModel model) { m_channelModel = model; }
void ViterbiDecoder2::setSovaEnabled(bool enable) { m_sovaEnabled = enable; }

/**
 * @brief 构建网格图(状态转移表)
 */
void ViterbiDecoder2::buildTrellis()
{
    m_nextState.assign(m_numStates, QVector<int>(2, 0));
    m_output.assign(m_numStates, QVector<QVector<int>>(2));

    for (int state = 0; state < m_numStates; ++state) {
        for (int input = 0; input < 2; ++input) {
            /* 移位寄存器: 高位移出，新输入进入最低位 */
            int reg = (state << 1) | input;
            m_nextState[state][input] = reg & (m_numStates - 1);

            /* 计算每个生成多项式的输出 */
            m_output[state][input].resize(m_rate);
            for (int p = 0; p < m_rate; ++p) {
                int out = 0;
                int poly = m_polynomials[p];
                int tmp = reg;
                for (int bit = 0; bit < m_constraintLength; ++bit) {
                    if (poly & (1 << bit)) {
                        out ^= (tmp & 1);
                    }
                    tmp >>= 1;
                }
                m_output[state][input][p] = out;
            }
        }
    }
}

/**
 * @brief 解码AWGN信道接收序列
 * @param received 接收的软判决样本(每m_rate个对应一个编码符号)
 * @return 解码结果
 */
ViterbiDecoder2::DecodeResult ViterbiDecoder2::decode(const QVector<double>& received)
{
    QElapsedTimer timer;
    timer.start();

    DecodeResult result;
    int numSymbols = received.size() / m_rate;
    if (numSymbols == 0) return result;

    /* 初始化路径度量 */
    m_pathMetric.assign(m_numStates, std::numeric_limits<double>::max());
    m_pathMetric[0] = 0.0;
    m_survivors.clear();

    for (int t = 0; t < numSymbols; ++t) {
        /* 提取当前时刻的接收符号 */
        QVector<double> rx(m_rate);
        for (int i = 0; i < m_rate; ++i) {
            rx[i] = received[t * m_rate + i];
        }

        QVector<double> newMetric(m_numStates, std::numeric_limits<double>::max());
        QVector<int> newSurvivor(m_numStates, 0);

        for (int state = 0; state < m_numStates; ++state) {
            for (int input = 0; input < 2; ++input) {
                int prevState = (state >> 1) | (input << (m_constraintLength - 2));
                if (prevState >= m_numStates) continue;

                double prevMetric = m_pathMetric[prevState];
                if (prevMetric >= std::numeric_limits<double>::max()) continue;

                double bm = branchMetric(rx, prevState, input);
                double candidate = prevMetric + bm;

                if (candidate < newMetric[state]) {
                    newMetric[state] = candidate;
                    newSurvivor[state] = input;
                }
            }
        }

        m_pathMetric = newMetric;
        m_survivors.append(newSurvivor);

        /* 限制回溯历史长度 */
        if (m_survivors.size() > m_tracebackDepth + 10) {
            m_survivors.removeFirst();
        }
    }

    /* 回溯获取解码比特 */
    traceback(result);

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalDecodedBlocks;
    m_stats.totalBitsDecoded += static_cast<quint64>(result.hardBits.size());
    m_metricSum += result.metric;
    m_stats.avgPathMetric = m_metricSum
        / static_cast<double>(m_stats.totalDecodedBlocks);
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalDecodedBlocks);

    emit blockDecoded(result.hardBits.size(), result.metric);
    return result;
}

/**
 * @brief PAM符号解码
 * @param symbols 接收PAM符号
 * @param bitsPerSymbol 每符号比特数
 * @return 解码结果
 */
ViterbiDecoder2::DecodeResult ViterbiDecoder2::decodePam(
    const QVector<double>& symbols, int bitsPerSymbol)
{
    QElapsedTimer timer;
    timer.start();

    DecodeResult result;
    int numSymbols = symbols.size();
    if (numSymbols == 0) return result;

    m_pathMetric.assign(m_numStates, std::numeric_limits<double>::max());
    m_pathMetric[0] = 0.0;
    m_survivors.clear();

    for (int t = 0; t < numSymbols; ++t) {
        QVector<double> newMetric(m_numStates, std::numeric_limits<double>::max());
        QVector<int> newSurvivor(m_numStates, 0);

        for (int state = 0; state < m_numStates; ++state) {
            for (int input = 0; input < 2; ++input) {
                int prevState = (state >> 1) | (input << (m_constraintLength - 2));
                if (prevState >= m_numStates) continue;
                double prevMetric = m_pathMetric[prevState];
                if (prevMetric >= std::numeric_limits<double>::max()) continue;

                double bm = branchMetricPam(symbols[t], prevState, input, bitsPerSymbol);
                double candidate = prevMetric + bm;
                if (candidate < newMetric[state]) {
                    newMetric[state] = candidate;
                    newSurvivor[state] = input;
                }
            }
        }
        m_pathMetric = newMetric;
        m_survivors.append(newSurvivor);
        if (m_survivors.size() > m_tracebackDepth + 10) {
            m_survivors.removeFirst();
        }
    }

    traceback(result);

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalDecodedBlocks;
    m_stats.totalBitsDecoded += static_cast<quint64>(result.hardBits.size());
    m_metricSum += result.metric;
    m_stats.avgPathMetric = m_metricSum
        / static_cast<double>(m_stats.totalDecodedBlocks);
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalDecodedBlocks);

    emit blockDecoded(result.hardBits.size(), result.metric);
    return result;
}

/** @brief 重置统计信息 */
void ViterbiDecoder2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_metricSum = 0.0;
}

/**
 * @brief 计算AWGN分支度量(欧几里得距离)
 * @param received 接收符号
 * @param state 前一状态
 * @param input 输入比特
 * @return 分支度量值
 */
double ViterbiDecoder2::branchMetric(const QVector<double>& received,
                                     int state, int input) const
{
    QVector<int> out = outputBits(state, input);
    double metric = 0.0;
    for (int i = 0; i < m_rate && i < received.size(); ++i) {
        /* 将0/1映射到 -1/+1，计算软距离 */
        double expected = out[i] == 1 ? 1.0 : -1.0;
        double diff = received[i] - expected;
        metric += diff * diff;
    }
    return metric;
}

/**
 * @brief 计算PAM分支度量
 * @param symbol 接收符号
 * @param state 前一状态
 * @param input 输入比特
 * @param bitsPerSymbol 每符号比特数
 * @return 分支度量值
 */
double ViterbiDecoder2::branchMetricPam(double symbol, int state,
                                        int input, int bitsPerSymbol) const
{
    Q_UNUSED(bitsPerSymbol)
    QVector<int> out = outputBits(state, input);
    /* 使用第一位作为PAM判决 */
    double expected = out[0] == 1 ? 1.0 : -1.0;
    double diff = symbol - expected;
    return diff * diff;
}

/**
 * @brief 获取状态转移输出比特
 * @param state 当前状态
 * @param input 输入比特
 * @return 输出比特向量
 */
QVector<int> ViterbiDecoder2::outputBits(int state, int input) const
{
    if (state < m_output.size() && input < m_output[state].size()) {
        return m_output[state][input];
    }
    return QVector<int>(m_rate, 0);
}

/**
 * @brief 回溯获取解码结果
 * @param result [out] 解码结果
 */
void ViterbiDecoder2::traceback(DecodeResult& result)
{
    if (m_survivors.isEmpty()) return;

    /* 找最小度量状态作为回溯起点 */
    int bestState = 0;
    double bestMetric = m_pathMetric[0];
    for (int s = 1; s < m_numStates; ++s) {
        if (m_pathMetric[s] < bestMetric) {
            bestMetric = m_pathMetric[s];
            bestState = s;
        }
    }
    result.metric = bestMetric;

    /* 从尾部回溯到头部 */
    int state = bestState;
    int depth = qMin(m_survivors.size(), m_tracebackDepth);
    QList<int> bits;

    for (int t = m_survivors.size() - 1; t >= 0 && bits.size() < depth; --t) {
        int input = m_survivors[t][state];
        bits.prepend(input);
        /* 逆向状态转移: 当前状态的低位来自前一状态的高位 */
        state = ((state & (m_numStates - 1)) >> 1) | (input << (m_constraintLength - 2));
        if (state >= m_numStates) state &= (m_numStates - 1);
    }

    result.hardBits.resize(bits.size());
    for (int i = 0; i < bits.size(); ++i) {
        result.hardBits[i] = bits[i];
    }

    /* SOVA软输出: 简化实现 — 使用路径度量差作为可靠性 */
    if (m_sovaEnabled) {
        result.softOutputs.resize(bits.size());
        for (int i = 0; i < bits.size(); ++i) {
            /* 近似可靠性: 当前度量与次优度量的差值 */
            double secondBest = std::numeric_limits<double>::max();
            for (int s = 0; s < m_numStates; ++s) {
                if (s != bestState && m_pathMetric[s] < secondBest) {
                    secondBest = m_pathMetric[s];
                }
            }
            result.softOutputs[i] = qAbs(secondBest - bestMetric);
        }
    }
}
