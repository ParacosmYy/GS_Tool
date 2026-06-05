#include "ConvCode5.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化卷积码编解码器
 * @param parent 父对象指针
 */
ConvCode5::ConvCode5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置卷积码约束长度
 * @param len 约束长度，决定编码记忆深度
 */
void ConvCode5::setConstraintLen(int len)
{
    m_constraintLen = qMax(2, len);
}

/**
 * @brief 对输入比特序列进行卷积编码
 *
 * 使用生成多项式 (octal 171, 133) 对输入比特流进行卷积编码，
 * 每个输入比特产生两个输出比特，码率为1/2。
 *
 * @param bits 输入比特序列(0或1)
 * @return 编码后的比特序列
 */
QVector<int> ConvCode5::encode(const QVector<int>& bits)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result;
    if (bits.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalOperations++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
        emit codingCompleted(0);
        return result;
    }

    /* 移位寄存器初始化 */
    int reg = 0;
    int mask = (1 << m_constraintLen) - 1;

    /* 生成多项式: G1 = 171(八进制), G2 = 133(八进制) */
    int g1 = 0171; /* 001111001 */
    int g2 = 0133; /* 001011011 */

    for (int bit : bits) {
        /* 移入新比特 */
        reg = ((reg << 1) | bit) & mask;

        /* 计算两个生成多项式的输出 */
        int out1 = 0, out2 = 0;
        int r = reg;
        int m1 = g1, m2 = g2;
        while (r > 0) {
            out1 ^= (r & 1) & (m1 & 1);
            out2 ^= (r & 1) & (m2 & 1);
            r >>= 1;
            m1 >>= 1;
            m2 >>= 1;
        }
        result.append(out1);
        result.append(out2);
    }

    /* 尾比特：清零移位寄存器 */
    for (int i = 0; i < m_constraintLen - 1; ++i) {
        reg = (reg << 1) & mask;
        int out1 = 0, out2 = 0;
        int r = reg;
        int m1 = g1, m2 = g2;
        while (r > 0) {
            out1 ^= (r & 1) & (m1 & 1);
            out2 ^= (r & 1) & (m2 & 1);
            r >>= 1;
            m1 >>= 1;
            m2 >>= 1;
        }
        result.append(out1);
        result.append(out2);
    }

    m_timeSum += timer.elapsed();
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
    emit codingCompleted(bits.size());
    return result;
}

/**
 * @brief 对接收序列进行Viterbi解码
 *
 * 使用Viterbi算法在网格图上搜索最大似然路径，
 * 通过度量比较和路径回溯恢复原始比特序列。
 *
 * @param received 接收的比特序列(软判决或硬判决)
 * @return 解码后的比特序列
 */
QVector<int> ConvCode5::decode(const QVector<int>& received)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result;
    if (received.isEmpty() || received.size() < 4) {
        m_timeSum += timer.elapsed();
        m_stats.totalOperations++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
        emit codingCompleted(0);
        return result;
    }

    int numStates = 1 << (m_constraintLen - 1);
    int steps = received.size() / 2;

    /* 路径度量和路径历史 */
    QVector<double> metric(numStates, 1e18);
    QVector<QVector<int>> path(numStates);
    metric[0] = 0.0;

    for (int t = 0; t < steps; ++t) {
        int r0 = (t * 2 < received.size()) ? received[t * 2] : 0;
        int r1 = (t * 2 + 1 < received.size()) ? received[t * 2 + 1] : 0;

        QVector<double> newMetric(numStates, 1e18);
        QVector<QVector<int>> newPath(numStates);

        for (int s = 0; s < numStates; ++s) {
            if (metric[s] >= 1e17) continue;

            for (int inBit = 0; inBit <= 1; ++inBit) {
                /* 计算下一状态 */
                int nextState = ((s << 1) | inBit) & (numStates - 1);
                /* 计算分支度量(汉明距离) */
                int g1Bit = (__builtin_parity(s & 0171)) ^ inBit;
                int g2Bit = (__builtin_parity(s & 0133)) ^ inBit;
                double branchMetric = std::abs(r0 - g1Bit) + std::abs(r1 - g2Bit);
                double totalMetric = metric[s] + branchMetric;

                if (totalMetric < newMetric[nextState]) {
                    newMetric[nextState] = totalMetric;
                    newPath[nextState] = path[s];
                    newPath[nextState].append(inBit);
                }
            }
        }

        metric = newMetric;
        path = newPath;
    }

    /* 找最小度量路径 */
    int bestState = 0;
    for (int s = 1; s < numStates; ++s) {
        if (metric[s] < metric[bestState]) bestState = s;
    }

    /* 输出解码结果(去掉尾比特) */
    int dataLen = qMax(0, path[bestState].size() - (m_constraintLen - 1));
    for (int i = 0; i < dataLen; ++i) {
        result.append(path[bestState][i]);
    }

    m_timeSum += timer.elapsed();
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
    emit codingCompleted(result.size());
    return result;
}

/**
 * @brief 重置统计数据
 */
void ConvCode5::resetStatistics()
{
    m_stats.totalOperations = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
