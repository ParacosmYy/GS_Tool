/**
 * @file ConvolutionalCode2.cpp
 * @brief 卷积码编解码器实现 — 生成多项式编码 + Viterbi硬判决译码
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 实现基于生成多项式的卷积编码和基于网格图的 Viterbi 硬判决译码。
 * 编码时根据约束长度和生成多项式产生输出码流；
 * 译码时在网格图上搜索最小汉明距离路径恢复原始比特。
 */

#include "utils/code54/ConvolutionalCode2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

// ──────────────────────────────────────────────
// 构造函数
// ──────────────────────────────────────────────

/**
 * @brief 构造函数，初始化默认生成多项式
 *
 * 默认约束长度为 7，生成多项式为 [0171, 0133]（八进制），
 * 对应常见的 rate-1/2 卷积码。
 *
 * @param parent 父QObject对象
 */
ConvolutionalCode2::ConvolutionalCode2(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("ConvolutionalCode2"));
    // 默认生成多项式：171(八进制)=0x79, 133(八进制)=0x5B
    m_generators = {0x79, 0x5B};
}

// ──────────────────────────────────────────────
// 参数配置
// ──────────────────────────────────────────────

/**
 * @brief 设置生成多项式列表
 *
 * 每个生成多项式用一个整数表示，最低位对应当前输入比特。
 * 多项式数量决定编码输出路数（rate = 1/n_gen）。
 *
 * @param gens 生成多项式列表
 */
void ConvolutionalCode2::setGenerators(const QVector<int>& gens)
{
    m_generators = gens;
    if (m_generators.isEmpty()) {
        m_generators = {0x79, 0x5B};
    }
}

/**
 * @brief 设置约束长度
 *
 * 约束长度决定了编码器的移位寄存器级数。
 * 网格图状态数为 2^(constraintLength-1)。
 *
 * @param k 约束长度，必须 >= 2
 */
void ConvolutionalCode2::setConstraintLength(int k)
{
    m_constraint = qMax(2, k);
}

// ──────────────────────────────────────────────
// 编码
// ──────────────────────────────────────────────

/**
 * @brief 对输入比特流执行卷积编码
 *
 * 编码器维护一个 (constraintLength-1) 级移位寄存器。
 * 每输入一个比特，根据所有生成多项式计算对应输出。
 * 寄存器在编码结束后清零（tail biting）。
 *
 * @param bits 输入比特流（0/1）
 * @return 编码输出比特流，长度 = (inputLen + constraint - 1) * numGenerators
 */
QVector<int> ConvolutionalCode2::encode(const QVector<int>& bits)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> output;
    const int nGen = m_generators.size();
    const int regLen = m_constraint - 1;
    int shiftReg = 0;

    // 编码输入比特
    for (int i = 0; i < bits.size(); ++i) {
        // 将新比特移入寄存器高位
        shiftReg = ((shiftReg << 1) | (bits[i] & 1)) & ((1 << m_constraint) - 1);

        // 对每个生成多项式计算输出
        for (int g = 0; g < nGen; ++g) {
            int outBit = 0;
            for (int b = 0; b < m_constraint; ++b) {
                if (m_generators[g] & (1 << b)) {
                    outBit ^= (shiftReg >> b) & 1;
                }
            }
            output.append(outBit);
        }
    }

    // 尾部清零：冲刷移位寄存器
    for (int t = 0; t < regLen; ++t) {
        shiftReg = (shiftReg << 1) & ((1 << m_constraint) - 1);
        for (int g = 0; g < nGen; ++g) {
            int outBit = 0;
            for (int b = 0; b < m_constraint; ++b) {
                if (m_generators[g] & (1 << b)) {
                    outBit ^= (shiftReg >> b) & 1;
                }
            }
            output.append(outBit);
        }
    }

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalEncodes++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    emit encodeCompleted(bits.size(), output.size());
    return output;
}

// ──────────────────────────────────────────────
// Viterbi 译码
// ──────────────────────────────────────────────

/**
 * @brief 对软判决输入执行 Viterbi 硬判决译码
 *
 * 算法步骤：
 * 1. 将软判决值量化为硬比特（>0 → 1, <=0 → 0）
 * 2. 在网格图上逐时间步执行加-比较-选择（ACS）
 * 3. 回溯幸存路径得到译码比特
 *
 * @param softBits 软判决输入，正值代表1，负值代表0
 * @return 译码后的比特流
 */
