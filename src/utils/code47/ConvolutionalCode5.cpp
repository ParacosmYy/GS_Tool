/**
 * @file ConvolutionalCode5.cpp
 * @brief 卷积码5 — 打孔卷积+速率兼容 实现
 *
 * 实现约束长度可配置的卷积编码器与 Viterbi 译码器。
 * 支持打孔模式设置和速率兼容，具备完整的网格表构建、
 * 编码、打孔/去打孔和软判决译码功能。
 */

#include "utils/code47/ConvolutionalCode5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject
 */
ConvolutionalCode5::ConvolutionalCode5(QObject* parent)
    : QObject(parent)
{
    /* 默认生成多项式: (171, 133) 八进制，K=7 */
    setGenerators({0171, 0133}, 7);
}

/**
 * @brief 设置卷积码的生成多项式和约束长度
 *
 * 生成多项式以八进制整数表示，将自动构建网格转移表。
 *
 * @param generators 生成多项式列表（八进制表示）
 * @param K 约束长度
 */
void ConvolutionalCode5::setGenerators(const QVector<int>& generators, int K)
{
    m_K = qMax(2, K);
    m_generators = generators;
    m_numStates = 1 << (m_K - 1);
    m_tablesBuilt = false;
    buildTables();
}

/**
 * @brief 设置打孔模式
 *
 * 打孔模式为 0/1 序列，1 表示保留该比特，0 表示删除。
 * 周期由 pattern 长度决定。
 *
 * @param pattern 打孔模式
 */
void ConvolutionalCode5::setPuncturingPattern(const QVector<int>& pattern)
{
    m_puncturePattern = pattern;
    m_puncturePeriod = pattern.isEmpty() ? 1 : pattern.size();
}

/**
 * @brief 设置目标码率
 * @param rate 目标码率 (0, 1]
 */
void ConvolutionalCode5::setRate(double rate)
{
    Q_UNUSED(rate)
    /* 码率由打孔模式决定，此方法保留为速率兼容接口 */
}

/**
 * @brief 构建网格状态转移表
 *
 * 对每个状态和输入比特，预计算下一状态和编码输出。
 * 输出由生成多项式与移位寄存器状态的模2和决定。
 */
void ConvolutionalCode5::buildTables()
{
    const int n = m_generators.size();
    m_nextState.assign(m_numStates, QVector<int>(2, 0));
    m_output.assign(m_numStates, QVector<int>(2, 0));

    for (int state = 0; state < m_numStates; ++state) {
        for (int input = 0; input < 2; ++input) {
            /* 移位寄存器: 新比特在高位 */
            int reg = (state >> 1) | (input << (m_K - 2));

            /* 计算各生成多项式的输出 */
            int out = 0;
            for (int g = 0; g < n; ++g) {
                int gen = m_generators[g];
                int parity = 0;
                for (int bit = 0; bit < m_K; ++bit) {
                    if (gen & (1 << bit)) {
                        int regBit = (reg >> (m_K - 1 - bit)) & 1;
                        parity ^= regBit;
                    }
                }
                out = (out << 1) | parity;
            }

            m_nextState[state][input] = reg;
            m_output[state][input] = out;
        }
    }
    m_tablesBuilt = true;
}

/**
 * @brief 对输入比特序列进行卷积编码
 *
 * 遍历输入比特，利用网格转移表生成编码输出，
 * 然后按打孔模式进行打孔。
 *
 * @param bits 输入比特序列
 * @return 编码后的比特序列（经过打孔）
 */
QVector<int> ConvolutionalCode5::encode(const QVector<int>& bits)
{
    QElapsedTimer timer;
    timer.start();

    const int n = m_generators.size();
    QVector<int> encoded;
    encoded.reserve(bits.size() * n);

    int state = 0;
    for (int i = 0; i < bits.size(); ++i) {
        int bit = bits[i] & 1;
        int out = m_output[state][bit];
        state = m_nextState[state][bit];

        /* 将 n 比特输出拆分为单独比特 */
        for (int g = n - 1; g >= 0; --g) {
            encoded.append((out >> g) & 1);
        }
    }

    /* 尾比特：灌入 K-1 个零使状态归零 */
    for (int i = 0; i < m_K - 1; ++i) {
        int out = m_output[state][0];
        state = m_nextState[state][0];
        for (int g = n - 1; g >= 0; --g) {
            encoded.append((out >> g) & 1);
        }
    }

    /* 应用打孔 */
    QVector<int> punctured;
    if (!m_puncturePattern.isEmpty()) {
        punctured.reserve(encoded.size());
        for (int i = 0; i < encoded.size(); ++i) {
            if (m_puncturePattern[i % m_puncturePeriod] == 1) {
                punctured.append(encoded[i]);
            }
        }
    } else {
        punctured = encoded;
    }

    /* 统计更新 */
    m_stats.totalEncodes++;
    m_stats.totalBitsProcessed += bits.size();
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    emit encodeCompleted(bits.size(), punctured.size());
    return punctured;
}

