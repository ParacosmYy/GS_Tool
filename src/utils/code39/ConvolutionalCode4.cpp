/**
 * @file ConvolutionalCode4.cpp
 * @brief 卷积码4 — 软判决Viterbi+回溯实现
 *
 * 实现卷积码编码和Viterbi解码：
 * - 网格图构建（状态转移表/输出表）
 * - 编码：移位寄存器+生成多项式
 * - 软判决Viterbi：分支度量+路径度量+回溯
 * - 硬判决解码
 * 所有运算带有QElapsedTimer计时和统计信息追踪。
 */

#include "code39/ConvolutionalCode4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <limits>

/**
 * @brief 构造函数
 * @param parent 父对象
 */
ConvolutionalCode4::ConvolutionalCode4(QObject* parent)
    : QObject(parent)
{
    /* 默认配置：K=7, 生成多项式 [171, 133]（八进制） */
    m_generators = {0x71, 0x53};
    m_K = 7;
    m_nGenerators = 2;
    m_numStates = 1 << (m_K - 1);
    buildTables();
}

/**
 * @brief 设置生成多项式和约束长度
 * @param generators 生成多项式列表（八进制/整数形式）
 * @param constraintLength 约束长度K
 */
void ConvolutionalCode4::setGenerator(const QVector<int>& generators, int constraintLength)
{
    m_generators = generators;
    m_K = constraintLength;
    m_nGenerators = generators.size();
    m_numStates = 1 << (m_K - 1);
    m_tablesBuilt = false;
    buildTables();
}

/**
 * @brief 构建状态转移表和输出表
 *
 * 对每个状态和每个输入比特，计算：
 * - next_state: 移位寄存器新状态
 * - output: 编码输出比特组
 */
void ConvolutionalCode4::buildTables()
{
    m_nextState.resize(m_numStates, QVector<int>(2, 0));
    m_output.resize(m_numStates, QVector<int>(2, 0));

    for (int state = 0; state < m_numStates; ++state) {
        for (int input = 0; input <= 1; ++input) {
            /* 构造移位寄存器内容：当前状态 + 新输入 */
            int reg = (state << 1) | input;
            /* 只保留K位 */
            reg &= (1 << m_K) - 1;

            /* 计算每个生成多项式的输出 */
            int outBits = 0;
            for (int g = 0; g < m_nGenerators; ++g) {
                int parity = 0;
                for (int bit = 0; bit < m_K; ++bit) {
                    if (m_generators[g] & (1 << bit))
                        parity ^= ((reg >> bit) & 1);
                }
                outBits |= (parity << g);
            }
            m_output[state][input] = outBits;

            /* 新状态：去掉最高位 */
            m_nextState[state][input] = (reg >> 1) & (m_numStates - 1);
        }
    }
    m_tablesBuilt = true;
}

/**
 * @brief 蝶形转移计算
 * @param state 当前状态
 * @param input 输入比特
 * @return 新状态
 */
int ConvolutionalCode4::butterflyTransition(int state, int input) const
{
    return m_nextState[state][input];
}

/**
 * @brief 卷积编码
 * @param bits 输入比特流
 * @return 编码输出比特流（长度 = 输入长度 * nGenerators）
 */
QVector<int> ConvolutionalCode4::encode(const QVector<int>& bits)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_tablesBuilt) buildTables();

    QVector<int> encoded;
    encoded.reserve(bits.size() * m_nGenerators);
    int state = 0;

    for (int bit : bits) {
        int outBits = m_output[state][bit];
        state = m_nextState[state][bit];

        /* 展开输出比特 */
        for (int g = 0; g < m_nGenerators; ++g)
            encoded.append((outBits >> g) & 1);
    }

    /* 尾部终止：填入K-1个零 */
    for (int i = 0; i < m_K - 1; ++i) {
        int outBits = m_output[state][0];
        state = m_nextState[state][0];
        for (int g = 0; g < m_nGenerators; ++g)
            encoded.append((outBits >> g) & 1);
    }

    /* 更新统计 */
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalEncodes++;
    m_stats.totalBitsProcessed += bits.size();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalEncodes + m_stats.totalDecodes);

    emit encodeCompleted(bits.size(), encoded.size());
    return encoded;
}

