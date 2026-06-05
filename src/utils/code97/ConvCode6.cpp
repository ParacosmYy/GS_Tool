#include "ConvCode6.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file ConvCode6.cpp
 * @brief 卷积码编解码器实现
 *
 * 实现卷积编码和Viterbi软判决译码。编码通过移位寄存器
 * 和生成多项式产生输出，译码通过网格图上的最大似然路径搜索实现。
 */

/**
 * @brief 构造函数，初始化默认约束长度和生成多项式
 * @param parent 父QObject对象指针
 */
ConvCode6::ConvCode6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置约束长度
 * @param length 约束长度(K)，决定编码器记忆深度
 */
void ConvCode6::setConstraintLen(int length)
{
    m_constraintLen = qMax(2, length);
}

/**
 * @brief 设置生成多项式
 * @param gen 生成多项式的八进制表示向量
 */
void ConvCode6::setGenerator(const QVector<int>& gen)
{
    if (gen.size() >= 2) {
        m_generator = gen;
    }
}

/**
 * @brief 卷积编码
 *
 * 将输入比特流通过移位寄存器，每个输入比特产生
 * n个输出比特(n=生成多项式个数)。
 *
 * @param bits 输入比特序列(0或1)
 * @return 编码输出比特序列
 */
QVector<int> ConvCode6::encode(const QVector<int>& bits)
{
    if (bits.isEmpty()) return {};

    QElapsedTimer timer;
    timer.start();

    const int n = m_generator.size();
    const int regLen = m_constraintLen - 1;
    QVector<int> output;
    QVector<int> reg(regLen, 0);

    for (int bit : bits) {
        // 移位寄存器右移，新比特进入最高位
        for (int i = regLen - 1; i > 0; --i) {
            reg[i] = reg[i - 1];
        }
        reg[0] = bit;

        // 对每个生成多项式计算输出
        for (int g = 0; g < n; ++g) {
            int outBit = bit; // 包含当前输入
            int genPoly = m_generator[g];
            for (int i = 0; i < regLen; ++i) {
                if (genPoly & (1 << (regLen - i))) {
                    outBit ^= reg[i];
                }
            }
            output.append(outBit);
        }
    }

    m_stats.totalEncoded += bits.size();
    m_timeSum += timer.elapsed();
    const int total = m_stats.totalEncoded + m_stats.totalDecoded;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit codingCompleted(0);
    return output;
}

/**
 * @brief Viterbi译码(软判决)
 *
 * 在网格图上进行最大似然路径搜索:
 * 1. 对每个时刻的状态计算分支度量(欧氏距离)
 * 2. 选择累积度量最小的路径保留(加-比-选)
 * 3. 回溯幸存路径获得译码结果
 *
 * @param received 接收到的软比特序列(实数值)
 * @return 译码后的比特序列
 */
QVector<int> ConvCode6::decode(const QVector<double>& received)
{
    if (received.isEmpty()) return {};

    QElapsedTimer timer;
    timer.start();

    const int n = m_generator.size();
    const int numStates = 1 << (m_constraintLen - 1);
    const int frameLen = received.size() / n;
    int correctedBits = 0;

    // 初始化路径度量
    QVector<double> metric(numStates, 1e18);
    QVector<int> prevPath(numStates * frameLen, 0);
    metric[0] = 0.0;

    for (int t = 0; t < frameLen; ++t) {
        QVector<double> newMetric(numStates, 1e18);

        for (int state = 0; state < numStates; ++state) {
            if (metric[state] >= 1e17) continue;

            // 尝试输入0和1
            for (int input = 0; input <= 1; ++input) {
                int nextState = (state >> 1) | (input << (m_constraintLen - 2));

                // 计算分支度量(欧氏距离)
                double branchMetric = 0.0;
                for (int g = 0; g < n; ++g) {
                    const double expected = ((input ^ state) & 1) ? 1.0 : -1.0;
                    const double rx = received[t * n + g];
                    branchMetric += (rx - expected) * (rx - expected);
                }

                const double totalMetric = metric[state] + branchMetric;
                if (totalMetric < newMetric[nextState]) {
                    newMetric[nextState] = totalMetric;
                    prevPath[t * numStates + nextState] = state;
                }
            }
        }
        metric = newMetric;
    }

    // 回溯
    QVector<int> decoded(frameLen);
    int state = 0;
    for (int t = 0; t < numStates; ++t) {
        if (metric[t] < metric[state]) state = t;
    }
    for (int t = frameLen - 1; t >= 0; --t) {
        decoded[t] = (state >> (m_constraintLen - 2)) & 1;
        state = prevPath[t * numStates + state];
    }

    m_stats.totalDecoded += decoded.size();
    m_timeSum += timer.elapsed();
    const int total = m_stats.totalEncoded + m_stats.totalDecoded;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit codingCompleted(correctedBits);
    return decoded;
}

/**
 * @brief 重置所有统计信息
 */
void ConvCode6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
