/**
 * @file ConvolutionalCoder.cpp
 * @brief 卷积编码器/维特比译码器实现
 * @author Serial Tool Team
 * @date 2026-06-05
 */

#include "utils/code4/ConvolutionalCoder.h"

#include <QElapsedTimer>
#include <QtGlobal>
#include <limits>

/** @brief 构造函数
 *  @param constraintLength 约束长度K(默认7)
 *  @param generators 生成多项式列表(八进制，默认[171,133]即K=7速率1/2)
 *  @param parent 父对象 */
ConvolutionalCoder::ConvolutionalCoder(int constraintLength,
                                       const QVector<int> &generators,
                                       QObject *parent)
    : QObject(parent)
    , m_constraintLength(qMax(2, constraintLength))
    , m_generators(generators.isEmpty() ? QVector<int>{171, 133} : generators)
    , m_numStates(1 << (m_constraintLength - 1))
{
}

/** @brief 卷积编码: 对每个输入比特，将当前移位寄存器状态与各生成多项式做异或
 *  @param input 输入比特流(0/1)
 *  @return 编码输出比特流(长度 = input.size() * numGenerators) */
QVector<int> ConvolutionalCoder::encode(const QVector<int> &input)
{
    QElapsedTimer timer;
    timer.start();

    if (input.isEmpty() || m_generators.isEmpty()) {
        return {};
    }

    int n = input.size();
    int numGen = m_generators.size();
    QVector<int> output;
    output.reserve(n * numGen);

    int state = 0; /* 移位寄存器初始状态为0 */
    int mask = m_numStates - 1;

    for (int i = 0; i < n; ++i) {
        /* 移入新比特到最高位 */
        state = ((state << 1) | (input[i] & 1)) & mask;

        /* 对每个生成多项式计算输出 */
        for (int g = 0; g < numGen; ++g) {
            output.append(applyPolynomial(m_generators[g], state));
        }
    }

    /* 尾比特: 冲刷移位寄存器(K-1个零) */
    for (int t = 0; t < m_constraintLength - 1; ++t) {
        state = (state << 1) & mask;
        for (int g = 0; g < numGen; ++g) {
            output.append(applyPolynomial(m_generators[g], state));
        }
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    m_stats.totalEncoded += static_cast<quint64>(n);
    m_timeSum += elapsed;
    quint64 totalOps = m_stats.totalEncoded + m_stats.totalDecoded;
    m_stats.avgProcessingTimeMs = (totalOps > 0)
        ? m_timeSum / static_cast<double>(totalOps) : 0.0;

    emit encodeCompleted(n, output.size());
    return output;
}

/** @brief 维特比硬判决译码: 在网格图上搜索Hamming距离最小的路径
 *  @param received 接收的编码比特流(0/1)
 *  @return 译码输出比特流(去除尾比特) */
QVector<int> ConvolutionalCoder::decode(const QVector<int> &received)
{
    QElapsedTimer timer;
    timer.start();

    int numGen = m_generators.size();
    if (received.isEmpty() || numGen == 0) {
        return {};
    }

    /* 每个时间步消耗 numGen 个编码比特 */
    int totalSteps = received.size() / numGen;
    if (totalSteps < m_constraintLength) {
        return {};
    }

    int dataBits = totalSteps - (m_constraintLength - 1); /* 去掉尾比特对应的步数 */
    if (dataBits <= 0) {
        return {};
    }

    int numStates = m_numStates;
    const double INF = std::numeric_limits<double>::max();

    /* 路径度量: accumCost[s] = 到达状态s的累积Hamming距离 */
    QVector<double> accumCost(numStates, INF);
    QVector<double> nextCost(numStates, INF);
    accumCost[0] = 0.0; /* 初始状态为0 */

    /* 路径历史: history[t][s] = 到达状态s的前驱状态 */
    QVector<QVector<int>> history(totalSteps,
                                  QVector<int>(numStates, 0));

    /* 逐时间步推进网格 */
    for (int t = 0; t < totalSteps; ++t) {
        nextCost.fill(INF);

        /* 从接收序列中提取当前步的编码比特 */
        QVector<int> rxBits(numGen);
        for (int g = 0; g < numGen; ++g) {
            int idx = t * numGen + g;
            rxBits[g] = (idx < received.size()) ? received[idx] : 0;
        }

        for (int s = 0; s < numStates; ++s) {
            if (accumCost[s] >= INF) continue;

            /* 两种输入: 0和1 */
            for (int bit = 0; bit <= 1; ++bit) {
                int nextState = ((s << 1) | bit) & (numStates - 1);
                QVector<int> expected = computeOutput(s, bit);

                /* 计算Hamming距离 */
                double dist = 0.0;
                for (int g = 0; g < numGen; ++g) {
                    dist += qAbs(rxBits[g] - expected[g]);
                }

                double newCost = accumCost[s] + dist;
                if (newCost < nextCost[nextState]) {
                    nextCost[nextState] = newCost;
                    history[t][nextState] = s;
                }
            }
        }

        /* 交换累积代价 */
        accumCost = nextCost;
    }

    /* 回溯: 从代价最小的终态开始 */
    int bestState = 0;
    double bestCost = accumCost[0];
    for (int s = 1; s < numStates; ++s) {
        if (accumCost[s] < bestCost) {
            bestCost = accumCost[s];
            bestState = s;
        }
    }

    QVector<int> decoded;
    decoded.reserve(totalSteps);
    int curState = bestState;
    for (int t = totalSteps - 1; t >= 0; --t) {
        /* 输入比特 = nextState的最低位 */
        decoded.append(curState & 1);
        curState = history[t][curState];
    }

    /* 反转回溯结果(时间顺序) */
    std::reverse(decoded.begin(), decoded.end());

    /* 去除尾比特(最后K-1个) */
    if (decoded.size() > m_constraintLength - 1) {
        decoded.resize(decoded.size() - (m_constraintLength - 1));
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    m_stats.totalDecoded += static_cast<quint64>(decoded.size());
    m_timeSum += elapsed;
    quint64 totalOps = m_stats.totalEncoded + m_stats.totalDecoded;
    m_stats.avgProcessingTimeMs = (totalOps > 0)
        ? m_timeSum / static_cast<double>(totalOps) : 0.0;

    emit decodeCompleted(decoded.size());
    return decoded;
}

/** @brief 设置生成多项式
 *  @param gens 生成多项式列表(八进制表示) */
void ConvolutionalCoder::setGenerators(const QVector<int> &gens)
{
    if (!gens.isEmpty()) {
        m_generators = gens;
    }
}

/** @brief 重置统计计数器 */
void ConvolutionalCoder::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 计算给定状态和输入的编码输出
 *  @param state 移位寄存器状态(不含新输入位)
 *  @param input 输入比特(0/1)
 *  @return 各生成多项式的输出比特列表 */
QVector<int> ConvolutionalCoder::computeOutput(int state, int input) const
{
    /* 新状态 = (state << 1) | input */
    int newState = ((state << 1) | (input & 1)) & (m_numStates - 1);
    QVector<int> out;
    out.reserve(m_generators.size());
    for (int poly : m_generators) {
        out.append(applyPolynomial(poly, newState));
    }
    return out;
}

/** @brief 将生成多项式(八进制)应用于寄存器状态，输出奇偶校验位
 *  @param poly 生成多项式(八进制表示)
 *  @param state 寄存器状态
 *  @return 0或1 */
int ConvolutionalCoder::applyPolynomial(int poly, int state) const
{
    int result = 0;
    int bits = poly;
    int reg = state;
    /* 对多项式的每一位，如果为1则与对应寄存器位异或 */
    while (bits > 0 || reg > 0) {
        if (bits & 1) {
            result ^= (reg & 1);
        }
        bits >>= 1;
        reg >>= 1;
    }
    return result;
}