QVector<int> ConvolutionalCode2::decode(const QVector<double>& softBits)
{
    QElapsedTimer timer;
    timer.start();

    const int nGen = m_generators.size();
    const int numStates = 1 << (m_constraint - 1);
    const int totalSymbols = softBits.size() / nGen;
    const int dataLen = totalSymbols - (m_constraint - 1);

    if (dataLen <= 0) {
        return {};
    }

    // 软判决转硬比特
    QVector<int> hardBits(softBits.size());
    for (int i = 0; i < softBits.size(); ++i) {
        hardBits[i] = (softBits[i] > 0.0) ? 1 : 0;
    }

    // 构建网格图：对每个状态和输入，预计算输出符号
    // outputTable[state][inputBit] = 编码输出符号
    QVector<QVector<QVector<int>>> outputTable(numStates, QVector<QVector<int>>(2));
    for (int s = 0; s < numStates; ++s) {
        for (int input = 0; input < 2; ++input) {
            int shiftReg = ((s << 1) | input) & ((1 << m_constraint) - 1);
            for (int g = 0; g < nGen; ++g) {
                int outBit = 0;
                for (int b = 0; b < m_constraint; ++b) {
                    if (m_generators[g] & (1 << b)) {
                        outBit ^= (shiftReg >> b) & 1;
                    }
                }
                outputTable[s][input].append(outBit);
            }
        }
    }

    // 预计算汉明距离查找表
    // distTable[state][input][time] = 汉明距离
    QVector<QVector<QVector<int>>> distTable(numStates, QVector<QVector<int>>(2));
    for (int s = 0; s < numStates; ++s) {
        for (int input = 0; input < 2; ++input) {
            // 延迟计算：在ACS时根据接收符号计算
        }
    }

    // Viterbi ACS
    const int INF = 0x7FFFFFFF;
    QVector<int> pathMetric(numStates, INF);
    pathMetric[0] = 0; // 起始状态为0

    // 幸存路径历史
    QVector<QVector<int>> history(totalSymbols, QVector<int>(numStates, 0));

    for (int t = 0; t < totalSymbols; ++t) {
        QVector<int> newMetric(numStates, INF);

        // 当前接收的nGen个符号
        QVector<int> received(nGen);
        for (int g = 0; g < nGen; ++g) {
            int idx = t * nGen + g;
            received[g] = (idx < hardBits.size()) ? hardBits[idx] : 0;
        }

        for (int s = 0; s < numStates; ++s) {
            if (pathMetric[s] >= INF) continue;

            for (int input = 0; input < 2; ++input) {
                // 前一状态：s的低(regLen-1)位 + input作为新高位
                int prevState = (s >> 1) | (input << (m_constraint - 2));
                // 到达当前状态s时，输入必须是 s 的最低位
                // 实际上：如果当前输入为 (s & 1)，前一状态 = s >> 1
                // 重新推导：
                // 编码时 shiftReg = ((prev << 1) | input)
                // 所以 prev = shiftReg >> 1, input = shiftReg & 1
                // 当前状态 s = shiftReg 的低 (constraint-1) 位
            }
        }

        // 重新实现ACS
        newMetric.fill(INF);
        for (int s = 0; s < numStates; ++s) {
            // 状态s由前一状态和输入到达：
            // 如果输入为0: prev = s, 新shiftReg = s << 1
            // 如果输入为1: prev = s, 新shiftReg = (s << 1) | 1
            // 下一状态: next = (s << 1 | input) & (numStates - 1)
            // 所以从状态s、输入input -> 下一状态next = ((s << 1) | input) & (numStates-1)
        }

        newMetric.fill(INF);
        for (int s = 0; s < numStates; ++s) {
            if (pathMetric[s] >= INF) continue;

            for (int input = 0; input < 2; ++input) {
                int nextState = ((s << 1) | input) & (numStates - 1);
                int shiftReg = ((s << 1) | input);

                // 计算该转移的编码输出
                QVector<int> expected(nGen);
                for (int g = 0; g < nGen; ++g) {
                    int outBit = 0;
                    for (int b = 0; b < m_constraint; ++b) {
                        if (m_generators[g] & (1 << b)) {
                            outBit ^= (shiftReg >> b) & 1;
                        }
                    }
                    expected[g] = outBit;
                }

                int dist = 0;
                for (int g = 0; g < nGen; ++g) {
                    dist += (expected[g] != received[g]) ? 1 : 0;
                }

                int newCost = pathMetric[s] + dist;
                if (newCost < newMetric[nextState]) {
                    newMetric[nextState] = newCost;
                    history[t][nextState] = s;
                }
            }
        }

        pathMetric = newMetric;
    }

    // 回溯：从状态0开始（因为尾部清零）
    int bestState = 0;
    int minMetric = pathMetric[0];
    for (int s = 1; s < numStates; ++s) {
        if (pathMetric[s] < minMetric) {
            minMetric = pathMetric[s];
            bestState = s;
        }
    }

    QVector<int> decoded;
    decoded.reserve(dataLen);

    int currentState = bestState;
    for (int t = totalSymbols - 1; t >= 0; --t) {
        int prevState = history[t][currentState];
        int inputBit = currentState & 1;
        decoded.prepend(inputBit);
        currentState = prevState;
    }

    // 移除尾部比特
    while (decoded.size() > dataLen) {
        decoded.removeLast();
    }

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalDecodes++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    emit decodeCompleted(minMetric);
    return decoded;
}

// ──────────────────────────────────────────────
// 统计接口
// ──────────────────────────────────────────────

/**
 * @brief 获取当前统计信息
 * @return 包含编码/译码次数和平均耗时的Stats结构
 */
ConvolutionalCode2::Stats ConvolutionalCode2::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有累计统计信息
 */
void ConvolutionalCode2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

// ──────────────────────────────────────────────
// 私有方法
// ──────────────────────────────────────────────

/**
 * @brief 计算两个比特向量之间的汉明距离
 * @param a 第一个比特向量
 * @param b 第二个比特向量
 * @return 汉明距离（不同位的数量）
 */
int ConvolutionalCode2::hammingDistance(const QVector<int>& a, const QVector<int>& b) const
{
    int dist = 0;
    const int len = qMin(a.size(), b.size());
    for (int i = 0; i < len; ++i) {
        if (a[i] != b[i]) {
            ++dist;
        }
    }
    return dist;
}
