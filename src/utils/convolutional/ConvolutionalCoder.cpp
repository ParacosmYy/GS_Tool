/**
 * @file ConvolutionalCoder.cpp
 * @brief 卷积码编解码器实现 — Viterbi解码纠错
 */

#include "utils/convolutional/ConvolutionalCoder.h"

#include <QElapsedTimer>
#include <algorithm>
#include <climits>

/** @brief 构造函数 @param parent 父对象 */
ConvolutionalCoder::ConvolutionalCoder(QObject* parent)
    : QObject(parent)
    , m_constraintLength(7)
    , m_generators({0171, 0133})
    , m_numStates(1 << (m_constraintLength - 1))
    , m_shiftRegister(0)
    , m_timeSum(0.0)
{
}

/** @brief 编码输入字节流 @param data 原始数据 @return 编码后数据 */
QByteArray ConvolutionalCoder::encode(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.isEmpty()) {
        return {};
    }

    /* 将字节展开为比特，逐比特编码 */
    QVector<int> allEncodedBits;
    allEncodedBits.reserve(data.size() * 8 * m_generators.size());

    m_shiftRegister = 0;
    for (int i = 0; i < data.size(); ++i) {
        unsigned char byte = static_cast<unsigned char>(data[i]);
        for (int bit = 7; bit >= 0; --bit) {
            int inputBit = (byte >> bit) & 1;
            QVector<int> encoded = encodeBit(inputBit);
            for (int v : encoded) {
                allEncodedBits.append(v);
            }
        }
    }

    /* 将编码比特打包为字节 */
    QByteArray result;
    int totalBits = allEncodedBits.size();
    int byteCount = (totalBits + 7) / 8;
    result.resize(byteCount, 0);

    for (int i = 0; i < totalBits; ++i) {
        if (allEncodedBits[i]) {
            result[i / 8] |= (1 << (7 - (i % 8)));
        }
    }

    m_stats.totalEncodes++;
    m_timeSum += timer.elapsed();
    double total = static_cast<double>(m_stats.totalEncodes + m_stats.totalDecodes);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit encoded();
    return result;
}

/** @brief Viterbi解码 @param data 接收编码数据 @return 解码后原始数据 */
QByteArray ConvolutionalCoder::decode(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.isEmpty()) {
        return {};
    }

    /* 将字节展开为比特 */
    QVector<int> receivedBits;
    receivedBits.reserve(data.size() * 8);
    for (int i = 0; i < data.size(); ++i) {
        unsigned char byte = static_cast<unsigned char>(data[i]);
        for (int bit = 7; bit >= 0; --bit) {
            receivedBits.append((byte >> bit) & 1);
        }
    }

    /* Viterbi解码 */
    QVector<int> decodedBits = viterbiDecode(receivedBits);

    /* 将解码比特打包为字节 */
    QByteArray result;
    int totalBits = decodedBits.size();
    int byteCount = totalBits / 8;
    result.resize(byteCount, 0);

    for (int i = 0; i < byteCount * 8; ++i) {
        if (i < decodedBits.size() && decodedBits[i]) {
            result[i / 8] |= (1 << (7 - (i % 8)));
        }
    }

    m_stats.totalDecodes++;
    m_timeSum += timer.elapsed();
    double total = static_cast<double>(m_stats.totalEncodes + m_stats.totalDecodes);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit decoded();
    return result;
}

/** @brief 设置约束长度 @param length 约束长度(3~9) */
void ConvolutionalCoder::setConstraintLength(int length)
{
    if (length < 3) length = 3;
    if (length > 9) length = 9;
    m_constraintLength = length;
    m_numStates = 1 << (length - 1);
}

/** @brief 设置生成多项式 @param polynomials 生成多项式列表 */
void ConvolutionalCoder::setGeneratorPolynomials(const QVector<int>& polynomials)
{
    if (polynomials.isEmpty()) return;
    m_generators = polynomials;
}