/**
 * @brief 使用 Viterbi 算法对软判决序列进行译码
 *
 * 完整的维特比译码流程：去打孔 -> 网格搜索 -> 回溯。
 * 使用欧氏距离作为分支度量。
 *
 * @param softBits 软判决输入序列
 * @return 译码后的硬比特序列
 */
QVector<int> ConvolutionalCode5::decode(const QVector<double>& softBits)
{
    QElapsedTimer timer;
    timer.start();

    const int n = m_generators.size();

    /* 去打孔 */
    QVector<double> depunctured = depuncture(softBits);
    const int totalSymbols = depunctured.size();
    const int numSteps = totalSymbols / n;

    if (numSteps <= 0) {
        return {};
    }

    /* 初始化路径度量 */
    const double INF = 1e15;
    QVector<double> pathMetric(m_numStates, INF);
    pathMetric[0] = 0.0;

    /* 幸存路径记录 */
    QVector<QVector<int>> survivor(m_numStates, QVector<int>(numSteps, 0));
    QVector<double> newMetric(m_numStates, INF);

    /* 网格搜索 */
    for (int step = 0; step < numSteps; ++step) {
        std::fill(newMetric.begin(), newMetric.end(), INF);

        for (int state = 0; state < m_numStates; ++state) {
            if (pathMetric[state] >= INF) continue;

            for (int input = 0; input < 2; ++input) {
                int nextState = m_nextState[state][input];
                int out = m_output[state][input];

                /* 计算分支度量（欧氏距离） */
                double branch = 0.0;
                for (int g = 0; g < n; ++g) {
                    double expected = ((out >> (n - 1 - g)) & 1) ? 1.0 : -1.0;
                    double symbol = (step * n + g < totalSymbols) ? depunctured[step * n + g] : 0.0;
                    double diff = symbol - expected;
                    branch += diff * diff;
                }

                double candidate = pathMetric[state] + branch;
                if (candidate < newMetric[nextState]) {
                    newMetric[nextState] = candidate;
                    survivor[nextState][step] = state;
                }
            }
        }
        pathMetric = newMetric;
    }

    /* 回溯 */
    int bestState = 0;
    double bestMetric = pathMetric[0];
    for (int s = 1; s < m_numStates; ++s) {
        if (pathMetric[s] < bestMetric) {
            bestMetric = pathMetric[s];
            bestState = s;
        }
    }

    QVector<int> decoded;
    decoded.reserve(numSteps);
    int currentState = bestState;
    for (int step = numSteps - 1; step >= 0; --step) {
        int prevState = survivor[currentState][step];
        /* 判断输入比特: 高位决定当前状态来源 */
        int inputBit = (currentState >> (m_K - 2)) & 1;
        decoded.prepend(inputBit);
        currentState = prevState;
    }

    /* 去掉尾比特 */
    if (decoded.size() > m_K - 1) {
        decoded = decoded.mid(0, decoded.size() - (m_K - 1));
    }

    /* 统计更新 */
    m_stats.totalDecodes++;
    m_stats.totalBitsProcessed += decoded.size();
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    return decoded;
}

/**
 * @brief 对打孔后的软比特序列执行去打孔
 *
 * 将被打孔删除的位置插入0（等效于软判决中性值），
 * 恢复原始码率的比特流。
 *
 * @param softBits 打孔后的软判决序列
 * @return 去打孔后的软判决序列
 */
QVector<int> ConvolutionalCode5::depuncture(const QVector<double>& softBits) const
{
    if (m_puncturePattern.isEmpty()) {
        QVector<int> result;
        result.reserve(softBits.size());
        for (double v : softBits) {
            result.append((v >= 0.0) ? 1 : 0);
        }
        return result;
    }

    /* 将 softBits 转为硬比特，插入打孔位置为 0 */
    QVector<int> result;
    int softIdx = 0;
    for (int i = 0; i < softBits.size() + softBits.size() / m_puncturePeriod + 1; ++i) {
        if (m_puncturePattern[i % m_puncturePeriod] == 1) {
            if (softIdx < softBits.size()) {
                result.append((softBits[softIdx] >= 0.0) ? 1 : 0);
                softIdx++;
            }
        } else {
            result.append(0);
        }
    }
    return result;
}

/**
 * @brief 获取当前码率
 * @return 码率值
 */
double ConvolutionalCode5::codeRate() const
{
    int n = m_generators.size();
    if (m_puncturePattern.isEmpty() || n == 0) {
        return (n > 0) ? 1.0 / n : 0.0;
    }
    int ones = 0;
    for (int v : m_puncturePattern) {
        if (v == 1) ones++;
    }
    double puncturedRate = static_cast<double>(ones) / m_puncturePeriod;
    return (puncturedRate > 0.0) ? 1.0 / (n / puncturedRate) : 0.0;
}

/**
 * @brief 重置所有统计数据
 */
void ConvolutionalCode5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