/**
 * @brief 软判决Viterbi解码
 * @param softBits 软比特输入（每个符号一个浮点值，0~1表示0→1概率）
 * @return 解码后的比特流
 *
 * 使用欧氏距离作为分支度量，执行加-比较-选择(ACS)操作，
 * 最后回溯最优路径。
 */
QVector<int> ConvolutionalCode4::decodeSoft(const QVector<double>& softBits)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_tablesBuilt) buildTables();

    int numSymbols = softBits.size() / m_nGenerators;
    if (numSymbols <= 0) return {};

    /* 路径度量初始化 */
    const double INF = 1e30;
    QVector<double> pathMetric(m_numStates, INF);
    pathMetric[0] = 0.0;

    /* 回溯存储 */
    QVector<QVector<int>> traceback(numSymbols, QVector<int>(m_numStates, 0));

    for (int t = 0; t < numSymbols; ++t) {
        QVector<double> newMetric(m_numStates, INF);

        for (int state = 0; state < m_numStates; ++state) {
            if (pathMetric[state] >= INF) continue;

            for (int input = 0; input <= 1; ++input) {
                int nextState = m_nextState[state][input];
                int outBits = m_output[state][input];

                /* 计算分支度量：欧氏距离 */
                double branchMetric = 0.0;
                for (int g = 0; g < m_nGenerators; ++g) {
                    double expected = (outBits >> g) & 1;
                    double received = softBits[t * m_nGenerators + g];
                    double diff = received - expected;
                    branchMetric += diff * diff;
                }

                double totalMetric = pathMetric[state] + branchMetric;
                if (totalMetric < newMetric[nextState]) {
                    newMetric[nextState] = totalMetric;
                    traceback[t][nextState] = state;
                }
            }
        }
        pathMetric = newMetric;
    }

    /* 回溯：从度量最小的状态开始 */
    int bestState = 0;
    double bestMetric = pathMetric[0];
    for (int s = 1; s < m_numStates; ++s) {
        if (pathMetric[s] < bestMetric) {
            bestMetric = pathMetric[s];
            bestState = s;
        }
    }

    /* 反向回溯 */
    int dataLen = numSymbols - (m_K - 1);
    QVector<int> decoded;
    decoded.reserve(dataLen);
    int curState = bestState;
    for (int t = numSymbols - 1; t >= 0; --t) {
        int prevState = traceback[t][curState];
        /* 判断输入比特：prevState的高位+输入 -> curState */
        int input = (curState >> (m_K - 2)) & 1;
        if (t < dataLen)
            decoded.prepend(input);
        /* 实际上需要反向推导输入 */
        for (int in = 0; in <= 1; ++in) {
            if (m_nextState[prevState][in] == curState) {
                if (t < dataLen)
                    decoded.front() = in;
                break;
            }
        }
        curState = prevState;
    }

    /* 更新统计 */
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDecodes++;
    m_stats.totalBitsProcessed += decoded.size();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalEncodes + m_stats.totalDecodes);

    emit decodeCompleted(decoded.size(), bestMetric);
    return decoded;
}

/**
 * @brief 硬判决解码
 * @param hardBits 硬比特输入（0或1）
 * @return 解码后的比特流
 */
QVector<int> ConvolutionalCode4::decodeHard(const QVector<int>& hardBits)
{
    /* 将硬比特转为软比特：0->0.0, 1->1.0 */
    QVector<double> softBits;
    softBits.reserve(hardBits.size());
    for (int bit : hardBits)
        softBits.append(static_cast<double>(bit));
    return decodeSoft(softBits);
}

/**
 * @brief 终止编码器（将状态归零）
 */
void ConvolutionalCode4::terminate()
{
    /* terminate由encode自动执行尾部填充 */
}

/**
 * @brief 重置所有统计计数器
 */
void ConvolutionalCode4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