/** @brief 重置统计 */
void ConvolutionalCoder::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 编码单个比特 @param inputBit 输入比特 @return 编码输出比特 */
QVector<int> ConvolutionalCoder::encodeBit(int inputBit)
{
    /* 移位寄存器左移后加入新比特 */
    m_shiftRegister = ((m_shiftRegister << 1) | inputBit)
                      & (m_numStates - 1);

    QVector<int> output;
    output.reserve(m_generators.size());
    for (int gen : m_generators) {
        int bit = 0;
        int reg = m_shiftRegister;
        int mask = gen;
        /* 按多项式对移位寄存器进行异或 */
        for (int i = 0; i < m_constraintLength; ++i) {
            if (mask & (1 << i)) {
                bit ^= ((reg >> i) & 1);
            }
        }
        output.append(bit);
    }
    return output;
}

/** @brief Viterbi核心算法 @param receivedBits 接收比特序列 @return 解码比特 */
QVector<int> ConvolutionalCoder::viterbiDecode(const QVector<int>& receivedBits)
{
    int rate = m_generators.size();
    int stepBits = rate; /* 每步消费rate个比特 */
    int numSteps = receivedBits.size() / stepBits;

    if (numSteps <= 0) return {};

    /* 初始化路径度量 */
    QVector<int> pathMetric(m_numStates, INT_MAX);
    pathMetric[0] = 0;
    QVector<QVector<int>> paths;
    paths.reserve(numSteps);

    for (int step = 0; step < numSteps; ++step) {
        /* 提取当前步接收的比特 */
        QVector<int> rxBits(rate);
        for (int r = 0; r < rate; ++r) {
            int idx = step * rate + r;
            rxBits[r] = (idx < receivedBits.size()) ? receivedBits[idx] : 0;
        }

        QVector<int> newMetric(m_numStates, INT_MAX);
        QVector<int> predecessor(m_numStates, 0);

        for (int state = 0; state < m_numStates; ++state) {
            if (pathMetric[state] == INT_MAX) continue;

            for (int input = 0; input <= 1; ++input) {
                int nextState = ((state << 1) | input) & (m_numStates - 1);
                QVector<int> expected = getOutput(state, input);
                int metric = branchMetric(expected, rxBits);
                int totalMetric = pathMetric[state] + metric;

                if (totalMetric < newMetric[nextState]) {
                    newMetric[nextState] = totalMetric;
                    predecessor[nextState] = state;
                }
            }
        }

        pathMetric = newMetric;
        paths.append(predecessor);
    }

    /* 找最小度量状态 */
    int finalState = 0;
    int minMetric = pathMetric[0];
    for (int s = 1; s < m_numStates; ++s) {
        if (pathMetric[s] < minMetric) {
            minMetric = pathMetric[s];
            finalState = s;
        }
    }

    return traceback(paths, finalState, numSteps);
}

/** @brief 计算分支度量 @param expected 期望输出 @param received 实际接收 @return 汉明距离 */
int ConvolutionalCoder::branchMetric(const QVector<int>& expected,
                                     const QVector<int>& received) const
{
    int dist = 0;
    int len = std::min(expected.size(), received.size());
    for (int i = 0; i < len; ++i) {
        if (expected[i] != received[i]) ++dist;
    }
    return dist;
}

/** @brief 获取某状态某输入的编码输出 @param state 编码器状态 @param input 输入比特 @return 编码输出 */
QVector<int> ConvolutionalCoder::getOutput(int state, int input) const
{
    int reg = ((state << 1) | input) & (m_numStates - 1);
    QVector<int> output;
    output.reserve(m_generators.size());
    for (int gen : m_generators) {
        int bit = 0;
        for (int i = 0; i < m_constraintLength; ++i) {
            if (gen & (1 << i)) {
                bit ^= ((reg >> i) & 1);
            }
        }
        output.append(bit);
    }
    return output;
}

/** @brief 回溯路径 @param paths 路径历史 @param finalState 最终状态 @param length 回溯长度 @return 解码比特 */
QVector<int> ConvolutionalCoder::traceback(const QVector<QVector<int>>& paths,
                                           int finalState, int length) const
{
    QVector<int> decodedBits;
    decodedBits.reserve(length);

    int state = finalState;
    for (int step = length - 1; step >= 0; --step) {
        int prevState = paths[step][state];
        /* 输入比特 = 状态的LSB */
        int inputBit = state & 1;
        decodedBits.prepend(inputBit);
        state = prevState;
    }

    return decodedBits;
}
